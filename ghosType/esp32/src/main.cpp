#include <Arduino.h>

// T-Dongle-S3 핀 정의
#define BUTTON_PIN 0    // 부트 버튼
#define LED_PIN 39      // T-Dongle-S3의 LED는 보통 GPIO39

void setup() {
    // 시리얼 초기화
    Serial.begin(115200);
    
    // USB CDC가 준비될 때까지 대기
    delay(2000);
    
    Serial.println("=== T-Dongle-S3 Test ===");
    Serial.println("Starting...");
    
    // 버튼 입력 설정
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    
    // LED 핀 시도 (T-Dongle-S3는 내장 LED가 없을 수 있음)
    pinMode(LED_PIN, OUTPUT);
    
    Serial.println("Setup complete!");
    Serial.println("Press BOOT button to test");
}

void loop() {
    static unsigned long lastPrint = 0;
    static bool buttonPressed = false;
    
    // 1초마다 상태 출력
    if (millis() - lastPrint > 1000) {
        Serial.print(".");
        lastPrint = millis();
    }
    
    // 버튼 상태 확인
    if (digitalRead(BUTTON_PIN) == LOW && !buttonPressed) {
        buttonPressed = true;
        Serial.println("\nBOOT Button Pressed!");
        
        // LED 토글 시도
        static bool ledState = false;
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState);
    } else if (digitalRead(BUTTON_PIN) == HIGH) {
        buttonPressed = false;
    }
}