/**
 * @file ble_manager.cpp
 * @brief BLE 통신 관리 모듈 구현
 * @version 1.0
 * @date 2024-12-28
 * 
 * ⚠️ 중요: 이 파일을 수정하기 전에 ../BLE_Debug_History.md를 먼저 확인하세요!
 */

#include "ble_manager.h"
#include "hid_utils.h"

// 정적 멤버 변수 초기화
NimBLEServer* BLEManager::ble_server = nullptr;
NimBLEService* BLEManager::ble_service = nullptr;
NimBLECharacteristic* BLEManager::char_rx = nullptr;
NimBLECharacteristic* BLEManager::char_tx = nullptr;
NimBLEAdvertising* BLEManager::ble_advertising = nullptr;

bool BLEManager::initialized = false;
BLEConnectionState BLEManager::connection_state = BLE_STATE_DISCONNECTED;
uint8_t BLEManager::connected_clients = 0;

char* BLEManager::receive_buffer = nullptr;
size_t BLEManager::receive_buffer_size = 0;
size_t BLEManager::received_data_length = 0;
uint32_t BLEManager::last_receive_time = 0;

uint32_t BLEManager::total_bytes_received = 0;
uint32_t BLEManager::total_bytes_sent = 0;
uint32_t BLEManager::total_connections = 0;

/**
 * @brief BLE 서버 콜백 클래스
 * 
 * 클라이언트 연결/해제 이벤트를 처리합니다.
 * 연결 상태 변화를 추적하고 통계를 업데이트합니다.
 */
class BLEManager::ServerCallbacks : public NimBLEServerCallbacks {
public:
    /**
     * @brief 클라이언트 연결 시 호출되는 콜백
     * @param server BLE 서버 인스턴스
     * 
     * 새로운 클라이언트가 연결되면 연결 카운터를 증가시키고
     * 시스템 상태를 연결됨으로 변경합니다.
     */
    void onConnect(NimBLEServer* server) override {
        BLEManager::connected_clients++;
        BLEManager::total_connections++;
        BLEManager::updateConnectionState(BLE_STATE_CONNECTED);
    }

    /**
     * @brief 클라이언트 연결 해제 시 호출되는 콜백
     * @param server BLE 서버 인스턴스
     * 
     * 클라이언트 연결이 해제되면 카운터를 감소시키고
     * 필요시 광고를 재시작합니다.
     */
    void onDisconnect(NimBLEServer* server) override {
        if (BLEManager::connected_clients > 0) {
            BLEManager::connected_clients--;
        }
        
        // 모든 클라이언트가 연결 해제된 경우 광고 상태로 전환
        if (BLEManager::connected_clients == 0) {
            BLEManager::updateConnectionState(BLE_STATE_ADVERTISING);
        }
        
        // 연결 해제 후 안정화 지연 및 광고 재시작
        HIDUtils::safeDelay(500);
        if (BLEManager::ble_advertising) {
            BLEManager::ble_advertising->start();
        }
    }
};

/**
 * @brief BLE 특성 콜백 클래스
 * 
 * 클라이언트로부터의 데이터 수신을 처리합니다.
 * 수신된 데이터를 안전하게 버퍼에 저장합니다.
 */
class BLEManager::CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
public:
    /**
     * @brief 클라이언트가 특성에 데이터를 쓸 때 호출되는 콜백
     * @param characteristic 데이터가 쓰여진 특성
     * 
     * 수신된 데이터를 내부 버퍼에 저장하고 통계를 업데이트합니다.
     */
    void onWrite(NimBLECharacteristic* characteristic) override {
        std::string value = characteristic->getValue();
        
        if (value.length() > 0) {
            // 수신된 데이터를 버퍼에 안전하게 저장
            BLEManager::storeReceivedData(
                reinterpret_cast<const uint8_t*>(value.c_str()), 
                value.length()
            );
            
            // 수신 통계 업데이트
            BLEManager::updateStatistics(value.length(), 0);
        }
    }
};

bool BLEManager::initialize() {
    // 이미 초기화된 경우 성공 반환
    if (initialized) {
        return true;
    }

    // 수신 버퍼 초기화
    if (!initializeReceiveBuffer()) {
        return false;
    }

    try {
        // 기존 BLE 스택 정리 (충돌 방지)
        if (NimBLEDevice::getInitialized()) {
            NimBLEDevice::deinit(true);
            HIDUtils::safeDelay(1000);
        }

        // NimBLE 장치 초기화 - 기본 설정으로 최대 호환성 확보
        NimBLEDevice::init(BLE_DEVICE_NAME);

        // BLE 서버 생성
        ble_server = NimBLEDevice::createServer();
        if (!ble_server) {
            cleanupReceiveBuffer();
            return false;
        }

        // 서버 콜백 설정 - 연결 상태 관리
        ble_server->setCallbacks(new ServerCallbacks());

        // GHOSTYPE 서비스 생성
        ble_service = ble_server->createService(BLE_SERVICE_UUID);
        if (!ble_service) {
            cleanupReceiveBuffer();
            return false;
        }

        // RX 특성 생성 (클라이언트 → 서버 데이터 수신용)
        char_rx = ble_service->createCharacteristic(
            BLE_CHAR_RX_UUID,
            NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
        );
        if (!char_rx) {
            cleanupReceiveBuffer();
            return false;
        }
        char_rx->setCallbacks(new CharacteristicCallbacks());

        // TX 특성 생성 (서버 → 클라이언트 응답 전송용)
        char_tx = ble_service->createCharacteristic(
            BLE_CHAR_TX_UUID,
            NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
        );
        if (!char_tx) {
            cleanupReceiveBuffer();
            return false;
        }

        // 서비스 시작
        ble_service->start();

        // 광고 설정 - 최대 호환성을 위한 기본 설정 사용
        ble_advertising = NimBLEDevice::getAdvertising();
        if (!ble_advertising) {
            cleanupReceiveBuffer();
            return false;
        }

        // 광고 데이터 설정
        ble_advertising->addServiceUUID(BLE_SERVICE_UUID);
        ble_advertising->setName(BLE_DEVICE_NAME);
        ble_advertising->setScanResponse(true);
        
        // 연결 매개변수 권장값 설정 (안정성 우선)
        ble_advertising->setMinPreferred(BLE_MIN_CONN_INTERVAL);
        ble_advertising->setMaxPreferred(BLE_MAX_CONN_INTERVAL);

        // 광고 시작
        if (!ble_advertising->start()) {
            cleanupReceiveBuffer();
            return false;
        }

        // 초기화 완료
        initialized = true;
        updateConnectionState(BLE_STATE_ADVERTISING);
        
        return true;

    } catch (...) {
        // 예외 발생 시 정리 작업 수행
        cleanupReceiveBuffer();
        initialized = false;
        return false;
    }
}

void BLEManager::deinitialize() {
    if (!initialized) {
        return;
    }

    // 모든 클라이언트 연결 안전하게 해제
    disconnectAllClients();

    // 광고 중단
    if (ble_advertising) {
        ble_advertising->stop();
    }

    // BLE 스택 완전 정리
    NimBLEDevice::deinit(true);

    // 수신 버퍼 메모리 해제
    cleanupReceiveBuffer();

    // 모든 포인터 초기화
    ble_server = nullptr;
    ble_service = nullptr;
    char_rx = nullptr;
    char_tx = nullptr;
    ble_advertising = nullptr;
    
    // 상태 초기화
    initialized = false;
    connection_state = BLE_STATE_DISCONNECTED;
    connected_clients = 0;
}

bool BLEManager::hasReceivedData() {
    return (received_data_length > 0 && receive_buffer != nullptr);
}

BLEReceivedData BLEManager::getReceivedData() {
    // 기본 반환값 (실패 시)
    BLEReceivedData result = {
        .data = nullptr,
        .length = 0,
        .timestamp = 0,
        .valid = false
    };

    // 수신된 데이터가 없는 경우
    if (!hasReceivedData()) {
        return result;
    }

    // 수신된 데이터를 새로운 메모리에 복사 (호출자가 해제 책임)
    result.data = new char[received_data_length + 1];
    if (result.data) {
        memcpy(result.data, receive_buffer, received_data_length);
        result.data[received_data_length] = '\0';  // null 종료 문자 보장
        result.length = received_data_length;
        result.timestamp = last_receive_time;
        result.valid = true;
    }

    // 내부 수신 버퍼 클리어
    received_data_length = 0;
    last_receive_time = 0;

    return result;
}

bool BLEManager::sendResponse(const String& response) {
    // 초기화 및 연결 상태 확인
    if (!initialized || !char_tx || connected_clients == 0) {
        return false;
    }

    try {
        // 응답 데이터 설정 및 알림 전송
        char_tx->setValue(response.c_str());
        char_tx->notify();
        
        // 송신 통계 업데이트
        updateStatistics(0, response.length());
        
        return true;
    } catch (...) {
        return false;
    }
}

BLEConnectionState BLEManager::getConnectionState() {
    return connection_state;
}

uint8_t BLEManager::getConnectedClientCount() {
    return connected_clients;
}

bool BLEManager::restartAdvertising() {
    if (!initialized || !ble_advertising) {
        return false;
    }

    try {
        // 기존 광고 중단
        ble_advertising->stop();
        HIDUtils::safeDelay(500);
        
        // 광고 재시작
        bool result = ble_advertising->start();
        if (result && connected_clients == 0) {
            updateConnectionState(BLE_STATE_ADVERTISING);
        }
        
        return result;
    } catch (...) {
        return false;
    }
}

bool BLEManager::updateConnectionParams(uint16_t min_interval, uint16_t max_interval, 
                                      uint16_t latency, uint16_t timeout) {
    // 매개변수 유효성 검사
    if (min_interval > max_interval || min_interval < 6 || max_interval > 3200) {
        return false;
    }

    // NimBLE에서는 연결 매개변수 업데이트가 자동으로 처리됨
    // 필요시 추후 구현 가능
    return true;
}

bool BLEManager::disconnectAllClients() {
    if (!initialized || !ble_server) {
        return true;  // 이미 연결이 없는 상태
    }

    try {
        // 모든 클라이언트 연결 해제
        ble_server->disconnect(0);  // 0은 모든 연결을 의미
        
        // 상태 업데이트
        connected_clients = 0;
        updateConnectionState(BLE_STATE_ADVERTISING);
        
        return true;
    } catch (...) {
        return false;
    }
}

bool BLEManager::isSystemHealthy() {
    // 초기화 상태 확인
    if (!initialized) {
        return false;
    }

    // 필수 객체들 존재 확인
    if (!ble_server || !ble_service || !char_rx || !char_tx) {
        return false;
    }

    // NimBLE 스택 상태 확인
    if (!NimBLEDevice::getInitialized()) {
        return false;
    }

    return true;
}

void BLEManager::getStatistics(uint32_t& bytes_received, uint32_t& bytes_sent, uint32_t& connection_count) {
    bytes_received = total_bytes_received;
    bytes_sent = total_bytes_sent;
    connection_count = total_connections;
}

bool BLEManager::initializeReceiveBuffer() {
    // 기존 버퍼 정리
    if (receive_buffer) {
        cleanupReceiveBuffer();
    }

    // 새 버퍼 할당
    receive_buffer_size = MAX_MESSAGE_LENGTH;
    receive_buffer = new char[receive_buffer_size];
    
    if (!receive_buffer) {
        receive_buffer_size = 0;
        return false;
    }

    // 버퍼 상태 초기화
    received_data_length = 0;
    last_receive_time = 0;
    
    return true;
}

void BLEManager::cleanupReceiveBuffer() {
    if (receive_buffer) {
        delete[] receive_buffer;
        receive_buffer = nullptr;
    }
    
    receive_buffer_size = 0;
    received_data_length = 0;
    last_receive_time = 0;
}

bool BLEManager::storeReceivedData(const uint8_t* data, size_t length) {
    // 입력 유효성 검사
    if (!receive_buffer || length == 0 || length > receive_buffer_size) {
        return false;
    }

    // 새로운 데이터로 버퍼 업데이트 (최신 데이터만 유지)
    memcpy(receive_buffer, data, length);
    received_data_length = length;
    last_receive_time = millis();
    
    return true;
}

void BLEManager::updateConnectionState(BLEConnectionState new_state) {
    if (connection_state != new_state) {
        connection_state = new_state;
        // 상태 변경 시 추가 처리가 필요한 경우 여기에 구현
    }
}

void BLEManager::updateStatistics(size_t bytes_received, size_t bytes_sent) {
    total_bytes_received += bytes_received;
    total_bytes_sent += bytes_sent;
}