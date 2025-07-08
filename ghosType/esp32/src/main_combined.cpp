/**
 * @file main_combined.cpp
 * @brief GHOSTYPE BLE + HID 통합 버전
 * 
 * 부팅 시 BOOT 버튼으로 모드 선택:
 * - 버튼 안 누름: BLE 모드
 * - 버튼 누름: HID 모드
 */

#include <Arduino.h>
#include <USB.h>
#include <USBHIDKeyboard.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <esp_gap_ble_api.h>

#define BUTTON_PIN 0
#define MODE_SELECT_TIME 3000  // 3초 동안 모드 선택

// 동작 모드
enum OperationMode {
    MODE_BLE,
    MODE_HID
};

OperationMode currentMode = MODE_BLE;

// HID 키보드 객체
USBHIDKeyboard keyboard;

// BLE 객체들
BLEServer* pServer = NULL;
BLECharacteristic* pRxCharacteristic = NULL;
BLECharacteristic* pTxCharacteristic = NULL;
bool deviceConnected = false;

// BLE UUID
#define SERVICE_UUID        "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define RX_CHAR_UUID        "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
#define TX_CHAR_UUID        "6e400003-b5a3-f393-e0a9-e50e24dcca9e"

// 함수 선언
void initBLEMode();
void initHIDMode();
void processBLECommand(std::string command);
void typeTextHID(const char* text);

// BLE 콜백들
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

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string rxValue = pCharacteristic->getValue();
        if (rxValue.length() > 0) {
            Serial.print("BLE 수신: ");
            Serial.println(rxValue.c_str());
            processBLECommand(rxValue);
        }
    }
};

void setup() {
    Serial.begin(115200);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    
    // 모드 선택 대기
    Serial.println("\n=== GHOSTYPE 부팅 중 ===");
    Serial.println("3초 내에 BOOT 버튼을 누르면 HID 모드");
    Serial.println("누르지 않으면 BLE 모드로 시작합니다...");
    
    unsigned long startTime = millis();
    bool buttonPressed = false;
    
    while (millis() - startTime < MODE_SELECT_TIME) {
        if (digitalRead(BUTTON_PIN) == LOW) {
            buttonPressed = true;
            break;
        }
        delay(50);
    }
    
    if (buttonPressed) {
        currentMode = MODE_HID;
        Serial.println("\n>>> HID 모드 선택됨!");
        initHIDMode();
    } else {
        currentMode = MODE_BLE;
        Serial.println("\n>>> BLE 모드 선택됨!");
        initBLEMode();
    }
}

void loop() {
    if (currentMode == MODE_BLE) {
        // BLE 모드 동작
        static unsigned long lastStatus = 0;
        if (millis() - lastStatus > 5000) {
            Serial.print("BLE 상태: ");
            Serial.println(deviceConnected ? "연결됨" : "대기중");
            lastStatus = millis();
        }
        
    } else {
        // HID 모드 동작
        static bool buttonPressed = false;
        
        if (digitalRead(BUTTON_PIN) == LOW && !buttonPressed) {
            buttonPressed = true;
            Serial.println("HID 테스트 타이핑...");
            typeTextHID("GHOSTYPE HID Mode Active!");
            
        } else if (digitalRead(BUTTON_PIN) == HIGH) {
            buttonPressed = false;
        }
    }
    
    delay(50);
}

void initBLEMode() {
    // BLE 초기화
    BLEDevice::init("GHOSTYPE-S3");
    
    // 보안 설정
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
    
    Serial.println("BLE 초기화 완료! 연결 대기 중...");
}

void initHIDMode() {
    // USB HID 초기화
    USB.begin();
    keyboard.begin();
    
    Serial.println("HID 키보드 초기화 완료!");
    Serial.println("BOOT 버튼을 누르면 테스트 텍스트를 타이핑합니다.");
}

void processBLECommand(std::string command) {
    // BLE로 받은 텍스트를 저장만 (HID 모드에서 사용)
    Serial.println("BLE 명령 처리 (시뮬레이션)");
    
    if (pTxCharacteristic && deviceConnected) {
        String response = "OK:BLE Mode - HID disabled";
        pTxCharacteristic->setValue(response.c_str());
        pTxCharacteristic->notify();
    }
}

void typeTextHID(const char* text) {
    delay(500);
    
    while (*text) {
        keyboard.write(*text);
        Serial.print(*text);
        text++;
        delay(50);
    }
    
    keyboard.write(KEY_RETURN);
    Serial.println("\n타이핑 완료!");
}