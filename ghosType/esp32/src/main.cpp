#include <Arduino.h>
#include <NimBLEDevice.h>

// 버튼 핀
#define BUTTON_PIN 0

// BLE 설정
BLEServer* pServer = nullptr;
BLECharacteristic* pCharacteristic = nullptr;
bool deviceConnected = false;
bool oldDeviceConnected = false;

#define SERVICE_UUID        "12345678-1234-5678-9012-123456789abc"
#define CHARACTERISTIC_UUID "87654321-4321-8765-2109-cba9876543210"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        Serial.println("BLE Client Connected!");
    };

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        Serial.println("BLE Client Disconnected!");
    }
};

void setup() {
    // 시리얼 초기화
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("=== T-Dongle-S3 BLE Test ===");
    Serial.println("LCD 없이 BLE 기능만 테스트합니다");
    
    // 버튼 설정
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    
    // BLE 초기화
    Serial.println("Initializing BLE...");
    BLEDevice::init("GHOSTYPE-S3");
    
    // BLE 서버 생성
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    
    // 서비스 생성
    BLEService *pService = pServer->createService(SERVICE_UUID);
    
    // 특성 생성
    pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      NIMBLE_PROPERTY::READ |
                      NIMBLE_PROPERTY::WRITE |
                      NIMBLE_PROPERTY::NOTIFY
                    );
    
    pCharacteristic->setValue("Hello GHOSTYPE");
    
    // 서비스 시작
    pService->start();
    
    // 광고 시작
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    
    Serial.println("BLE Ready! Device name: GHOSTYPE-S3");
    Serial.println("Waiting for client connection...");
}

void loop() {
    static bool buttonPressed = false;
    static int buttonCount = 0;
    
    // 버튼 확인
    if (digitalRead(BUTTON_PIN) == LOW && !buttonPressed) {
        buttonPressed = true;
        buttonCount++;
        
        Serial.print("Button pressed! Count: ");
        Serial.println(buttonCount);
        
        // BLE로 버튼 카운트 전송
        if (deviceConnected) {
            String value = "Button: " + String(buttonCount);
            pCharacteristic->setValue(value.c_str());
            pCharacteristic->notify();
            Serial.println("Sent to BLE: " + value);
        }
        
    } else if (digitalRead(BUTTON_PIN) == HIGH) {
        buttonPressed = false;
    }
    
    // 연결 상태 변경 처리
    if (!deviceConnected && oldDeviceConnected) {
        delay(500);
        pServer->startAdvertising();
        Serial.println("Start advertising again");
        oldDeviceConnected = deviceConnected;
    }
    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
    }
    
    delay(50);
}