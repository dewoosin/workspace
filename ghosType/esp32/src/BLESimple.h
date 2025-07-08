#pragma once
#include <NimBLEDevice.h>

// 간단한 BLE 서버 클래스 (문제 해결용)
// Simple BLE server class (for troubleshooting)

class BLESimple {
public:
    BLESimple();
    
    bool begin();
    void stop();
    
    bool isConnected();
    bool hasReceivedData();
    std::string getReceivedData();
    void sendNotification(const char* data);
    
    static BLESimple* instance;
    bool deviceConnected;
    std::string receivedData;

private:
    NimBLEServer* pServer;
    NimBLECharacteristic* pCharacteristicRX;
    NimBLECharacteristic* pCharacteristicTX;
    
    // 콜백 클래스들
    class ServerCallbacks : public NimBLEServerCallbacks {
    public:
        void onConnect(NimBLEServer* pServer) override;
        void onDisconnect(NimBLEServer* pServer) override;
    };
    
    class CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
    public:
        void onWrite(NimBLECharacteristic* pCharacteristic) override;
    };
};