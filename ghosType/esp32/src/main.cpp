// Simple ESP32 BLE Test Firmware
// 간단한 ESP32 BLE 테스트 펌웨어

#include <Arduino.h>
#include "BLESimple.h"

BLESimple* bleSimple = nullptr;

void setup() {
    // 시리얼 없이 시작 (충돌 방지)
    // Start without serial (prevent conflicts)
    
    // BLE 시작
    bleSimple = new BLESimple();
    bool success = bleSimple->begin();
    
    // LED 초기화 (상태 표시용)
    pinMode(2, OUTPUT);
    
    if (success) {
        // 성공시 LED 켜기
        digitalWrite(2, HIGH);
        delay(1000);
        digitalWrite(2, LOW);
    } else {
        // 실패시 LED 깜빡임
        for (int i = 0; i < 5; i++) {
            digitalWrite(2, HIGH);
            delay(200);
            digitalWrite(2, LOW);
            delay(200);
        }
    }
}

void loop() {
    // 연결 상태 LED 표시
    if (bleSimple->isConnected()) {
        digitalWrite(2, HIGH);
    } else {
        digitalWrite(2, LOW);
    }
    
    // 수신된 데이터 처리
    if (bleSimple->hasReceivedData()) {
        std::string data = bleSimple->getReceivedData();
        
        // 간단한 응답 전송
        String response = "OK:" + String(data.length());
        bleSimple->sendNotification(response.c_str());
    }
    
    delay(100);
}