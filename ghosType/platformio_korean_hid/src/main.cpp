/**
 * GHOSTYPE Korean HID - ESP32 네이티브 USB 버전
 */

#include <Arduino.h>
#include <USB.h>
#include <USBHIDKeyboard.h>

// 네이티브 USB HID 키보드 객체
USBHIDKeyboard keyboard;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=== ESP32 네이티브 USB HID 테스트 ===");
    
    // USB 설정
    USB.manufacturerName("Samsung Electronics");
    USB.productName("Korean USB Keyboard");
    USB.serialNumber("KR2024KB001");
    USB.VID(0x04E8);  // Samsung VID
    USB.PID(0x7021);  // Korean Keyboard PID
    
    // USB 시작
    Serial.println("USB 초기화 중...");
    USB.begin();
    
    // 키보드 시작
    Serial.println("키보드 초기화 중...");
    keyboard.begin();
    
    Serial.println("✅ 초기화 완료!");
    Serial.println("5초 후 테스트 시작...");
    delay(5000);
}

void loop() {
    static int counter = 0;
    
    counter++;
    Serial.printf("🔄 테스트 %d: 'A' 키 전송\n", counter);
    
    // 'A' 키 전송
    keyboard.write('A');
    delay(500);
    
    // 엔터 키 전송
    keyboard.write(KEY_RETURN);
    delay(500);
    
    // 한영 전환 키 전송 (Right Alt)
    if (counter % 3 == 0) {
        Serial.println("📝 한영 전환 키 전송");
        keyboard.press(KEY_RIGHT_ALT);
        delay(50);
        keyboard.releaseAll();
        delay(500);
    }
    
    delay(2000);  // 2초 대기
}