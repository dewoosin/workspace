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
    NimBLEDevice::init("ESP32");
    
    // 서버 생성
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());
    
    // 서비스 생성  
    NimBLEService* pService = pServer->createService("12345678-1234-5678-9012-123456789abc");
    
    // RX 특성 (쓰기용)
    pCharacteristicRX = pService->createCharacteristic(
        "12345678-1234-5678-9012-123456789abd",
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
    );
    pCharacteristicRX->setCallbacks(new CharacteristicCallbacks());
    
    // TX 특성 (알림용)
    pCharacteristicTX = pService->createCharacteristic(
        "12345678-1234-5678-9012-123456789abe",
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );
    
    // 서비스 시작
    pService->start();
    
    // 광고 시작
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID("12345678-1234-5678-9012-123456789abc");
    pAdvertising->setName("ESP32");
    pAdvertising->start();
    
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
    }
}

void BLESimple::ServerCallbacks::onDisconnect(NimBLEServer* pServer) {
    if (BLESimple::instance) {
        BLESimple::instance->deviceConnected = false;
    }
    delay(500);
    pServer->getAdvertising()->start();
}

// 특성 콜백
void BLESimple::CharacteristicCallbacks::onWrite(NimBLECharacteristic* pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (BLESimple::instance && value.length() > 0) {
        BLESimple::instance->receivedData = value;
    }
}