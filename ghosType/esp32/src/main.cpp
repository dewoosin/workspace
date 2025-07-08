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

// BLE 서버 콜백
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        Serial.println("BLE 연결됨!");
    }
    
    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        Serial.println("BLE 연결 해제됨");
        delay(500);
        pServer->getAdvertising()->start();
    }
};

// BLE 데이터 수신 콜백
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string rxValue = pCharacteristic->getValue();
        
        if (rxValue.length() > 0) {
            Serial.print("BLE 수신: ");
            Serial.println(rxValue.c_str());
            
            // 뮤텍스로 큐 보호
            if (xSemaphoreTake(queueMutex, portMAX_DELAY) == pdTRUE) {
                // 받은 텍스트를 큐에 추가
                typingQueue.push(String(rxValue.c_str()));
                xSemaphoreGive(queueMutex);
                
                Serial.println("텍스트 큐에 추가됨");
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
            Serial.print("타이핑 시작: ");
            Serial.println(text);
            
            // 텍스트 파싱 및 타이핑
            if (text.startsWith("GHTYPE_KOR:")) {
                text = text.substring(11);
            } else if (text.startsWith("GHTYPE_ENG:")) {
                text = text.substring(11);
            }
            
            // 실제 타이핑
            for (int i = 0; i < text.length(); i++) {
                keyboard.write(text[i]);
                delay(30); // 타이핑 속도 조절
            }
            
            // Enter 키
            keyboard.write(KEY_RETURN);
            
            Serial.println("타이핑 완료!");
            isTyping = false;
            lastTypeTime = millis();
            
        } else {
            xSemaphoreGive(queueMutex);
        }
    }
}

// BLE 초기화 태스크
void bleTask(void * parameter) {
    // BLE 초기화
    BLEDevice::init("GHOSTYPE-S3");
    
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
    
    Serial.println("BLE 태스크 시작됨!");
    
    // BLE 태스크 루프
    while(1) {
        // BLE 상태 체크
        static unsigned long lastCheck = 0;
        if (millis() - lastCheck > 10000) {
            Serial.print("BLE 상태: ");
            Serial.println(deviceConnected ? "연결됨" : "대기중");
            lastCheck = millis();
        }
        
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n=== GHOSTYPE 실시간 BLE + HID ===");
    Serial.println("BLE로 받은 텍스트를 즉시 USB 키보드로 타이핑합니다.");
    
    // 큐 뮤텍스 생성
    queueMutex = xSemaphoreCreateMutex();
    
    // USB HID 초기화
    Serial.println("1. USB HID 키보드 초기화...");
    USB.begin();
    keyboard.begin();
    Serial.println("   ✓ HID 초기화 완료");
    
    // BLE를 별도 태스크로 실행
    Serial.println("2. BLE 태스크 생성...");
    xTaskCreatePinnedToCore(
        bleTask,          // 태스크 함수
        "BLE_Task",       // 태스크 이름
        8192,             // 스택 크기
        NULL,             // 파라미터
        1,                // 우선순위
        NULL,             // 태스크 핸들
        0                 // CPU 코어 (0번 코어)
    );
    
    Serial.println("   ✓ BLE 태스크 생성 완료");
    Serial.println("\n준비 완료! BLE 연결을 기다립니다...\n");
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