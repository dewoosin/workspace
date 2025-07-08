#pragma once
#include <Arduino.h>
#include <NimBLEDevice.h>
#include <string>

class BLENimbleManager {
private:
    NimBLEServer* pServer;
    NimBLECharacteristic* pCharacteristicRX;
    NimBLECharacteristic* pCharacteristicTX;
    NimBLEAdvertising* pAdvertising;
    bool deviceConnected;
    std::string receivedData;
    std::string fragmentBuffer;  // Buffer for fragmented packets / 분할된 패킷용 버퍼
    unsigned long fragmentStartTime;  // Timestamp when fragmentation started / 분할 시작 시간
    static const uint32_t FRAGMENT_TIMEOUT = 5000;  // 5 second timeout / 5초 타임아웃
    static const size_t MAX_FRAGMENT_SIZE = 1024;    // Maximum fragment buffer size / 최대 분할 버퍼 크기
    
    class ServerCallbacks;
    class CharacteristicCallbacks;
    
public:
    BLENimbleManager();
    ~BLENimbleManager();
    
    bool begin();
    void stop();
    bool hasReceivedData();
    std::string getReceivedData();
    void sendNotification(const char* data);
    void printStatus();
    bool isConnected();
    void checkFragmentTimeout();  // Check and clear expired fragments / 만료된 분할 확인 및 제거
};