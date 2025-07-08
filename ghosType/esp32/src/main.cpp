#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define BUTTON_PIN 0

BLEServer* pServer = NULL;
BLECharacteristic* pRxCharacteristic = NULL;  // 수신용
BLECharacteristic* pTxCharacteristic = NULL;  // 송신용
bool deviceConnected = false;

#define SERVICE_UUID        "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define RX_CHAR_UUID        "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
#define TX_CHAR_UUID        "6e400003-b5a3-f393-e0a9-e50e24dcca9e"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        Serial.println("🔗 *** BLE DEVICE CONNECTED! ***");
        Serial.println("🎉 연결 성공! 클라이언트가 연결되었습니다!");
    };

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        Serial.println("❌ *** BLE DEVICE DISCONNECTED ***");
        Serial.println("📱 클라이언트 연결이 해제되었습니다");
        
        // 광고 재시작
        delay(500);
        pServer->getAdvertising()->start();
        Serial.println("🔄 광고 재시작됨 - 다시 연결 가능");
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
    
    // Web Bluetooth 호환을 위한 기본 보안 설정
    BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT_NO_MITM);
    
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
    
    // RX 특성 생성 (웹 → ESP32)
    Serial.println("4. RX 특성 생성...");
    pRxCharacteristic = pService->createCharacteristic(
                        RX_CHAR_UUID,
                        BLECharacteristic::PROPERTY_WRITE
                      );
    Serial.println("   ✓ RX 특성 생성 완료");
    
    // TX 특성 생성 (ESP32 → 웹)
    Serial.println("5. TX 특성 생성...");
    pTxCharacteristic = pService->createCharacteristic(
                        TX_CHAR_UUID,
                        BLECharacteristic::PROPERTY_READ |
                        BLECharacteristic::PROPERTY_NOTIFY
                      );
    
    // Web Bluetooth 호환을 위한 Descriptor 추가
    pTxCharacteristic->addDescriptor(new BLE2902());
    Serial.println("   ✓ TX 특성 생성 완료");
    
    // 서비스 시작
    Serial.println("6. BLE 서비스 시작...");
    pService->start();
    Serial.println("   ✓ BLE 서비스 시작 완료");
    
    // 광고 시작
    Serial.println("7. BLE 광고 시작...");
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // 연결 간격 최적화
    pAdvertising->setMaxPreferred(0x12);
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
    
    // 5초마다 상태 확인 (더 자주)
    if (millis() - lastStatus > 5000) {
        Serial.print("📡 BLE 상태 체크 - 연결됨: ");
        Serial.print(deviceConnected ? "YES ✅" : "NO ❌");
        Serial.print(" | 버튼 카운트: ");
        Serial.print(buttonCount);
        Serial.println(" | 연결 대기 중...");
        lastStatus = millis();
    }
    
    // 버튼 처리
    if (digitalRead(BUTTON_PIN) == LOW && !buttonPressed) {
        buttonPressed = true;
        buttonCount++;
        
        Serial.print("🔘 버튼 눌림! 카운트: ");
        Serial.println(buttonCount);
        
        // BLE로 데이터 전송
        if (deviceConnected && pTxCharacteristic) {
            String msg = "Button count: " + String(buttonCount);
            pTxCharacteristic->setValue(msg.c_str());
            pTxCharacteristic->notify();
            Serial.println("   📤 BLE로 데이터 전송됨");
        }
        
    } else if (digitalRead(BUTTON_PIN) == HIGH) {
        buttonPressed = false;
    }
    
    delay(50);
}