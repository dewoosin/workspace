#include "BLENimbleManager.h"
#include "BLEConfig.h"

class BLENimbleManager::ServerCallbacks : public NimBLEServerCallbacks {
private:
    BLENimbleManager* manager;
    
public:
    ServerCallbacks(BLENimbleManager* mgr) : manager(mgr) {}
    
    void onConnect(NimBLEServer* pServer) {
        manager->deviceConnected = true;
        // Production build - no serial logging
        // 프로덕션 빌드 - 시리얼 로깅 없음
    }
    
    void onDisconnect(NimBLEServer* pServer) {
        manager->deviceConnected = false;
        // Production build - no serial logging
        // 프로덕션 빌드 - 시리얼 로깅 없음
        delay(500);
        manager->pAdvertising->start();
    }
};

class BLENimbleManager::CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
private:
    BLENimbleManager* manager;
    
public:
    CharacteristicCallbacks(BLENimbleManager* mgr) : manager(mgr) {}
    
    void onWrite(NimBLECharacteristic* pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (value.length() > 0) {
            // Handle potential packet fragmentation
            // 패킷 분할 가능성 처리
            
            // Check for JSON start/end markers
            // JSON 시작/끝 마커 확인
            if (value.front() == '{' && value.back() == '}') {
                // Complete JSON packet
                // 완전한 JSON 패킷
                manager->receivedData = value;
                manager->fragmentBuffer.clear();
                manager->fragmentStartTime = 0;
            } else if (value.front() == '{') {
                // Start of fragmented JSON
                // 분할된 JSON의 시작
                if (value.length() < MAX_FRAGMENT_SIZE) {
                    manager->fragmentBuffer = value;
                    manager->fragmentStartTime = millis();
                }
            } else if (value.back() == '}' && !manager->fragmentBuffer.empty()) {
                // End of fragmented JSON
                // 분할된 JSON의 끝
                if (manager->fragmentBuffer.length() + value.length() < MAX_FRAGMENT_SIZE) {
                    manager->fragmentBuffer += value;
                    manager->receivedData = manager->fragmentBuffer;
                    manager->fragmentBuffer.clear();
                    manager->fragmentStartTime = 0;
                }
            } else if (!manager->fragmentBuffer.empty()) {
                // Middle fragment
                // 중간 분할
                if (manager->fragmentBuffer.length() + value.length() < MAX_FRAGMENT_SIZE) {
                    manager->fragmentBuffer += value;
                }
            } else {
                // Non-JSON data (protocol commands)
                // 비JSON 데이터 (프로토콜 명령)
                manager->receivedData = value;
            }
        }
    }
};

BLENimbleManager::BLENimbleManager() : 
    pServer(nullptr), 
    pCharacteristicRX(nullptr), 
    pCharacteristicTX(nullptr),
    pAdvertising(nullptr),
    deviceConnected(false),
    fragmentStartTime(0) {
}

BLENimbleManager::~BLENimbleManager() {
    stop();
}

bool BLENimbleManager::begin() {
    try {
        NimBLEDevice::init(BLE_DEVICE_NAME);
        NimBLEDevice::setPower(ESP_PWR_LVL_P9);
        
        // Set MTU to 512 bytes for large JSON payloads
        // 큰 JSON 페이로드를 위해 MTU를 512바이트로 설정
        NimBLEDevice::setMTU(512);
        
        pServer = NimBLEDevice::createServer();
        pServer->setCallbacks(new ServerCallbacks(this));
        
        NimBLEService* pService = pServer->createService(BLE_SERVICE_UUID);
        
        pCharacteristicRX = pService->createCharacteristic(
            BLE_CHAR_UUID_RX,
            NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
        );
        pCharacteristicRX->setCallbacks(new CharacteristicCallbacks(this));
        
        pCharacteristicTX = pService->createCharacteristic(
            BLE_CHAR_UUID_TX,
            NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
        );
        
        pService->start();
        
        pAdvertising = NimBLEDevice::getAdvertising();
        pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
        pAdvertising->setScanResponse(true);
        pAdvertising->setMinPreferred(0x06);
        pAdvertising->setMaxPreferred(0x12);
        pAdvertising->start();
        
        // Production build - no serial logging
        // 프로덕션 빌드 - 시리얼 로깅 없음
        return true;
        
    } catch (...) {
        // Production build - no serial logging
        // 프로덕션 빌드 - 시리얼 로깅 없음
        return false;
    }
}

void BLENimbleManager::stop() {
    if (pAdvertising) {
        pAdvertising->stop();
    }
    if (pServer) {
        pServer->getAdvertising()->stop();
    }
    NimBLEDevice::deinit(true);
}

bool BLENimbleManager::hasReceivedData() {
    return !receivedData.empty();
}

std::string BLENimbleManager::getReceivedData() {
    std::string data = receivedData;
    receivedData.clear();
    return data;
}

void BLENimbleManager::sendNotification(const char* data) {
    if (deviceConnected && pCharacteristicTX) {
        pCharacteristicTX->setValue(data);
        pCharacteristicTX->notify();
    }
}

void BLENimbleManager::printStatus() {
    // Production build - no serial logging
    // 프로덕션 빌드 - 시리얼 로깅 없음
}

bool BLENimbleManager::isConnected() {
    return deviceConnected;
}

void BLENimbleManager::checkFragmentTimeout() {
    // Check if fragment buffer has timed out
    // 분할 버퍼가 타임아웃되었는지 확인
    if (!fragmentBuffer.empty() && fragmentStartTime > 0) {
        if (millis() - fragmentStartTime > FRAGMENT_TIMEOUT) {
            // Clear expired fragment
            // 만료된 분할 제거
            fragmentBuffer.clear();
            fragmentStartTime = 0;
        }
    }
}