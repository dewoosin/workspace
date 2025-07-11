/**
 * GHOSTYPE Korean HID - 최소 버전 (TinyUSB 완전 제거)
 */

#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=== ESP32 기본 테스트 ===");
    Serial.println("TinyUSB 없이 테스트 중...");
    
    delay(5000);
}

void loop() {
    static int counter = 0;
    
    counter++;
    Serial.printf("🔄 테스트 %d초\n", counter);
    
    delay(1000);
}