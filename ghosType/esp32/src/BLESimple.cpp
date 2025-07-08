#include "BLESimple.h"
#include "BLEConfig.h"

// 간단한 BLE 서버 구현 (문제 해결용)
// Simple BLE server implementation (for troubleshooting)

BLESimple* BLESimple::instance = nullptr;

BLESimple::BLESimple() : 
    deviceConnected(false),
    receivedData("") {
    instance = this;
}

bool BLESimple::begin() {
    // 기본 BLE 초기화
    Serial.println("  1. NimBLE 장치 초기화...");
    NimBLEDevice::init("ESP32");
    Serial.println("  ✅ NimBLE 장치 초기화 완료");
    
    // 서버 생성
    Serial.println("  2. BLE 서버 생성...");
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());
    Serial.println("  ✅ BLE 서버 생성 완료");
    
    // 서비스 생성  
    Serial.println("  3. BLE 서비스 생성...");
    NimBLEService* pService = pServer->createService("12345678-1234-5678-9012-123456789abc");
    Serial.println("  ✅ BLE 서비스 생성 완료");
    
    // RX 특성 (쓰기용)
    Serial.println("  4. RX 특성 생성...");
    pCharacteristicRX = pService->createCharacteristic(
        "12345678-1234-5678-9012-123456789abd",
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
    );
    pCharacteristicRX->setCallbacks(new CharacteristicCallbacks());
    Serial.println("  ✅ RX 특성 생성 완료");
    
    // TX 특성 (알림용)
    Serial.println("  5. TX 특성 생성...");
    pCharacteristicTX = pService->createCharacteristic(
        "12345678-1234-5678-9012-123456789abe",
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );
    Serial.println("  ✅ TX 특성 생성 완료");
    
    // 서비스 시작
    Serial.println("  6. 서비스 시작...");
    pService->start();
    Serial.println("  ✅ 서비스 시작 완료");
    
    // 광고 시작
    Serial.println("  7. 광고 설정 및 시작...");
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID("12345678-1234-5678-9012-123456789abc");
    pAdvertising->setName("ESP32");
    pAdvertising->start();
    Serial.println("  ✅ 광고 시작 완료");
    
    Serial.println("🎉 BLE 초기화 모든 단계 완료!");
    return true;
}

void BLESimple::stop() {
    if (pServer) {
        pServer->getAdvertising()->stop();
    }
    NimBLEDevice::deinit(true);
}

bool BLESimple::hasReceivedData() {
    return !receivedData.empty();
}

std::string BLESimple::getReceivedData() {
    std::string data = receivedData;
    receivedData.clear();
    return data;
}

void BLESimple::sendNotification(const char* data) {
    if (deviceConnected && pCharacteristicTX) {
        pCharacteristicTX->setValue(data);
        pCharacteristicTX->notify();
    }
}

bool BLESimple::isConnected() {
    return deviceConnected;
}

// 서버 콜백
void BLESimple::ServerCallbacks::onConnect(NimBLEServer* pServer) {
    if (BLESimple::instance) {
        BLESimple::instance->deviceConnected = true;
        Serial.println("🔗 서버 콜백: 클라이언트 연결됨");
    }
}

void BLESimple::ServerCallbacks::onDisconnect(NimBLEServer* pServer) {
    if (BLESimple::instance) {
        BLESimple::instance->deviceConnected = false;
        Serial.println("❌ 서버 콜백: 클라이언트 연결 해제됨");
    }
    Serial.println("🔄 광고 재시작...");
    delay(500);
    pServer->getAdvertising()->start();
}

// 특성 콜백
void BLESimple::CharacteristicCallbacks::onWrite(NimBLECharacteristic* pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (BLESimple::instance && value.length() > 0) {
        Serial.printf("📝 특성 콜백: 데이터 수신 (길이: %d)\n", value.length());
        BLESimple::instance->receivedData = value;
    }
}