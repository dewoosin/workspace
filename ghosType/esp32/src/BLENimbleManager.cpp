// src/BLENimbleManager.cpp
// GHOSTYPE 상품화 버전 - NimBLE 기반 BLE 관리자 구현 (보안 완전 제거)

#include "BLEConfig.h"  // 반드시 첫 번째로 포함
#include "BLENimbleManager.h"
#include <Preferences.h>
#include <esp_system.h>
#include <esp_gap_ble_api.h>

// 전역 Preferences 객체 (NVS 접근용)
static Preferences preferences;

// ===== 서버 콜백 구현 (보안 제거) =====
void ServerCallbacks::onConnect(NimBLEServer* pServer) {
    Serial.println("🔌 BLE 연결 시도 감지");
}

void ServerCallbacks::onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) {
    if (!desc) {
        Serial.println("❌ 연결 정보 없음");
        return;
    }
    
    char addrStr[18];
    NimBLEAddress addr(desc->peer_ota_addr);
    strcpy(addrStr, addr.toString().c_str());
    
    Serial.printf("🔗 연결: %s (핸들: %d)\n", addrStr, desc->conn_handle);
    
    // 최소한의 지연만
    delay(100);
    
    manager->handleConnect(desc->conn_handle, std::string(addrStr));
}

void ServerCallbacks::onDisconnect(NimBLEServer* pServer) {
    Serial.println("🔌 BLE 연결 해제 감지");
}

void ServerCallbacks::onDisconnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) {
    if (!desc) return;
    Serial.printf("📱 연결 해제: 핸들 %d\n", desc->conn_handle);
    manager->handleDisconnect(desc->conn_handle);
}

void ServerCallbacks::onMTUChange(uint16_t MTU, ble_gap_conn_desc* desc) {
    Serial.printf("📏 MTU: %d bytes (연결: %d)\n", MTU, desc->conn_handle);
    auto it = manager->connectedDevices.find(desc->conn_handle);
    if (it != manager->connectedDevices.end()) {
        it->second.mtu = MTU;
    }
}

// 보안 관련 콜백 완전 제거/무시
uint32_t ServerCallbacks::onPassKeyRequest() {
    Serial.println("🔑 패스키 요청 무시 (보안 비활성화)");
    return 0;
}

void ServerCallbacks::onAuthenticationComplete(ble_gap_conn_desc* desc) {
    if (!desc) return;
    Serial.printf("🔐 인증 생략 (연결: %d)\n", desc->conn_handle);
    // 무조건 성공으로 처리
    manager->handleAuthComplete(desc->conn_handle, true);
}

bool ServerCallbacks::onConfirmPIN(uint32_t pin) {
    Serial.println("🔢 PIN 확인 무시 (보안 비활성화)");
    return false; // 거부하여 PIN 프로세스 차단
}

// ===== 특성 콜백 구현 =====
void CharacteristicCallbacks::onWrite(NimBLECharacteristic* pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0) {
        if (charType == "RX") {
            Serial.printf("📝 RX 데이터 수신: %d bytes\n", value.length());
            
            for (const auto& pair : manager->connectedDevices) {
                if (pair.second.isSubscribed) {
                    manager->handleDataReceived(pair.first, value);
                    break;
                }
            }
        }
    }
}

void CharacteristicCallbacks::onRead(NimBLECharacteristic* pCharacteristic) {
    std::string uuid = pCharacteristic->getUUID().toString();
    #if DEBUG_VERBOSE
    Serial.printf("📖 특성 읽기: %s\n", uuid.c_str());
    #endif
}

void CharacteristicCallbacks::onNotify(NimBLECharacteristic* pCharacteristic) {
    #if DEBUG_VERBOSE
    Serial.println("📢 Notify 전송 완료");
    #endif
}

void CharacteristicCallbacks::onStatus(NimBLECharacteristic* pCharacteristic, Status status, int code) {
    std::string uuid = pCharacteristic->getUUID().toString();
    
    switch (status) {
        case Status::SUCCESS_NOTIFY:
            #if DEBUG_VERBOSE
            Serial.printf("✅ Notify 성공: %s\n", uuid.c_str());
            #endif
            break;
        case Status::ERROR_GATT:
            Serial.printf("❌ GATT 에러: %s (코드: %d)\n", uuid.c_str(), code);
            manager->totalErrors++;
            break;
        default:
            break;
    }
}

void CharacteristicCallbacks::onSubscribe(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc, uint16_t subValue) {
    String uuid = pCharacteristic->getUUID().toString().c_str();
    
    if (subValue == 0) {
        Serial.printf("📵 Notify 구독 해제: %s\n", uuid.c_str());
        manager->handleSubscriptionChange(desc->conn_handle, false);
    } else if (subValue == 1) {
        Serial.printf("📬 Notify 구독: %s\n", uuid.c_str());
        manager->handleSubscriptionChange(desc->conn_handle, true);
        
        // ===== 자동 메시지 전송 완전 제거 =====
        // 연결 시 자동으로 키보드 입력이 되는 것을 방지
        if (charType == "TX") {
            Serial.println("🔗 TX 특성 구독됨 - 자동 메시지 전송 안함 (키보드 입력 방지)");
            // 자동 메시지 전송하지 않음
            // pCharacteristic->setValue("GHOSTYPE Connected!");  // 주석 처리
            // pCharacteristic->notify();                          // 주석 처리
        }
        
    } else if (subValue == 2) {
        Serial.printf("📮 Indicate 구독: %s\n", uuid.c_str());
    }
}

// ===== BLENimbleManager 생성자 =====
BLENimbleManager::BLENimbleManager()
    : pServer(nullptr)
    , pAdvertising(nullptr)
    , pUartService(nullptr)
    , pTxCharacteristic(nullptr)
    , pRxCharacteristic(nullptr)
    , pDeviceInfoService(nullptr)
    , pBatteryService(nullptr)
    , serverCallbacks(nullptr)
    , rxCallbacks(nullptr)
    , txCallbacks(nullptr)
    , currentState(BLEConnectionState::IDLE)
    , isInitialized(false)
    , isAdvertising(false)
    , numConnections(0)
    , totalMessages(0)
    , totalBytes(0)
    , totalErrors(0)
    , startTime(0)
    , lastAdvertiseTime(0)
    , fixedPasskey(0)
    , isSecurityEnabled(false) {
    
    // 수신 큐 생성
    rxQueue = xQueueCreate(20, sizeof(std::string*));
    rxMutex = xSemaphoreCreateMutex();
    
    // MAC 주소 읽기
    esp_read_mac(deviceMAC, ESP_MAC_BT);
}

// ===== BLENimbleManager 소멸자 =====
BLENimbleManager::~BLENimbleManager() {
    stop();
    
    if (rxQueue) vQueueDelete(rxQueue);
    if (rxMutex) vSemaphoreDelete(rxMutex);
    
    delete serverCallbacks;
    delete rxCallbacks;
    delete txCallbacks;
}

// ===== BLE 초기화 (보안 완전 제거) =====
bool BLENimbleManager::begin() {
    if (isInitialized) {
        Serial.println("⚠️ BLE 이미 초기화됨");
        return true;
    }
    
    Serial.println("\n╔════════════════════════════════════════╗");
    Serial.println("║     BLE 시스템 초기화 시작             ║");
    Serial.println("╚════════════════════════════════════════╝");
    
    startTime = millis();
    currentState = BLEConnectionState::IDLE;
    
    // 1. 디바이스 이름 생성
    generateDeviceName();
    Serial.printf("📱 디바이스명: %s\n", deviceNameWithMac.c_str());
    
    // 2. NimBLE 초기화
    NimBLEDevice::init(deviceNameWithMac);
    
    // 3. 전력 설정
    NimBLEDevice::setPower(BLE_TX_POWER);
    Serial.printf("📡 송신 출력: +%ddBm\n", 9);
    
    // 4. MTU 설정
    NimBLEDevice::setMTU(BLE_MTU_SIZE);
    Serial.printf("📏 기본 MTU: %d bytes\n", BLE_MTU_SIZE);
    
    // 5. 보안 완전 비활성화
    Serial.println("🔓 보안 완전 비활성화 - 연결 안정성 최우선");
    NimBLEDevice::setSecurityAuth(false, false, false);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);
    NimBLEDevice::setSecurityPasskey(0);
    isSecurityEnabled = false;
    
    // 6. 서버 생성
    pServer = NimBLEDevice::createServer();
    if (!pServer) {
        Serial.println("❌ BLE 서버 생성 실패!");
        currentState = BLEConnectionState::ERROR;
        return false;
    }
    
    // 7. 서버 콜백 설정
    serverCallbacks = new ServerCallbacks(this);
    pServer->setCallbacks(serverCallbacks);
    
    // 8. 서비스 생성
    Serial.println("\n📋 서비스 생성 중...");
    createServices();
    
    // 9. 페어링 정보 로드 생략 (보안 비활성화)
    Serial.println("📋 페어링 정보 생략 (보안 비활성화)");
    
    // 10. 광고 설정 및 시작
    configureAdvertising();
    startAdvertising();
    
    isInitialized = true;
    currentState = BLEConnectionState::ADVERTISING;
    
    Serial.println("\n✅ BLE 초기화 완료!");
    Serial.println("┌────────────────────────────────────────┐");
    Serial.printf("│ 이름: %-32s │\n", deviceNameWithMac.c_str());
    Serial.printf("│ MAC: %s       │\n", getMACAddress().c_str());
    Serial.printf("│ 최대 연결: %d개                         │\n", MAX_CONNECTED_DEVICES);
    Serial.println("└────────────────────────────────────────┘");
    
    return true;
}

// ===== 디바이스 이름 생성 =====
void BLENimbleManager::generateDeviceName() {
    char macStr[5];
    snprintf(macStr, sizeof(macStr), "%02X%02X", deviceMAC[4], deviceMAC[5]);
    deviceNameWithMac = std::string(DEVICE_NAME) + std::string(macStr);
    
    Serial.printf("🏷️ 생성된 디바이스 이름: %s\n", deviceNameWithMac.c_str());
    NimBLEDevice::setDeviceName(deviceNameWithMac);
}

// ===== MAC 주소 문자열 반환 =====
std::string BLENimbleManager::getMACAddress() const {
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             deviceMAC[0], deviceMAC[1], deviceMAC[2],
             deviceMAC[3], deviceMAC[4], deviceMAC[5]);
    return std::string(macStr);
}

// ===== 서비스 생성 =====
void BLENimbleManager::createServices() {
    createUartService();
    createDeviceInfoService();
    createBatteryService();
    Serial.println("✅ 모든 서비스 생성 완료");
}

// ===== UART 서비스 생성 =====
void BLENimbleManager::createUartService() {
    Serial.println("  📡 Nordic UART Service 생성 중...");
    
    pUartService = pServer->createService(SERVICE_UUID);
    
    pRxCharacteristic = pUartService->createCharacteristic(
        CHARACTERISTIC_UUID_RX,
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
    );
    rxCallbacks = new CharacteristicCallbacks(this, "RX");
    pRxCharacteristic->setCallbacks(rxCallbacks);
    
    pTxCharacteristic = pUartService->createCharacteristic(
        CHARACTERISTIC_UUID_TX,
        NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ
    );
    txCallbacks = new CharacteristicCallbacks(this, "TX");
    pTxCharacteristic->setCallbacks(txCallbacks);
    
    pTxCharacteristic->setValue("GHOSTYPE");
    pUartService->start();
    Serial.println("  ✅ UART Service 생성 완료");
}

// ===== Device Information Service 생성 =====
void BLENimbleManager::createDeviceInfoService() {
    Serial.println("  📱 Device Information Service 생성 중...");
    
    pDeviceInfoService = pServer->createService(DIS_SERVICE_UUID);
    
    pManufacturerChar = pDeviceInfoService->createCharacteristic(
        MANUFACTURER_CHAR_UUID, NIMBLE_PROPERTY::READ);
    pManufacturerChar->setValue(MANUFACTURER_NAME);
    
    pModelChar = pDeviceInfoService->createCharacteristic(
        MODEL_CHAR_UUID, NIMBLE_PROPERTY::READ);
    pModelChar->setValue(HARDWARE_VERSION);
    
    pSerialChar = pDeviceInfoService->createCharacteristic(
        SERIAL_CHAR_UUID, NIMBLE_PROPERTY::READ);
    char serial[13];
    snprintf(serial, sizeof(serial), "%02X%02X%02X%02X%02X%02X",
             deviceMAC[0], deviceMAC[1], deviceMAC[2],
             deviceMAC[3], deviceMAC[4], deviceMAC[5]);
    pSerialChar->setValue(serial);
    
    pFirmwareChar = pDeviceInfoService->createCharacteristic(
        FIRMWARE_CHAR_UUID, NIMBLE_PROPERTY::READ);
    pFirmwareChar->setValue(PRODUCT_VERSION);
    
    pHardwareChar = pDeviceInfoService->createCharacteristic(
        HARDWARE_CHAR_UUID, NIMBLE_PROPERTY::READ);
    pHardwareChar->setValue("1.0");
    
    pSoftwareChar = pDeviceInfoService->createCharacteristic(
        SOFTWARE_CHAR_UUID, NIMBLE_PROPERTY::READ);
    pSoftwareChar->setValue(ESP.getSdkVersion());
    
    pDeviceInfoService->start();
    Serial.println("  ✅ Device Info Service 생성 완료");
}

// ===== Battery Service 생성 =====
void BLENimbleManager::createBatteryService() {
    Serial.println("  🔋 Battery Service 생성 중...");
    
    pBatteryService = pServer->createService(BATTERY_SERVICE_UUID);
    
    pBatteryLevelChar = pBatteryService->createCharacteristic(
        BATTERY_LEVEL_CHAR_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );
    
    uint8_t batteryLevel = 100;
    pBatteryLevelChar->setValue(&batteryLevel, 1);
    
    pBatteryService->start();
    Serial.println("  ✅ Battery Service 생성 완료");
}

// ===== 광고 설정 (최소 버전) =====
void BLENimbleManager::configureAdvertising() {
    Serial.println("\n🔊 BLE 광고 설정 중... (최소 버전)");
    
    pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->reset();
    
    // 가장 기본적인 설정만
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setName(deviceNameWithMac);
    
    // 매우 보수적인 광고 간격
    pAdvertising->setMinInterval(0x40);   // 40ms
    pAdvertising->setMaxInterval(0x100);  // 160ms
    
    Serial.println("✅ 최소 광고 설정 완료");
    Serial.printf("   이름: %s\n", deviceNameWithMac.c_str());
    Serial.printf("   UUID: %s\n", SERVICE_UUID);
}

// ===== 광고 시작 (안전 버전) =====
void BLENimbleManager::startAdvertising() {
    if (isAdvertising) {
        Serial.println("⚠️ 이미 광고 중입니다");
        return;
    }
    
    Serial.println("🔊 BLE 광고 시작... (안전 모드)");
    
    pAdvertising->start();
    
    isAdvertising = true;
    lastAdvertiseTime = millis();
    currentState = BLEConnectionState::ADVERTISING;
    
    Serial.println("✅ BLE 광고 시작됨");
    Serial.printf("   → %s\n", deviceNameWithMac.c_str());
}

// ===== 연결 처리 (보안 제거) =====
void BLENimbleManager::handleConnect(uint16_t connHandle, const std::string& address) {
    Serial.println("\n╔════════════════════════════════════════╗");
    Serial.println("║         새 디바이스 연결 중...         ║");
    Serial.println("╚════════════════════════════════════════╝");
    
    numConnections = pServer->getConnectedCount();
    
    if (numConnections > MAX_CONNECTED_DEVICES) {
        Serial.printf("⚠️ 최대 연결 수 초과! (현재: %d, 최대: %d)\n", 
                      numConnections, MAX_CONNECTED_DEVICES);
        pServer->disconnect(connHandle);
        return;
    }
    
    if (connectedDevices.find(connHandle) != connectedDevices.end()) {
        Serial.println("⚠️ 이미 연결된 핸들!");
        return;
    }
    
    // 연결 정보 생성
    ConnectedDevice device;
    device.connHandle = connHandle;
    device.address = address;
    device.name = "Unknown";
    device.connectedTime = millis();
    device.lastActivityTime = millis();
    device.isSubscribed = false;
    device.isAuthenticated = true;  // 항상 인증됨으로 처리
    device.mtu = BLE_MTU_SIZE;
    device.rssi = 0;
    
    connectedDevices[connHandle] = device;
    currentState = BLEConnectionState::CONNECTED;
    
    Serial.println("✅ 디바이스 연결 성공!");
    Serial.printf("📱 주소: %s\n", address.c_str());
    Serial.printf("🔗 핸들: %d\n", connHandle);
    Serial.printf("👥 연결 수: %d/%d\n", numConnections, MAX_CONNECTED_DEVICES);
    
    // 연결 파라미터 업데이트 생략 (안정성 최우선)
    Serial.println("⚙️ 연결 파라미터 업데이트 생략 (안정성 우선)");
    
    // 페어링 정보 저장 생략
    Serial.println("📋 페어링 정보 저장 생략 (보안 비활성화)");
    
    if (numConnections >= MAX_CONNECTED_DEVICES) {
        pauseAdvertising();
        Serial.println("📵 최대 연결 도달 - 광고 중지");
    }
}

// ===== 연결 해제 처리 =====
void BLENimbleManager::handleDisconnect(uint16_t connHandle) {
    auto it = connectedDevices.find(connHandle);
    if (it != connectedDevices.end()) {
        uint32_t connDuration = (millis() - it->second.connectedTime) / 1000;
        
        Serial.println("\n❌ 디바이스 연결 해제");
        Serial.printf("📱 주소: %s\n", it->second.address.c_str());
        Serial.printf("⏱️ 연결 시간: %d초\n", connDuration);
        
        connectedDevices.erase(it);
    }
    
    numConnections = pServer->getConnectedCount();
    Serial.printf("👥 남은 연결: %d/%d\n", numConnections, MAX_CONNECTED_DEVICES);
    
    if (numConnections == 0) {
        currentState = BLEConnectionState::IDLE;
    }
    
    if (numConnections < MAX_CONNECTED_DEVICES && !isAdvertising) {
        resumeAdvertising();
    }
}

// ===== 구독 상태 변경 처리 =====
void BLENimbleManager::handleSubscriptionChange(uint16_t connHandle, bool subscribed) {
    auto it = connectedDevices.find(connHandle);
    if (it != connectedDevices.end()) {
        it->second.isSubscribed = subscribed;
        Serial.printf("%s 알림 구독 %s (연결: %d)\n",
                      subscribed ? "✅" : "❌",
                      subscribed ? "활성화" : "해제",
                      connHandle);
    }
}

// ===== 인증 완료 처리 (항상 성공) =====
void BLENimbleManager::handleAuthComplete(uint16_t connHandle, bool success) {
    auto it = connectedDevices.find(connHandle);
    if (it != connectedDevices.end()) {
        it->second.isAuthenticated = true; // 항상 성공으로 처리
        Serial.printf("🔐 인증 생략 - 무조건 성공: %s\n", it->second.address.c_str());
    }
}

// ===== 데이터 수신 처리 =====
void BLENimbleManager::handleDataReceived(uint16_t connHandle, const std::string& data) {
    totalMessages++;
    totalBytes += data.length();
    
    auto it = connectedDevices.find(connHandle);
    if (it != connectedDevices.end()) {
        it->second.lastActivityTime = millis();
    }
    
    Serial.println("\n📨 데이터 수신:");
    Serial.printf("   내용: \"%s\"\n", data.c_str());
    Serial.printf("   크기: %d bytes\n", data.length());
    Serial.printf("   연결: %d\n", connHandle);
    Serial.printf("   총계: %d 메시지, %d bytes\n", totalMessages, totalBytes);
    
    if (xSemaphoreTake(rxMutex, portMAX_DELAY) == pdTRUE) {
        std::string* pData = new std::string(data);
        if (xQueueSend(rxQueue, &pData, 0) != pdTRUE) {
            delete pData;
            Serial.println("⚠️ 수신 큐 가득참!");
        }
        xSemaphoreGive(rxMutex);
    }
}

// ===== 데이터 전송 =====
bool BLENimbleManager::sendData(const std::string& data, uint16_t connHandle) {
    if (!pTxCharacteristic || numConnections == 0) {
        Serial.println("⚠️ 전송 불가: 연결 없음");
        return false;
    }
    
    if (connHandle != 0xFFFF) {
        auto it = connectedDevices.find(connHandle);
        if (it == connectedDevices.end() || !it->second.isSubscribed) {
            Serial.printf("⚠️ 전송 불가: 연결 %d 없음/미구독\n", connHandle);
            return false;
        }
    }
    
    size_t maxDataSize = BLE_MTU_SIZE - 3;
    if (data.length() > maxDataSize) {
        Serial.printf("⚠️ 데이터 크기 초과: %d > %d\n", data.length(), maxDataSize);
    }
    
    pTxCharacteristic->setValue(data);
    pTxCharacteristic->notify();
    
    #if DEBUG_VERBOSE
    Serial.printf("📤 전송 성공: \"%s\" (%d bytes)\n", 
                  data.c_str(), data.length());
    #endif
    
    return true;
}

// ===== 모든 연결로 데이터 전송 =====
bool BLENimbleManager::sendDataToAll(const std::string& data) {
    if (!pTxCharacteristic || numConnections == 0) {
        return false;
    }
    
    int subscribedCount = 0;
    for (const auto& pair : connectedDevices) {
        if (pair.second.isSubscribed) {
            subscribedCount++;
        }
    }
    
    if (subscribedCount == 0) {
        Serial.println("⚠️ 구독한 디바이스 없음");
        return false;
    }
    
    pTxCharacteristic->setValue(data);
    pTxCharacteristic->notify();
    
    Serial.printf("📢 전체 전송: \"%s\" → %d 디바이스\n", 
                  data.c_str(), subscribedCount);
    
    return true;
}

// ===== 수신 데이터 확인 =====
bool BLENimbleManager::hasReceivedData() {
    return uxQueueMessagesWaiting(rxQueue) > 0;
}

// ===== 수신 데이터 가져오기 =====
std::string BLENimbleManager::getReceivedData() {
    std::string result;
    std::string* pData = nullptr;
    
    if (xSemaphoreTake(rxMutex, portMAX_DELAY) == pdTRUE) {
        if (xQueueReceive(rxQueue, &pData, 0) == pdTRUE && pData) {
            result = *pData;
            delete pData;
        }
        xSemaphoreGive(rxMutex);
    }
    
    return result;
}

// ===== 페어링 관련 (생략) =====
void BLENimbleManager::loadPairedDevices() {
    // 보안 비활성화로 생략
}

void BLENimbleManager::savePairedDevices() {
    // 보안 비활성화로 생략  
}

void BLENimbleManager::addPairedDevice(const std::string& address, const std::string& name) {
    // 보안 비활성화로 생략
}

bool BLENimbleManager::isDevicePaired(const std::string& address) {
    return false; // 항상 false
}

// ===== 광고 제어 =====
void BLENimbleManager::pauseAdvertising() {
    if (isAdvertising && pAdvertising) {
        pAdvertising->stop();
        isAdvertising = false;
        Serial.println("⏸️ BLE 광고 일시정지");
    }
}

void BLENimbleManager::resumeAdvertising() {
    if (!isAdvertising && pAdvertising) {
        pAdvertising->start(0);
        isAdvertising = true;
        currentState = BLEConnectionState::ADVERTISING;
        Serial.println("▶️ BLE 광고 재개");
    }
}

// ===== 배터리 관리 =====
void BLENimbleManager::updateBatteryLevel(uint8_t level) {
    if (pBatteryLevelChar && level <= 100) {
        pBatteryLevelChar->setValue(&level, 1);
        pBatteryLevelChar->notify();
    }
}

uint8_t BLENimbleManager::getBatteryLevel() {
    if (pBatteryLevelChar) {
        std::string value = pBatteryLevelChar->getValue();
        if (value.length() > 0) {
            return static_cast<uint8_t>(value[0]);
        }
    }
    return 100;
}

// ===== 상태 출력 =====
void BLENimbleManager::printStatus() {
    uint32_t uptime = getUptimeSeconds();
    
    Serial.println("\n╔════════════════════════════════════════╗");
    Serial.println("║          GHOSTYPE 상태 정보            ║");
    Serial.println("╚════════════════════════════════════════╝");
    Serial.printf("⏱️ 가동 시간: %02d:%02d:%02d\n", 
                  uptime / 3600, (uptime % 3600) / 60, uptime % 60);
    Serial.printf("📡 BLE 상태: %s\n", getStateString().c_str());
    Serial.printf("🔗 연결 수: %d/%d\n", numConnections, MAX_CONNECTED_DEVICES);
    
    if (numConnections > 0) {
        Serial.println("\n📱 연결된 디바이스:");
        for (const auto& pair : connectedDevices) {
            const auto& device = pair.second;
            uint32_t connTime = (millis() - device.connectedTime) / 1000;
            Serial.printf("  [%d] %s\n", pair.first, device.address.c_str());
            Serial.printf("      연결: %d초, 구독: %s, 인증: %s\n",
                          connTime,
                          device.isSubscribed ? "✓" : "✗",
                          device.isAuthenticated ? "✓" : "✗");
        }
    }
    
    Serial.printf("\n📊 통계:\n");
    Serial.printf("   메시지: %d개\n", totalMessages);
    Serial.printf("   데이터: %d bytes\n", totalBytes);
    Serial.printf("   에러: %d회\n", totalErrors);
    Serial.printf("   처리율: %.1f msg/min\n", getMessagesPerMinute());
    
    Serial.printf("\n💾 시스템:\n");
    Serial.printf("   메모리: %d KB / %d KB\n", 
                  ESP.getFreeHeap() / 1024, ESP.getHeapSize() / 1024);
}

// ===== 상태 문자열 반환 =====
std::string BLENimbleManager::getStateString() const {
    switch (currentState) {
        case BLEConnectionState::IDLE:         return "대기";
        case BLEConnectionState::ADVERTISING:  return "광고 중";
        case BLEConnectionState::CONNECTING:   return "연결 중";
        case BLEConnectionState::CONNECTED:    return "연결됨";
        case BLEConnectionState::DISCONNECTING: return "연결 해제 중";
        case BLEConnectionState::ERROR:        return "에러";
        default:                              return "알 수 없음";
    }
}

// ===== 가동 시간 (초) =====
uint32_t BLENimbleManager::getUptimeSeconds() const {
    return (millis() - startTime) / 1000;
}

// ===== 분당 메시지 수 =====
float BLENimbleManager::getMessagesPerMinute() const {
    uint32_t uptime = getUptimeSeconds();
    if (uptime < 60) return 0;
    return (float)totalMessages * 60.0f / (float)uptime;
}

// ===== BLE 종료 =====
void BLENimbleManager::stop() {
    if (!isInitialized) return;
    
    Serial.println("\n🛑 BLE 시스템 종료 중...");
    
    currentState = BLEConnectionState::DISCONNECTING;
    
    disconnectAll();
    
    if (pAdvertising) {
        pAdvertising->stop();
    }
    
    NimBLEDevice::deinit();
    
    isInitialized = false;
    isAdvertising = false;
    currentState = BLEConnectionState::IDLE;
    
    Serial.println("✅ BLE 시스템 종료 완료");
}

// ===== 모든 연결 해제 =====
void BLENimbleManager::disconnectAll() {
    if (pServer && numConnections > 0) {
        std::vector<uint16_t> handles;
        for (const auto& pair : connectedDevices) {
            handles.push_back(pair.first);
        }
        
        for (uint16_t handle : handles) {
            Serial.printf("🔌 연결 해제 중: %d\n", handle);
            pServer->disconnect(handle);
            delay(100);
        }
        
        connectedDevices.clear();
        numConnections = 0;
    }
}

// ===== 페어링 정보 삭제 =====
void BLENimbleManager::clearPairedDevices() {
    Serial.println("🗑️ 페어링 기능 비활성화됨");
}

// ===== 시스템 리셋 =====
void BLENimbleManager::reset() {
    Serial.println("🔄 BLE 시스템 재시작 중...");
    stop();
    delay(1000);
    begin();
}

// ===== 나머지 유틸리티 함수들 =====
void BLENimbleManager::enableDebugMode(bool enable) {
    Serial.printf("🐛 디버그 모드: %s\n", enable ? "활성화" : "비활성화");
}

bool BLENimbleManager::sendDataToDevice(const std::string& data, const std::string& address) {
    for (const auto& pair : connectedDevices) {
        if (pair.second.address == address) {
            return sendData(data, pair.first);
        }
    }
    Serial.printf("⚠️ 디바이스 %s 연결되지 않음\n", address.c_str());
    return false;
}

bool BLENimbleManager::isDeviceConnected(const std::string& address) const {
    for (const auto& pair : connectedDevices) {
        if (pair.second.address == address) {
            return true;
        }
    }
    return false;
}

bool BLENimbleManager::isDeviceConnected(uint16_t connHandle) const {
    return connectedDevices.find(connHandle) != connectedDevices.end();
}

std::vector<ConnectedDevice> BLENimbleManager::getConnectedDevices() const {
    std::vector<ConnectedDevice> devices;
    for (const auto& pair : connectedDevices) {
        devices.push_back(pair.second);
    }
    return devices;
}

void BLENimbleManager::disconnectDevice(const std::string& address) {
    for (const auto& pair : connectedDevices) {
        if (pair.second.address == address) {
            disconnectDevice(pair.first);
            return;
        }
    }
}

void BLENimbleManager::disconnectDevice(uint16_t connHandle) {
    if (pServer && connectedDevices.find(connHandle) != connectedDevices.end()) {
        pServer->disconnect(connHandle);
    }
}

void BLENimbleManager::clearReceivedData() {
    std::string* pData = nullptr;
    if (xSemaphoreTake(rxMutex, portMAX_DELAY) == pdTRUE) {
        while (xQueueReceive(rxQueue, &pData, 0) == pdTRUE) {
            if (pData) {
                delete pData;
            }
        }
        xSemaphoreGive(rxMutex);
    }
}

void BLENimbleManager::updateAdvertisingData() {
    if (pAdvertising) {
        configureAdvertising();
    }
}

int8_t BLENimbleManager::getDeviceRSSI(uint16_t connHandle) {
    auto it = connectedDevices.find(connHandle);
    if (it != connectedDevices.end()) {
        return it->second.rssi;
    }
    return 0;
}

void BLENimbleManager::setSecurityAuth(bool bonding, bool mitm, bool sc) {
    // 보안 비활성화로 무시
}

void BLENimbleManager::setPasskey(uint32_t passkey) {
    // 보안 비활성화로 무시
}

void BLENimbleManager::enableSecurity(bool enable) {
    // 보안 비활성화로 무시
}

void BLENimbleManager::removePairedDevice(const std::string& address) {
    // 보안 비활성화로 무시
}

void BLENimbleManager::printDetailedStatus() {
    printStatus();
    Serial.println("📋 페어링 기능: 비활성화됨 (보안 제거)");
}