/**
 * GHOSTYPE Korean HID - USB 키보드 테스트
 */

#include <Arduino.h>
#include <USB.h>
#include <USBHIDKeyboard.h>

USBHIDKeyboard keyboard;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=== ESP32 USB 키보드 테스트 ===");
    
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
    Serial.println("10초 후 키보드 테스트 시작...");
    Serial.println("📝 메모장을 열어두세요!");
    
    delay(10000);
}

void loop() {
    static int counter = 0;
    
    counter++;
    Serial.printf("🔄 키보드 테스트 %d\n", counter);
    
    // 'A' 키 전송
    Serial.println("📝 'A' 키 전송");
    keyboard.write('A');
    delay(500);
    
    // 엔터 키 전송
    Serial.println("📝 엔터 키 전송");
    keyboard.write(KEY_RETURN);
    delay(1000);
    
    // 3번째마다 한영 전환 시도
    if (counter % 3 == 0) {
        Serial.println("📝 한영 전환 키 전송 (Right Alt)");
        keyboard.press(KEY_RIGHT_ALT);
        delay(100);
        keyboard.releaseAll();
        delay(500);
    }
    
    delay(3000);  // 3초 대기
}