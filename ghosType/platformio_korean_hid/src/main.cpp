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
    Serial.printf("🔄 테스트 %d: ", counter);
    
    // 12가지 한영 전환 방법 순환 테스트
    switch (counter % 12) {
        case 1:
            Serial.println("Alt + Shift (좌측)");
            keyboard.press(KEY_LEFT_ALT);
            keyboard.press(KEY_LEFT_SHIFT);
            delay(100);
            keyboard.releaseAll();
            break;
            
        case 2:
            Serial.println("Alt + Shift (우측)");
            keyboard.press(KEY_RIGHT_ALT);
            keyboard.press(KEY_RIGHT_SHIFT);
            delay(100);
            keyboard.releaseAll();
            break;
            
        case 3:
            Serial.println("Ctrl + Space");
            keyboard.press(KEY_LEFT_CTRL);
            keyboard.press(' ');
            delay(100);
            keyboard.releaseAll();
            break;
            
        case 4:
            Serial.println("Shift + Space");
            keyboard.press(KEY_LEFT_SHIFT);
            keyboard.press(' ');
            delay(100);
            keyboard.releaseAll();
            break;
            
        case 5:
            Serial.println("한글 키 (0xF2)");
            keyboard.press(0xF2);
            delay(100);
            keyboard.releaseAll();
            break;
            
        case 6:
            Serial.println("Right Alt 단독");
            keyboard.press(KEY_RIGHT_ALT);
            delay(100);
            keyboard.releaseAll();
            break;
            
        case 7:
            Serial.println("Left Alt 단독");
            keyboard.press(KEY_LEFT_ALT);
            delay(100);
            keyboard.releaseAll();
            break;
            
        case 8:
            Serial.println("Win + Space");
            keyboard.press(KEY_LEFT_GUI);
            keyboard.press(' ');
            delay(100);
            keyboard.releaseAll();
            break;
            
        case 9:
            Serial.println("F9 키");
            keyboard.press(KEY_F9);
            delay(100);
            keyboard.releaseAll();
            break;
            
        case 10:
            Serial.println("Menu 키");
            keyboard.press(KEY_MENU);
            delay(100);
            keyboard.releaseAll();
            break;
            
        case 11:
            Serial.println("한자 키 (0xF1)");
            keyboard.press(0xF1);
            delay(100);
            keyboard.releaseAll();
            break;
            
        case 0:
            Serial.println("Ctrl + Shift");
            keyboard.press(KEY_LEFT_CTRL);
            keyboard.press(KEY_LEFT_SHIFT);
            delay(100);
            keyboard.releaseAll();
            break;
    }
    
    delay(300);
    
    // 테스트 문자 전송
    keyboard.write('A');
    delay(200);
    keyboard.write(KEY_RETURN);
    
    delay(1000);  // 1초 대기
}