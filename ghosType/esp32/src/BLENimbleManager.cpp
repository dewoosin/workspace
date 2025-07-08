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
        // BLE 초기화 전 기존 연결 정리
        // Clean up existing connections before BLE initialization
        if (NimBLEDevice::getInitialized()) {
            NimBLEDevice::deinit(true);
        }
        delay(1000);
        
        // BLE 장치 초기화
        // Initialize BLE device
        NimBLEDevice::init(BLE_DEVICE_NAME);
        
        // 전력 설정 (연결 범위 향상)
        // Set power level (improve connection range)
        NimBLEDevice::setPower(ESP_PWR_LVL_P9);
        
        // MTU 설정을 더 보수적으로 (연결 성공률 향상)
        // Set MTU more conservatively (improve connection success rate)
        NimBLEDevice::setMTU(247);  // 512 -> 247로 변경
        
        // 보안 설정 (페어링 없이 연결 허용)
        // Security settings (allow connection without pairing)
        NimBLEDevice::setSecurityAuth(false, false, true);
        NimBLEDevice::setSecurityPasskey(123456);
        NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);
        
        // 서버 생성 및 콜백 설정
        // Create server and set callbacks
        pServer = NimBLEDevice::createServer();
        pServer->setCallbacks(new ServerCallbacks(this));
        
        // 서비스 생성
        // Create service
        NimBLEService* pService = pServer->createService(BLE_SERVICE_UUID);
        
        // 수신 특성 설정 (WRITE 권한)
        // Set RX characteristic (WRITE permission)
        pCharacteristicRX = pService->createCharacteristic(
            BLE_CHAR_UUID_RX,
            NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
        );
        pCharacteristicRX->setCallbacks(new CharacteristicCallbacks(this));
        
        // 송신 특성 설정 (NOTIFY 권한)
        // Set TX characteristic (NOTIFY permission)
        pCharacteristicTX = pService->createCharacteristic(
            BLE_CHAR_UUID_TX,
            NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
        );
        
        // 서비스 시작
        // Start service
        pService->start();
        
        // 광고 설정 (기본값으로 복원)
        // Configure advertising (restore to defaults)
        pAdvertising = NimBLEDevice::getAdvertising();
        pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
        pAdvertising->setName(BLE_DEVICE_NAME);
        pAdvertising->setScanResponse(true);
        
        // 기본 광고 간격 사용 (안정성 우선)
        // Use default advertising intervals (stability first)
        // setMinInterval/setMaxInterval 제거 - 기본값 사용
        
        // 광고 시작
        // Start advertising
        pAdvertising->start();
        
        // 연결 상태 초기화
        // Initialize connection state
        deviceConnected = false;
        
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