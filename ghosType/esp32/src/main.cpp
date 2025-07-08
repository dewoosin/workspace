/**
 * @file hid_test.cpp
 * @brief T-Dongle-S3 HID 키보드 테스트
 * 
 * BLE 없이 순수 HID 기능만 테스트
 */

#include <Arduino.h>
#include <USB.h>
#include <USBHIDKeyboard.h>

#define BUTTON_PIN 0

// HID 키보드 객체
USBHIDKeyboard keyboard;

// 한글 테스트용 텍스트
const char* testTexts[] = {
    "Hello World",
    "GHOSTYPE HID Test",
    "1234567890",
    "ESP32-S3 USB Keyboard"
};
int textIndex = 0;

// 함수 선언
void typeText(const char* text);

void setup() {
    Serial.begin(115200);
    
    // USB 초기화 대기
    delay(2000);
    
    Serial.println("\n=== T-Dongle-S3 HID Keyboard Test ===");
    Serial.println("USB HID 초기화 시작...");
    
    // 버튼 설정
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    
    // USB HID 초기화
    USB.begin();
    keyboard.begin();
    
    Serial.println("✓ USB HID 키보드 초기화 완료!");
    Serial.println("BOOT 버튼을 누르면 텍스트를 타이핑합니다.");
    Serial.println("메모장을 열고 테스트하세요!");
}

void loop() {
    static bool buttonPressed = false;
    static unsigned long lastTyping = 0;
    
    // 버튼 확인
    if (digitalRead(BUTTON_PIN) == LOW && !buttonPressed) {
        buttonPressed = true;
        
        // 연속 타이핑 방지 (2초 간격)
        if (millis() - lastTyping > 2000) {
            Serial.print("타이핑: ");
            Serial.println(testTexts[textIndex]);
            
            // 실제 타이핑
            typeText(testTexts[textIndex]);
            
            // 다음 텍스트로
            textIndex = (textIndex + 1) % 4;
            lastTyping = millis();
        }
        
    } else if (digitalRead(BUTTON_PIN) == HIGH) {
        buttonPressed = false;
    }
    
    delay(50);
}

void typeText(const char* text) {
    // 타이핑 시작 전 잠시 대기
    delay(500);
    
    // 한 글자씩 타이핑
    while (*text) {
        keyboard.write(*text);
        Serial.print(*text);
        text++;
        delay(50);  // 타이핑 속도
    }
    
    // Enter 키
    keyboard.write(KEY_RETURN);
    Serial.println("\n✓ 타이핑 완료!");
}