#include <Arduino.h>

void setup() {
    // 아무것도 하지 않음
    pinMode(2, OUTPUT);  // LED 핀만 설정
}

void loop() {
    // LED 깜빡임으로 정상 작동 확인
    digitalWrite(2, HIGH);
    delay(1000);
    digitalWrite(2, LOW);
    delay(1000);
}