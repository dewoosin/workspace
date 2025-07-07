// src/BLENimbleManager.h
// GHOSTYPE 상품화 버전 - NimBLE 기반 BLE 관리자
// 모든 플랫폼(Windows/iOS/Android) 호환성 보장

#ifndef BLE_NIMBLE_MANAGER_H
#define BLE_NIMBLE_MANAGER_H

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <NimBLEServer.h>
#include <NimBLEService.h>
#include <NimBLECharacteristic.h>
#include <NimBLEAdvertising.h>
#include <NimBLEUtils.h>
#include <NimBLEAddress.h>
#include <NimBLE2904.h>
#include <vector>
#include <map>
#include <queue>
#include <string>
#include "BLEConfig.h"

// 전방 선언
class BLENimbleManager;

// 연결된 디바이스 정보 구조체
struct ConnectedDevice {
    uint16_t connHandle;        // 연결 핸들
    std::string address;        // BLE 주소
    std::string name;           // 디바이스 이름
    uint32_t connectedTime;     // 연결 시작 시간
    uint32_t lastActivityTime;  // 마지막 활동 시간
    bool isSubscribed;          // Notify 구독 여부
    bool isAuthenticated;       // 인증 여부
    uint16_t mtu;              // 협상된 MTU 크기
    int8_t rssi;               // 신호 강도
};

// 페어링된 디바이스 정보 (NVS 저장용)
struct PairedDevice {
    uint8_t address[6];         // BLE MAC 주소
    char name[32];              // 디바이스 이름
    uint32_t lastConnected;     // 마지막 연결 시간
    uint8_t bondKey[16];        // 본딩 키 (선택사항)
};

// BLE 연결 상태
enum class BLEConnectionState {
    IDLE,                       // 대기 상태
    ADVERTISING,                // 광고 중
    CONNECTING,                 // 연결 중
    CONNECTED,                  // 연결됨
    DISCONNECTING,              // 연결 해제 중
    ERROR                       // 에러 상태
};

// BLE 서버 콜백 클래스
class ServerCallbacks : public NimBLEServerCallbacks {
private:
    BLENimbleManager* manager;
    
public:
    ServerCallbacks(BLENimbleManager* mgr) : manager(mgr) {}
    
    // 연결 관련 콜백
    void onConnect(NimBLEServer* pServer) override;
    void onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) override;
    void onDisconnect(NimBLEServer* pServer) override;
    void onDisconnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) override;
    
    // MTU 변경 콜백
    void onMTUChange(uint16_t MTU, ble_gap_conn_desc* desc) override;
    
    // 인증 관련 콜백
    uint32_t onPassKeyRequest() override;
    void onAuthenticationComplete(ble_gap_conn_desc* desc) override;
    bool onConfirmPIN(uint32_t pin) override;
};

// BLE 특성 콜백 클래스
class CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
private:
    BLENimbleManager* manager;
    std::string charType;       // 특성 종류 식별자
    
public:
    CharacteristicCallbacks(BLENimbleManager* mgr, const std::string& type = "") 
        : manager(mgr), charType(type) {}
    
    void onWrite(NimBLECharacteristic* pCharacteristic) override;
    void onRead(NimBLECharacteristic* pCharacteristic) override;
    void onNotify(NimBLECharacteristic* pCharacteristic) override;
    void onStatus(NimBLECharacteristic* pCharacteristic, Status status, int code) override;
    void onSubscribe(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc, uint16_t subValue) override;
};

// NimBLE 기반 BLE 매니저 클래스
class BLENimbleManager {
private:
    // ===== NimBLE 핵심 객체 =====
    NimBLEServer* pServer;
    NimBLEAdvertising* pAdvertising;
    
    // ===== 서비스 객체 =====
    // Nordic UART Service
    NimBLEService* pUartService;
    NimBLECharacteristic* pTxCharacteristic;
    NimBLECharacteristic* pRxCharacteristic;
    
    // Device Information Service
    NimBLEService* pDeviceInfoService;
    NimBLECharacteristic* pManufacturerChar;
    NimBLECharacteristic* pModelChar;
    NimBLECharacteristic* pSerialChar;
    NimBLECharacteristic* pFirmwareChar;
    NimBLECharacteristic* pHardwareChar;
    NimBLECharacteristic* pSoftwareChar;
    
    // Battery Service
    NimBLEService* pBatteryService;
    NimBLECharacteristic* pBatteryLevelChar;
    
    // ===== 콜백 객체 =====
    ServerCallbacks* serverCallbacks;
    CharacteristicCallbacks* rxCallbacks;
    CharacteristicCallbacks* txCallbacks;
    
    // ===== 연결 관리 =====
    std::map<uint16_t, ConnectedDevice> connectedDevices;  // 연결 핸들 -> 디바이스 정보
    std::vector<PairedDevice> pairedDevices;               // 페어링된 디바이스 목록
    
    // ===== 상태 관리 =====
    BLEConnectionState currentState;
    bool isInitialized;
    bool isAdvertising;
    uint8_t numConnections;
    std::string deviceNameWithMac;
    uint8_t deviceMAC[6];
    
    // ===== 수신 데이터 관리 (스레드 안전) =====
    QueueHandle_t rxQueue;
    SemaphoreHandle_t rxMutex;
    
    // ===== 통계 정보 =====
    uint32_t totalMessages;
    uint32_t totalBytes;
    uint32_t totalErrors;
    uint32_t startTime;
    uint32_t lastAdvertiseTime;
    
    // ===== 보안 관련 =====
    uint32_t fixedPasskey;
    bool isSecurityEnabled;
    
    // ===== 내부 메서드 =====
    void createServices();
    void createUartService();
    void createDeviceInfoService();
    void createBatteryService();
    void configureAdvertising();
    void startAdvertising();
    void generateDeviceName();
    void loadPairedDevices();
    void savePairedDevices();
    void addPairedDevice(const std::string& address, const std::string& name);
    bool isDevicePaired(const std::string& address);
    void updateConnectionParams(uint16_t connHandle);
    void cleanupStaleConnections();
    std::string getDeviceMAC();
    
public:
    // ===== 생성자/소멸자 =====
    BLENimbleManager();
    ~BLENimbleManager();
    
    // ===== 초기화 및 제어 =====
    bool begin();
    void stop();
    void reset();
    
    // ===== 연결 관리 =====
    uint8_t getConnectionCount() const { return numConnections; }
    bool isAnyDeviceConnected() const { return numConnections > 0; }
    bool isDeviceConnected(uint16_t connHandle) const;
    bool isDeviceConnected(const std::string& address) const;
    std::vector<ConnectedDevice> getConnectedDevices() const;
    void disconnectDevice(uint16_t connHandle);
    void disconnectDevice(const std::string& address);
    void disconnectAll();
    
    // ===== 데이터 송수신 =====
    bool sendData(const std::string& data, uint16_t connHandle = 0xFFFF);
    bool sendDataToAll(const std::string& data);
    bool sendDataToDevice(const std::string& data, const std::string& address);
    bool hasReceivedData();
    std::string getReceivedData();
    void clearReceivedData();
    
    // ===== 페어링 관리 =====
    std::vector<PairedDevice> getPairedDevices() const;
    void clearPairedDevices();
    void removePairedDevice(const std::string& address);
    
    // ===== 광고 제어 =====
    void pauseAdvertising();
    void resumeAdvertising();
    bool isCurrentlyAdvertising() const { return isAdvertising; }
    void updateAdvertisingData();
    
    // ===== 상태 및 통계 =====
    void printStatus();
    void printDetailedStatus();
    BLEConnectionState getState() const { return currentState; }
    std::string getStateString() const;
    uint32_t getUptimeSeconds() const;
    float getMessagesPerMinute() const;
    uint32_t getErrorCount() const { return totalErrors; }
    int8_t getDeviceRSSI(uint16_t connHandle);
    
    // ===== 배터리 관리 =====
    void updateBatteryLevel(uint8_t level);
    uint8_t getBatteryLevel();
    
    // ===== 보안 설정 =====
    void setSecurityAuth(bool bonding, bool mitm, bool sc);
    void setPasskey(uint32_t passkey);
    void enableSecurity(bool enable);
    
    // ===== 이벤트 처리 (콜백에서 호출) =====
    void handleConnect(uint16_t connHandle, const std::string& address);
    void handleDisconnect(uint16_t connHandle);
    void handleDataReceived(uint16_t connHandle, const std::string& data);
    void handleSubscriptionChange(uint16_t connHandle, bool subscribed);
    void handleAuthComplete(uint16_t connHandle, bool success);
    
    // ===== 디버그 및 유틸리티 =====
    void enableDebugMode(bool enable);
    std::string getDeviceName() const { return deviceNameWithMac; }
    std::string getMACAddress() const;
    
    // 친구 클래스 선언
    friend class ServerCallbacks;
    friend class CharacteristicCallbacks;
};

#endif // BLE_NIMBLE_MANAGER_H