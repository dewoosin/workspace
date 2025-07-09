/**
 * @file main.cpp
 * @brief GHOSTYPE BLE + HID 실시간 타이핑
 * 
 * BLE로 받은 텍스트를 즉시 USB HID로 타이핑
 * T-Dongle-S3 최적화 버전
 */

#include <Arduino.h>
#include <USB.h>
#include <USBHIDKeyboard.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <esp_gap_ble_api.h>
#include <queue>
#include <ArduinoJson.h>

// HID 키보드 객체
USBHIDKeyboard keyboard;

// BLE 객체들
BLEServer* pServer = NULL;
BLECharacteristic* pRxCharacteristic = NULL;
BLECharacteristic* pTxCharacteristic = NULL;
bool deviceConnected = false;

// 타이핑 큐 - BLE로 받은 텍스트를 저장
std::queue<String> typingQueue;
SemaphoreHandle_t queueMutex;

// BLE UUID
#define SERVICE_UUID        "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define RX_CHAR_UUID        "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
#define TX_CHAR_UUID        "6e400003-b5a3-f393-e0a9-e50e24dcca9e"

// 타이핑 상태
bool isTyping = false;
unsigned long lastTypeTime = 0;
int globalTypingSpeed = 15; // 웹 기본값과 동일 (selected option)

// 청크 재조립을 위한 변수들 (큰 데이터 지원)
String chunkBuffer = "";
bool isReceivingChunks = false;
unsigned long chunkStartTime = 0;

// 디버깅 플래그 (디버깅 시에만 true로 설정)
#define DEBUG_ENABLED true

// 조건부 시리얼 출력 매크로
#if DEBUG_ENABLED
    #define DEBUG_PRINT(x) Serial.print(x)
    #define DEBUG_PRINTLN(x) Serial.println(x)
#else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTLN(x)
#endif

// BLE 서버 콜백
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        DEBUG_PRINTLN("BLE 연결됨!");
    }
    
    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        DEBUG_PRINTLN("BLE 연결 해제됨");
        delay(500);
        pServer->getAdvertising()->start();
    }
};

// BLE 데이터 수신 콜백
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string rxValue = pCharacteristic->getValue();
        
        if (rxValue.length() > 0) {
            DEBUG_PRINT("BLE 수신 (길이: ");
            DEBUG_PRINT(rxValue.length());
            DEBUG_PRINT("): ");
            
            // 메모리 상태 체크
            DEBUG_PRINT("남은 힙 메모리: ");
            DEBUG_PRINT(ESP.getFreeHeap());
            DEBUG_PRINTLN(" bytes");
            
            // 너무 긴 텍스트는 거부
            if (rxValue.length() > 10000) {
                DEBUG_PRINTLN("텍스트가 너무 깁니다! 10KB 이하로 제한됩니다.");
                return;
            }
            
            String receivedText = String(rxValue.c_str());
            DEBUG_PRINTLN(receivedText);
            
            // 청크 처리 로직
            if (receivedText.startsWith("CHUNK_START:")) {
                // 청크 시작 - 버퍼 초기화하고 첫 번째 데이터 저장
                isReceivingChunks = true;
                chunkBuffer = receivedText.substring(12); // "CHUNK_START:" 제거
                chunkStartTime = millis();
                DEBUG_PRINTLN("청크 수신 시작");
                DEBUG_PRINT("초기 청크 길이: ");
                DEBUG_PRINTLN(chunkBuffer.length());
                return; // 아직 완전하지 않으므로 큐에 추가하지 않음
            } else if (receivedText == "CHUNK_END") {
                // 청크 종료 - 완성된 데이터를 큐에 추가
                if (isReceivingChunks) {
                    unsigned long elapsed = millis() - chunkStartTime;
                    DEBUG_PRINT("청크 수신 완료, 총 길이: ");
                    DEBUG_PRINT(chunkBuffer.length());
                    DEBUG_PRINT("바이트, 소요 시간: ");
                    DEBUG_PRINT(elapsed);
                    DEBUG_PRINTLN("ms");
                    
                    // 완성된 데이터를 큐에 추가
                    if (xSemaphoreTake(queueMutex, portMAX_DELAY) == pdTRUE) {
                        typingQueue.push(chunkBuffer);
                        xSemaphoreGive(queueMutex);
                        DEBUG_PRINTLN("재조립된 텍스트 큐에 추가됨");
                    }
                    
                    // 상태 리셋
                    isReceivingChunks = false;
                    chunkBuffer = "";
                    chunkStartTime = 0;
                }
                return;
            } else if (isReceivingChunks) {
                // 중간 청크 - 버퍼에 추가
                size_t freeHeap = ESP.getFreeHeap();
                if (freeHeap < 50000) { // 50KB 미만이면 경고
                    DEBUG_PRINTLN("경고: 메모리 부족으로 청크 수신 중단");
                    isReceivingChunks = false;
                    chunkBuffer = "";
                    return;
                }
                
                chunkBuffer += receivedText;
                DEBUG_PRINT("청크 추가, 현재 길이: ");
                DEBUG_PRINT(chunkBuffer.length());
                DEBUG_PRINT("바이트, 남은 힙: ");
                DEBUG_PRINT(freeHeap);
                DEBUG_PRINTLN(" bytes");
                return; // 아직 완전하지 않으므로 큐에 추가하지 않음
            }
            
            // 일반 데이터 (청크가 아닌 경우) - 직접 큐에 추가
            if (xSemaphoreTake(queueMutex, portMAX_DELAY) == pdTRUE) {
                typingQueue.push(receivedText);
                xSemaphoreGive(queueMutex);
                DEBUG_PRINTLN("일반 텍스트 큐에 추가됨");
            }
            
            // 응답 전송
            if (pTxCharacteristic && deviceConnected) {
                String response = "OK:Queued for typing";
                pTxCharacteristic->setValue(response.c_str());
                pTxCharacteristic->notify();
            }
        }
    }
};

// 타이핑 작업 처리
void processTypingQueue() {
    if (xSemaphoreTake(queueMutex, 0) == pdTRUE) {
        if (!typingQueue.empty() && !isTyping) {
            String text = typingQueue.front();
            typingQueue.pop();
            xSemaphoreGive(queueMutex);
            
            // 타이핑 시작
            isTyping = true;
            DEBUG_PRINT("타이핑 시작: ");
            DEBUG_PRINTLN(text);
            
            // JSON 또는 일반 텍스트 파싱
            String textToType = "";
            int speed_cps = globalTypingSpeed; // 웹에서 전달받은 속도만 사용
            
            // JSON 파싱 시도
            if (text.startsWith("{")) {
                StaticJsonDocument<8192> doc; // 크기를 8KB로 증가
                DeserializationError error = deserializeJson(doc, text);
                
                if (!error && doc.containsKey("text")) {
                    textToType = doc["text"].as<String>();
                    if (doc.containsKey("speed_cps")) {
                        speed_cps = doc["speed_cps"];
                        globalTypingSpeed = speed_cps; // 전역 속도도 업데이트
                        DEBUG_PRINT("JSON에서 타이핑 속도 업데이트: ");
                        DEBUG_PRINTLN(speed_cps);
                    }
                } else {
                    textToType = text; // JSON 파싱 실패시 원본 텍스트 사용
                }
            } else if (text.startsWith("GHTYPE_CFG:")) {
                // 설정 프로토콜 처리
                String configJson = text.substring(11);
                StaticJsonDocument<256> configDoc;
                DeserializationError configError = deserializeJson(configDoc, configJson);
                
                if (!configError && configDoc.containsKey("speed_cps")) {
                    globalTypingSpeed = configDoc["speed_cps"];
                    speed_cps = globalTypingSpeed;
                    DEBUG_PRINT("타이핑 속도 설정: ");
                    DEBUG_PRINTLN(globalTypingSpeed);
                }
                textToType = ""; // 설정만 처리하고 타이핑 없음
            } else if (text.startsWith("GHTYPE_")) {
                // 레거시 형식 지원
                if (text.startsWith("GHTYPE_KOR:")) {
                    textToType = text.substring(11);
                } else if (text.startsWith("GHTYPE_ENG:")) {
                    textToType = text.substring(11);
                } else if (text.startsWith("GHTYPE_SPE:haneng")) {
                    // 한영 전환 - Alt+Shift 조합
                    keyboard.press(KEY_LEFT_ALT);
                    delay(10);
                    keyboard.press(KEY_LEFT_SHIFT);
                    delay(10);
                    keyboard.release(KEY_LEFT_SHIFT);
                    keyboard.release(KEY_LEFT_ALT);
                    delay(50);
                    textToType = ""; // 타이핑할 텍스트 없음
                } else {
                    textToType = text;
                }
            } else {
                textToType = text; // 일반 텍스트
            }
            
            // 실제 타이핑
            if (textToType.length() > 0) {
                int delay_ms = 1000 / speed_cps; // 속도에 따른 딜레이 계산
                
                DEBUG_PRINT("=== 타이핑 시작 ===");
                DEBUG_PRINT("타이핑할 텍스트: '");
                DEBUG_PRINT(textToType);
                DEBUG_PRINTLN("'");
                DEBUG_PRINT("텍스트 길이: ");
                DEBUG_PRINTLN(textToType.length());
                DEBUG_PRINT("타이핑 속도: ");
                DEBUG_PRINT(speed_cps);
                DEBUG_PRINT(" CPS, 딜레이: ");
                DEBUG_PRINT(delay_ms);
                DEBUG_PRINTLN("ms");
                DEBUG_PRINTLN("=== 문자별 처리 시작 ===");
                
                for (int i = 0; i < textToType.length(); i++) {
                    char c = textToType[i];
                    
                    DEBUG_PRINT("문자 ");
                    DEBUG_PRINT(i);
                    DEBUG_PRINT(": ASCII ");
                    DEBUG_PRINT((int)c);
                    DEBUG_PRINT(" (");
                    if (c == '\n') {
                        DEBUG_PRINT("엔터키");
                    } else if (c == '\r') {
                        DEBUG_PRINT("캐리지 리턴");
                    } else if (c == '\t') {
                        DEBUG_PRINT("탭");
                    } else if (c >= 32 && c <= 126) {
                        DEBUG_PRINT(c);
                    } else {
                        DEBUG_PRINT("특수문자");
                    }
                    DEBUG_PRINTLN(")");
                    
                    // 특수 문자 처리
                    if (c == '\n' || c == '\r') {
                        // 엔터키 - 더 많은 딜레이 추가
                        DEBUG_PRINTLN("엔터키 입력!");
                        delay(50); // 엔터키 전 딜레이 증가
                        keyboard.press(KEY_RETURN);
                        delay(100); // 엔터키 누름 딜레이 대폭 증가
                        keyboard.release(KEY_RETURN);
                        delay(100); // 엔터키 후 딜레이 대폭 증가
                    } else if (c == '\t') {
                        // 탭키
                        DEBUG_PRINTLN("탭키 입력!");
                        keyboard.press(KEY_TAB);
                        delay(50);
                        keyboard.release(KEY_TAB);
                        delay(50);
                    } else {
                        // 일반 문자
                        keyboard.write(c);
                        delay(delay_ms); // 타이핑 속도 조절
                    }
                }
            }
            
            DEBUG_PRINTLN("타이핑 완료!");
            isTyping = false;
            lastTypeTime = millis();
            
            // 완료 응답 전송
            if (pTxCharacteristic && deviceConnected) {
                String response = "OK:Typing completed";
                pTxCharacteristic->setValue(response.c_str());
                pTxCharacteristic->notify();
            }
            
        } else {
            xSemaphoreGive(queueMutex);
        }
    }
}

// BLE 초기화 태스크
void bleTask(void * parameter) {
    // BLE 초기화 - JavaScript와 일치
    BLEDevice::init("GHOSTYPE");
    
    // MTU 크기 설정 (안정성을 위해 적절한 크기로 조정)
    BLEDevice::setMTU(247); // 안정성과 호환성을 위해 247로 설정
    
    // 보안 비활성화
    esp_ble_auth_req_t auth_req = ESP_LE_AUTH_NO_BOND;
    esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;
    uint8_t key_size = 16;
    uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    uint8_t rsp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, sizeof(uint8_t));
    
    // 서버 생성
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    
    // 서비스 생성
    BLEService *pService = pServer->createService(SERVICE_UUID);
    
    // RX 특성
    pRxCharacteristic = pService->createCharacteristic(
        RX_CHAR_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    pRxCharacteristic->setCallbacks(new MyCallbacks());
    
    // TX 특성
    pTxCharacteristic = pService->createCharacteristic(
        TX_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    pTxCharacteristic->addDescriptor(new BLE2902());
    
    // 서비스 시작
    pService->start();
    
    // 광고 시작
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMaxPreferred(0x12);
    BLEDevice::startAdvertising();
    
    DEBUG_PRINTLN("BLE 태스크 시작됨!");
    
    // BLE 태스크 루프
    while(1) {
        // BLE 상태 체크
        static unsigned long lastCheck = 0;
        if (millis() - lastCheck > 10000) {
            DEBUG_PRINT("BLE 상태: ");
            DEBUG_PRINTLN(deviceConnected ? "연결됨" : "대기중");
            lastCheck = millis();
        }
        
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void setup() {
    #if DEBUG_ENABLED
    Serial.begin(115200);
    delay(2000);
    #endif
    
    DEBUG_PRINTLN("\n=== GHOSTYPE 실시간 BLE + HID ===");
    DEBUG_PRINTLN("BLE로 받은 텍스트를 즉시 USB 키보드로 타이핑합니다.");
    
    // 큐 뮤텍스 생성
    queueMutex = xSemaphoreCreateMutex();
    
    // USB HID 초기화
    DEBUG_PRINTLN("1. USB HID 키보드 초기화...");
    USB.begin();
    keyboard.begin();
    DEBUG_PRINTLN("   ✓ HID 초기화 완료");
    
    // BLE를 별도 태스크로 실행
    DEBUG_PRINTLN("2. BLE 태스크 생성...");
    xTaskCreatePinnedToCore(
        bleTask,          // 태스크 함수
        "BLE_Task",       // 태스크 이름
        8192,             // 스택 크기
        NULL,             // 파라미터
        1,                // 우선순위
        NULL,             // 태스크 핸들
        0                 // CPU 코어 (0번 코어)
    );
    
    DEBUG_PRINTLN("   ✓ BLE 태스크 생성 완료");
    DEBUG_PRINTLN("\n준비 완료! BLE 연결을 기다립니다...\n");
}

void loop() {
    // 메인 루프는 HID 타이핑 처리에 집중
    
    // 타이핑 큐 처리
    if (!typingQueue.empty()) {
        // 이전 타이핑 완료 후 약간의 딜레이
        if (millis() - lastTypeTime > 100) {
            processTypingQueue();
        }
    }
    
    // CPU 부하 감소
    delay(10);
}