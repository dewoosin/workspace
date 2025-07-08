/**
 * @file ble_test.cpp
 * @brief 최소한의 BLE 광고 테스트
 * 
 * 이 파일은 BLE 연결 문제를 진단하기 위한 최소한의 테스트 코드입니다.
 * 복잡한 서비스나 특성 없이 단순 광고만 수행합니다.
 */

#include <Arduino.h>
#include <NimBLEDevice.h>

void setup() {
    // LED 핀 설정
    pinMode(2, OUTPUT);
    digitalWrite(2, LOW);
    
    // 초기화 지연
    delay(2000);
    
    // 초기화 신호 (LED 2회 깜빡임)
    for (int i = 0; i < 2; i++) {
        digitalWrite(2, HIGH);
        delay(300);
        digitalWrite(2, LOW);
        delay(300);
    }
    
    // 가장 기본적인 BLE 초기화
    NimBLEDevice::init("GHOSTYPE-TEST");
    
    // 기본 광고 시작
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->setName("GHOSTYPE-TEST");
    pAdvertising->setScanResponse(true);
    
    if (pAdvertising->start()) {
        // 광고 시작 성공 신호 (LED 계속 켜짐)
        digitalWrite(2, HIGH);
    } else {
        // 광고 시작 실패 신호 (빠른 깜빡임)
        while (true) {
            digitalWrite(2, HIGH);
            delay(100);
            digitalWrite(2, LOW);
            delay(100);
        }
    }
}

void loop() {
    // 광고 상태 유지만 수행
    delay(1000);
}