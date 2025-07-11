/**
 * GHOSTYPE Korean HID - PlatformIO Version (Simplified)
 */

#include <Arduino.h>
#include "Adafruit_TinyUSB.h"
#include "config.h"
#include "usb_descriptors.h"

// 전역 변수
KoreanUSBHID koreanKeyboard;
bool system_initialized = false;

// 함수 선언
void runInitialTest();
void testBasicKeys();
void testHangulToggle();
void testConsumerKeys();
void showSystemStatus();
void sendSimpleText(const char* text);
void sendEnter();
uint8_t charToKeycode(char c);

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n===============================================");
    Serial.println("  GHOSTYPE - Korean USB HID (PlatformIO)");
    Serial.println("===============================================");
    Serial.println("Initializing Korean keyboard functionality...");
    Serial.println("===============================================\n");
    
    // 한국어 키보드 초기화
    Serial.println("🔧 Initializing Korean USB HID...");
    
    if (koreanKeyboard.begin()) {
        Serial.println("✅ Korean USB HID initialized successfully");
        system_initialized = true;
        
        // 설정 정보 출력
        Serial.println("\n📊 Configuration:");
        Serial.println("   - VID: 0x04E8 (Samsung Electronics)");
        Serial.println("   - PID: 0x7021 (Korean USB Keyboard)");
        Serial.println("   - Country Code: 16 (Korean)");
        Serial.println("   - Language: Korean + English");
        
        // 상태 출력
        koreanKeyboard.printStatus();
        
        // 연결 대기
        Serial.println("\n🔍 Waiting for USB connection...");
        Serial.println("📋 Check Windows Device Manager for 'Korean USB Keyboard'");
        
        // 첫 번째 테스트
        delay(3000);
        runInitialTest();
        
    } else {
        Serial.println("❌ Korean USB HID initialization failed");
        Serial.println("🔧 Please check:");
        Serial.println("   - TinyUSB library installation");
        Serial.println("   - USB cable connection");
        Serial.println("   - Board settings (USB-OTG mode)");
        return;
    }
    
    Serial.println("\n🧪 System ready - Tests will run every 10 seconds");
    Serial.println("📝 Open Notepad to observe keyboard output");
    Serial.println("");
}

void loop() {
    if (!system_initialized) {
        delay(1000);
        return;
    }
    
    static unsigned long last_test = 0;
    static int test_cycle = 0;
    
    // 10초마다 테스트 실행
    if (millis() - last_test > 10000) {
        test_cycle++;
        
        Serial.printf("\n🔄 Test Cycle %d\n", test_cycle);
        Serial.println("================");
        
        switch (test_cycle % 4) {
            case 1:
                testBasicKeys();
                break;
            case 2:
                testHangulToggle();
                break;
            case 3:
                testConsumerKeys();
                break;
            case 0:
                showSystemStatus();
                break;
        }
        
        last_test = millis();
    }
    
    delay(100);
}

void runInitialTest() {
    Serial.println("🔍 Initial System Test");
    Serial.println("----------------------");
    
    // 연결 상태 확인
    Serial.printf("USB Connected: %s\n", koreanKeyboard.isConnected() ? "✅ Yes" : "❌ No");
    Serial.printf("HID Initialized: %s\n", koreanKeyboard.isInitialized() ? "✅ Yes" : "❌ No");
    
    if (koreanKeyboard.isConnected() && koreanKeyboard.isInitialized()) {
        Serial.println("✅ System ready for testing!");
        
        // 식별 텍스트 전송
        Serial.println("📝 Sending identification text...");
        sendSimpleText("GHOSTYPE Korean HID - PlatformIO Test");
        sendEnter();
        
    } else {
        Serial.println("⚠️  System not ready - continuing with tests anyway");
    }
}

void testBasicKeys() {
    Serial.println("🔤 Basic Key Input Test");
    Serial.println("-----------------------");
    
    Serial.println("📝 Sending: 'Hello Korean World'");
    
    sendSimpleText("Hello Korean World");
    sendEnter();
    
    Serial.println("✅ Basic key test completed");
}

void testHangulToggle() {
    Serial.println("🔄 Hangul Toggle Test");
    Serial.println("---------------------");
    
    Serial.printf("Current mode: %s\n", (koreanKeyboard.getCurrentMode() == LANG_MODE_KOREAN) ? "Korean" : "English");
    
    Serial.println("📝 Attempting hangul toggle...");
    
    // 한영 전환 시도
    bool success = koreanKeyboard.toggleLanguage();
    
    if (success) {
        Serial.println("✅ Toggle command sent successfully");
        
        delay(1000);
        
        // 모드 확인
        Serial.printf("New mode: %s\n", (koreanKeyboard.getCurrentMode() == LANG_MODE_KOREAN) ? "Korean" : "English");
        
        // 테스트 텍스트 전송
        if (koreanKeyboard.getCurrentMode() == LANG_MODE_KOREAN) {
            Serial.println("📝 Sending Korean test pattern (should type 안녕)");
            sendSimpleText("dkssud"); // 안녕
        } else {
            Serial.println("📝 Sending English test");
            sendSimpleText("English Mode");
        }
        
        sendEnter();
        
    } else {
        Serial.println("❌ Toggle command failed");
    }
    
    Serial.println("✅ Hangul toggle test completed");
}

void testConsumerKeys() {
    Serial.println("🎛️ Consumer Control Test");
    Serial.println("------------------------");
    
    Serial.println("📝 Testing consumer hangul key...");
    
    if (koreanKeyboard.sendConsumerKey(CONSUMER_HANGUL_TOGGLE)) {
        Serial.println("✅ Consumer hangul key sent");
    } else {
        Serial.println("❌ Consumer hangul key failed");
    }
    
    delay(1000);
    
    Serial.println("📝 Testing direct hangul key...");
    
    if (koreanKeyboard.sendKey(HID_KEY_HANGUL)) {
        Serial.println("✅ Direct hangul key sent");
    } else {
        Serial.println("❌ Direct hangul key failed");
    }
    
    delay(1000);
    
    Serial.println("✅ Consumer key test completed");
}

void showSystemStatus() {
    Serial.println("📊 System Status");
    Serial.println("----------------");
    
    koreanKeyboard.printStatus();
    koreanKeyboard.printStats();
    
    Serial.println("✅ Status report completed");
}

// 헬퍼 함수들
void sendSimpleText(const char* text) {
    for (int i = 0; text[i] != '\0'; i++) {
        char c = text[i];
        uint8_t keycode = charToKeycode(c);
        
        if (keycode != 0) {
            koreanKeyboard.sendKey(keycode);
            delay(50);
        }
    }
}

void sendEnter() {
    koreanKeyboard.sendKey(0x28); // Enter key
    delay(100);
}

uint8_t charToKeycode(char c) {
    // 기본 ASCII to HID keycode 변환
    if (c >= 'a' && c <= 'z') {
        return c - 'a' + 0x04;
    } else if (c >= 'A' && c <= 'Z') {
        return c - 'A' + 0x04;
    } else if (c >= '0' && c <= '9') {
        return c - '0' + 0x1E;
    } else if (c == ' ') {
        return 0x2C; // Space
    } else if (c == '-') {
        return 0x2D; // Minus
    } else if (c == '.') {
        return 0x37; // Period
    }
    
    return 0;
}