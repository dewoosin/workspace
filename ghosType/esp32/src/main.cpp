#include <Arduino.h>

#define BUTTON_PIN 0

void setup() {
    Serial.begin(115200);
    delay(3000);  // 충분한 시간 대기
    
    Serial.println("\n\n!!!! NEW BLE CODE RUNNING !!!!");
    Serial.println("=== T-Dongle-S3 BLE Test ===");
    Serial.println("이 메시지가 보이면 새 코드가 실행중입니다");
    
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    
    Serial.println("BLE 코드 로드 확인됨!");
}

void loop() {
    static bool buttonPressed = false;
    static int buttonCount = 0;
    
    if (digitalRead(BUTTON_PIN) == LOW && !buttonPressed) {
        buttonPressed = true;
        buttonCount++;
        
        Serial.print("NEW CODE - Button Count: ");
        Serial.println(buttonCount);
        
    } else if (digitalRead(BUTTON_PIN) == HIGH) {
        buttonPressed = false;
    }
    
    delay(50);
}