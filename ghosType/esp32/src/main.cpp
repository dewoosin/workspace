#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define BUTTON_PIN 0

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;

#define SERVICE_UUID        "12345678-1234-5678-9012-123456789abc"
#define CHARACTERISTIC_UUID "87654321-4321-8765-2109-cba987654321"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        Serial.println("*** BLE DEVICE CONNECTED! ***");
    };

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        Serial.println("*** BLE DEVICE DISCONNECTED ***");
    }
};

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n\n=================================");
    Serial.println("!!!! ESP32 NATIVE BLE TEST !!!!");
    Serial.println("=================================");
    
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    
    // BLE 초기화
    Serial.println("1. BLE 초기화 시작...");
    BLEDevice::init("GHOSTYPE-S3");
    Serial.println("   ✓ BLE 장치 초기화 완료");
    
    // 서버 생성
    Serial.println("2. BLE 서버 생성...");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    Serial.println("   ✓ BLE 서버 생성 완료");
    
    // 서비스 생성
    Serial.println("3. BLE 서비스 생성...");
    BLEService *pService = pServer->createService(SERVICE_UUID);
    Serial.println("   ✓ BLE 서비스 생성 완료");
    
    // 특성 생성
    Serial.println("4. BLE 특성 생성...");
    pCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID,
                        BLECharacteristic::PROPERTY_READ |
                        BLECharacteristic::PROPERTY_WRITE |
                        BLECharacteristic::PROPERTY_NOTIFY
                      );
    Serial.println("   ✓ BLE 특성 생성 완료");
    
    // 서비스 시작
    Serial.println("5. BLE 서비스 시작...");
    pService->start();
    Serial.println("   ✓ BLE 서비스 시작 완료");
    
    // 광고 시작
    Serial.println("6. BLE 광고 시작...");
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0);
    BLEDevice::startAdvertising();
    Serial.println("   ✓ BLE 광고 시작 완료");
    
    Serial.println("\n🎉 BLE 초기화 모든 단계 완료! 🎉");
    Serial.println("장치명: GHOSTYPE-S3");
    Serial.println("이제 블루투스 검색해보세요!");
    Serial.println("=================================\n");
}

void loop() {
    static bool buttonPressed = false;
    static int buttonCount = 0;
    static unsigned long lastStatus = 0;
    
    // 10초마다 상태 확인
    if (millis() - lastStatus > 10000) {
        Serial.print("📡 BLE 상태 체크 - 연결됨: ");
        Serial.print(deviceConnected ? "YES" : "NO");
        Serial.print(" | 버튼 카운트: ");
        Serial.println(buttonCount);
        lastStatus = millis();
    }
    
    // 버튼 처리
    if (digitalRead(BUTTON_PIN) == LOW && !buttonPressed) {
        buttonPressed = true;
        buttonCount++;
        
        Serial.print("🔘 버튼 눌림! 카운트: ");
        Serial.println(buttonCount);
        
        // BLE로 데이터 전송
        if (deviceConnected) {
            String msg = "Button count: " + String(buttonCount);
            pCharacteristic->setValue(msg.c_str());
            pCharacteristic->notify();
            Serial.println("   📤 BLE로 데이터 전송됨");
        }
        
    } else if (digitalRead(BUTTON_PIN) == HIGH) {
        buttonPressed = false;
    }
    
    delay(50);
}