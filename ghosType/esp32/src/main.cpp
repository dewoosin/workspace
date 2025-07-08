// Simple ESP32 BLE Test Firmware
// 간단한 ESP32 BLE 테스트 펌웨어

#include <Arduino.h>
#include "BLESimple.h"

BLESimple* bleSimple = nullptr;

void setup() {
    // 디버깅을 위한 시리얼 초기화
    // Initialize serial for debugging
    Serial.begin(115200);
    delay(1000);
    Serial.println("=== ESP32 BLE 테스트 시작 ===");
    Serial.println("펌웨어 버전: Simple Test v1.0");
    
    // BLE 시작
    Serial.println("BLE 초기화 중...");
    bleSimple = new BLESimple();
    bool success = bleSimple->begin();
    
    // LED 초기화 (상태 표시용)
    pinMode(2, OUTPUT);
    
    if (success) {
        // 성공시 LED 켜기
        Serial.println("✅ BLE 초기화 성공!");
        Serial.println("장치명: ESP32");
        Serial.println("서비스 UUID: 12345678-1234-5678-9012-123456789abc");
        Serial.println("광고 시작됨 - 연결 대기 중...");
        digitalWrite(2, HIGH);
        delay(1000);
        digitalWrite(2, LOW);
    } else {
        // 실패시 LED 깜빡임
        Serial.println("❌ BLE 초기화 실패!");
        for (int i = 0; i < 5; i++) {
            digitalWrite(2, HIGH);
            delay(200);
            digitalWrite(2, LOW);
            delay(200);
        }
    }
}

void loop() {
    static bool lastConnected = false;
    static unsigned long lastHeartbeat = 0;
    
    // 연결 상태 변화 감지
    bool currentConnected = bleSimple->isConnected();
    if (currentConnected != lastConnected) {
        if (currentConnected) {
            Serial.println("🔗 BLE 클라이언트 연결됨!");
            digitalWrite(2, HIGH);
        } else {
            Serial.println("❌ BLE 클라이언트 연결 해제됨");
            digitalWrite(2, LOW);
        }
        lastConnected = currentConnected;
    }
    
    // 연결 상태 LED 표시
    if (currentConnected) {
        digitalWrite(2, HIGH);
    } else {
        digitalWrite(2, LOW);
    }
    
    // 수신된 데이터 처리
    if (bleSimple->hasReceivedData()) {
        std::string data = bleSimple->getReceivedData();
        Serial.printf("📨 수신된 데이터: '%s' (길이: %d)\n", data.c_str(), data.length());
        
        // 간단한 응답 전송
        String response = "OK:" + String(data.length());
        bleSimple->sendNotification(response.c_str());
        Serial.printf("📤 응답 전송: '%s'\n", response.c_str());
    }
    
    // 10초마다 하트비트 로그
    if (millis() - lastHeartbeat > 10000) {
        Serial.printf("💓 상태: %s | 업타임: %lu초\n", 
                     currentConnected ? "연결됨" : "대기중", 
                     millis() / 1000);
        lastHeartbeat = millis();
    }
    
    delay(100);
}