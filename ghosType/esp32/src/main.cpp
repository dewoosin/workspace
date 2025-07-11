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
int globalTypingSpeed = 10; // 웹 기본값과 동일 (selected option)

// 언어 상태 관리 - 항상 영문 상태로 시작한다고 가정
bool isKoreanMode = false; // 현재 입력 언어 모드 (false = English, true = Korean)
bool isInitialized = false; // 초기화 상태 확인용

// 한글 키 코드 정의 (HID 키코드)
#ifndef KEY_HANGUL
#define KEY_HANGUL 0x90  // 한글/영문 전환 키 (HID_KEY_LANG1)
#endif


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

// 언어 전환 함수 - 초기 상태 강제 설정
void toggleToKoreanMode() {
    // 첫 번째 명령이면 항상 영문 상태에서 시작한다고 가정
    if (!isInitialized) {
        DEBUG_PRINTLN("초기화: 영문 모드로 강제 설정");
        isKoreanMode = false;
        isInitialized = true;
    }
    
    if (!isKoreanMode) {
        DEBUG_PRINTLN("한영 전환: 영문 → 한글");
        
        // 한/영 전환 키 전송 (오른쪽 Alt 키 - 한국 두벌식 키보드)
        keyboard.press(KEY_RIGHT_ALT);
        delay(50);
        keyboard.releaseAll();
        delay(800); // IME 전환 완료 대기
        
        isKoreanMode = true;
    } else {
        DEBUG_PRINTLN("이미 한글 모드입니다.");
    }
}

void toggleToEnglishMode() {
    // 첫 번째 명령이면 항상 영문 상태에서 시작한다고 가정
    if (!isInitialized) {
        DEBUG_PRINTLN("초기화: 영문 모드로 강제 설정");
        isKoreanMode = false;
        isInitialized = true;
        return; // 이미 영문 모드이므로 전환 불필요
    }
    
    if (isKoreanMode) {
        DEBUG_PRINTLN("한영 전환: 한글 → 영문");
        
        // 한/영 전환 키 전송 (오른쪽 Alt 키 - 한국 두벌식 키보드)
        keyboard.press(KEY_RIGHT_ALT);
        delay(50);
        keyboard.releaseAll();
        delay(800); // IME 전환 완료 대기
        
        isKoreanMode = false;
    } else {
        DEBUG_PRINTLN("이미 영문 모드입니다.");
    }
}

// 프로토콜 명령어 처리
bool processProtocolCommand(const String& line) {
    String trimmedLine = line;
    trimmedLine.trim();
    
    if (trimmedLine.length() == 0) {
        return true; // 빈 줄 무시
    }
    
    DEBUG_PRINT("프로토콜 명령 처리: ");
    DEBUG_PRINTLN(trimmedLine);
    
    // 언어 전환 명령어
    if (trimmedLine.equals("#CMD:HANGUL")) {
        toggleToKoreanMode();
        return true;
    }
    
    if (trimmedLine.equals("#CMD:ENGLISH")) {
        toggleToEnglishMode();
        return true;
    }
    
    // 엔터키 명령어
    if (trimmedLine.equals("#CMD:ENTER")) {
        DEBUG_PRINTLN("엔터키 입력");
        keyboard.press(KEY_RETURN);
        delay(100);
        keyboard.release(KEY_RETURN);
        delay(100);
        return true;
    }
    
    // 텍스트 입력 명령어
    if (trimmedLine.startsWith("#TEXT:")) {
        String textContent = trimmedLine.substring(6); // "#TEXT:" 제거
        
        DEBUG_PRINT("텍스트 입력: \"");
        DEBUG_PRINT(textContent);
        DEBUG_PRINTLN("\"");
        
        // 타이핑 속도 계산
        int delay_ms = 1000 / globalTypingSpeed;
        
        // 문자별 타이핑 (엔터키는 #CMD:ENTER로 별도 처리)
        for (int i = 0; i < textContent.length(); i++) {
            char c = textContent[i];
            
            // 특수 문자 개별 처리 (엔터키 제외)
            if (c == '\t') {
                keyboard.press(KEY_TAB);
                delay(50);
                keyboard.release(KEY_TAB);
                delay(50);
            } else {
                // 일반 문자 타이핑 (엔터키는 무시, #CMD:ENTER로 처리됨)
                if (c != '\n' && c != '\r') {
                    // 스페이스바 특별 처리 - 더 긴 딜레이
                    if (c == ' ') {
                        DEBUG_PRINTLN("스페이스바 입력");
                        keyboard.write(c);
                        delay(delay_ms + 50); // 스페이스바는 더 긴 딜레이
                    } else {
                        keyboard.write(c);
                        delay(delay_ms);
                    }
                }
            }
        }
        
        return true;
    }
    
    // 알 수 없는 명령어 - 무시하고 계속
    DEBUG_PRINT("알 수 없는 프로토콜 명령: ");
    DEBUG_PRINTLN(trimmedLine);
    return true;
}

// 구조화된 프로토콜 처리
void processStructuredProtocol(const String& text) {
    DEBUG_PRINTLN("구조화된 프로토콜 모드로 처리 시작");
    
    // 줄별로 분리하여 처리
    int startPos = 0;
    int lineNumber = 1;
    
    while (startPos < text.length()) {
        int endPos = text.indexOf('\n', startPos);
        if (endPos == -1) {
            endPos = text.length();
        }
        
        String line = text.substring(startPos, endPos);
        
        DEBUG_PRINT("라인 ");
        DEBUG_PRINT(lineNumber);
        DEBUG_PRINT(": ");
        DEBUG_PRINTLN(line);
        
        processProtocolCommand(line);
        
        startPos = endPos + 1;
        lineNumber++;
        
        // 명령어 간 약간의 딜레이
        delay(10);
    }
    
    DEBUG_PRINTLN("구조화된 프로토콜 처리 완료");
}


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
            DEBUG_PRINTLN(rxValue.c_str());
            
            // 수신 데이터의 각 바이트를 확인
            DEBUG_PRINT("수신 바이트: ");
            for (size_t i = 0; i < rxValue.length(); i++) {
                DEBUG_PRINT((int)rxValue[i]);
                DEBUG_PRINT(" ");
            }
            DEBUG_PRINTLN();
            
            // 뮤텍스로 큐 보호
            if (xSemaphoreTake(queueMutex, portMAX_DELAY) == pdTRUE) {
                // 받은 텍스트를 큐에 추가
                typingQueue.push(String(rxValue.c_str()));
                xSemaphoreGive(queueMutex);
                
                DEBUG_PRINTLN("텍스트 큐에 추가됨");
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
            
            // 새로운 구조화된 프로토콜 감지
            if (text.startsWith("#CMD:") || text.indexOf("\n#CMD:") != -1 || text.indexOf("\n#TEXT:") != -1) {
                DEBUG_PRINTLN("구조화된 프로토콜 감지됨");
                processStructuredProtocol(text);
                
                // 완료 응답 전송
                if (pTxCharacteristic && deviceConnected) {
                    String response = "OK:Structured protocol completed";
                    pTxCharacteristic->setValue(response.c_str());
                    pTxCharacteristic->notify();
                }
                
                DEBUG_PRINTLN("구조화된 프로토콜 타이핑 완료!");
                isTyping = false;
                lastTypeTime = millis();
                return; // 조기 반환하여 기존 로직 건너뛰기
            }
            
            // 기존 JSON 또는 일반 텍스트 파싱 (호환성 유지)
            String textToType = "";
            int speed_cps = globalTypingSpeed; // 웹에서 전달받은 속도만 사용
            
            // JSON 파싱 시도
            if (text.startsWith("{")) {
                StaticJsonDocument<512> doc;
                DeserializationError error = deserializeJson(doc, text);
                
                if (!error && doc.containsKey("text")) {
                    textToType = doc["text"].as<String>();
                    if (doc.containsKey("speed_cps")) {
                        speed_cps = doc["speed_cps"];
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
                
                DEBUG_PRINT("타이핑할 텍스트: ");
                DEBUG_PRINTLN(textToType);
                DEBUG_PRINT("텍스트 길이: ");
                DEBUG_PRINTLN(textToType.length());
                DEBUG_PRINT("타이핑 속도: ");
                DEBUG_PRINT(speed_cps);
                DEBUG_PRINT(" CPS, 딜레이: ");
                DEBUG_PRINT(delay_ms);
                DEBUG_PRINTLN("ms");
                
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
    
    // MTU 크기 설정 (기본값보다 작게 설정)
    BLEDevice::setMTU(185);
    
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