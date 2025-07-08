/**
 * @file simple_ble_test.cpp
 * @brief 가장 단순한 BLE 광고 테스트
 * 
 * 이전에 작동했던 최소한의 코드로 테스트
 */

#include <Arduino.h>
#include <NimBLEDevice.h>

void setup() {
    // 최소한의 초기화
    delay(1000);
    
    // BLE 초기화 - 가장 기본적인 설정만
    NimBLEDevice::init("GHOSTYPE");
    
    // 서버 생성
    NimBLEServer* pServer = NimBLEDevice::createServer();
    
    // 서비스 생성
    NimBLEService* pService = pServer->createService("12345678-1234-5678-9012-123456789abc");
    pService->start();
    
    // 광고 시작
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID("12345678-1234-5678-9012-123456789abc");
    pAdvertising->start();
}

void loop() {
    // 아무것도 하지 않음
    delay(2000);
}