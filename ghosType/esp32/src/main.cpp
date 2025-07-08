#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("ESP32-S3 Starting...");
    Serial.println("Basic test - LED blink only");
    
    pinMode(2, OUTPUT);  // LED 핀 설정
    Serial.println("Setup complete!");
}

void loop() {
    // LED 깜빡임으로 정상 작동 확인
    digitalWrite(2, HIGH);
    Serial.println("LED ON");
    delay(1000);
    
    digitalWrite(2, LOW);
    Serial.println("LED OFF");
    delay(1000);
}