#include <Arduino.h>
#include "korean/korean_usb_hid.h"

/**
 * @file main_step3.cpp
 * @brief STEP 3 테스트용 메인 파일
 * 
 * USB Descriptor 기본 구현 테스트
 * - 한국어 키보드 인식 테스트
 * - 기본 키 입력 테스트
 * - 한영 전환 기능 테스트
 */

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n===============================================");
    Serial.println("  GHOSTYPE - Korean USB HID Test (STEP 3)");
    Serial.println("===============================================");
    Serial.println("Testing basic USB Descriptor implementation");
    Serial.println("Expected: Windows recognizes as Korean keyboard");
    Serial.println("===============================================\n");
    
    // 한국어 키보드 초기화
    Serial.println("🔧 Initializing Korean USB HID...");
    
    if (KoreanKeyboard.begin()) {
        Serial.println("✅ Korean USB HID initialized successfully");
        
        // 설정 정보 출력
        Serial.println("\n📊 Configuration:");
        Serial.println("   - VID: 0x04E8 (Samsung Electronics)");
        Serial.println("   - PID: 0x7021 (Korean USB Keyboard)");
        Serial.println("   - Country Code: 16 (Korean)");
        Serial.println("   - Language: Korean(0x0412) + English(0x0409)");
        
        // 상태 출력
        KoreanKeyboard.printStatus();
        
    } else {
        Serial.println("❌ Korean USB HID initialization failed");
        Serial.println("🔧 Please check:");
        Serial.println("   - TinyUSB library installation");
        Serial.println("   - USB cable connection");
        Serial.println("   - ESP32-S3 USB mode");
        return;
    }
    
    Serial.println("\n🔍 Waiting for USB connection...");
    Serial.println("📋 Please check Windows Device Manager:");
    Serial.println("   1. Win+X → Device Manager");
    Serial.println("   2. Expand 'Keyboards' category");
    Serial.println("   3. Look for 'Korean USB Keyboard' or 'Samsung Electronics'");
    Serial.println("   4. Right-click → Properties → Details");
    Serial.println("   5. Hardware ID should be 'USB\\VID_04E8&PID_7021'");
    Serial.println("");
    
    // 연결 대기
    int wait_dots = 0;
    while (!KoreanKeyboard.isConnected()) {
        delay(500);
        Serial.print(".");
        wait_dots++;
        
        if (wait_dots > 20) {
            Serial.println("\n⚠️  Connection timeout - continuing anyway");
            break;
        }
    }
    
    if (KoreanKeyboard.isConnected()) {
        Serial.println("\n✅ USB connected successfully!");
    }
    
    Serial.println("\n🧪 Starting basic functionality tests...");
    Serial.println("📝 Open Notepad to observe the output");
    Serial.println("⏱️  Tests will run every 10 seconds");
    Serial.println("");
    
    // 첫 번째 테스트 실행
    delay(3000);
    runConnectionTest();
}

void loop() {
    static unsigned long last_test = 0;
    static int test_cycle = 0;
    
    // 10초마다 테스트 실행
    if (millis() - last_test > 10000) {
        test_cycle++;
        
        Serial.printf("\n🔄 Test Cycle %d\n", test_cycle);
        Serial.println("================");
        
        switch (test_cycle % 5) {
            case 1:
                testBasicKeys();
                break;
            case 2:
                testHangulToggle();
                break;
            case 3:
                testDirectHangulKeys();
                break;
            case 4:
                testConsumerKeys();
                break;
            case 0:
                showStatus();
                break;
        }
        
        last_test = millis();
    }
    
    delay(100);
}

void runConnectionTest() {
    Serial.println("🔍 Connection Test");
    Serial.println("------------------");
    
    Serial.printf("USB Connected: %s\n", KoreanKeyboard.isConnected() ? "✅ Yes" : "❌ No");
    Serial.printf("HID Initialized: %s\n", KoreanKeyboard.isInitialized() ? "✅ Yes" : "❌ No");
    
    if (KoreanKeyboard.isConnected() && KoreanKeyboard.isInitialized()) {
        Serial.println("✅ Ready for testing!");
        
        // 간단한 식별 문자 전송
        Serial.println("📝 Sending identification text...");
        sendText("GHOSTYPE Korean HID Test - STEP 3");
        sendEnter();
        
    } else {
        Serial.println("⚠️  Connection or initialization issue");
    }
}

void testBasicKeys() {
    Serial.println("🔤 Basic Key Test");
    Serial.println("-----------------");
    
    Serial.println("📝 Sending: 'Hello Korean Keyboard'");
    
    sendText("Hello Korean Keyboard");
    sendEnter();
    
    Serial.println("✅ Basic key test completed");
}

void testHangulToggle() {
    Serial.println("🔄 Hangul Toggle Test");
    Serial.println("---------------------");
    
    Serial.printf("Current mode: %s\n", (KoreanKeyboard.getCurrentMode() == LANG_MODE_KOREAN) ? "Korean" : "English");
    
    Serial.println("📝 Attempting hangul toggle...");
    
    if (KoreanKeyboard.toggleLanguage()) {
        Serial.println("✅ Toggle command sent successfully");
    } else {
        Serial.println("❌ Toggle command failed");
    }
    
    delay(1000);
    
    Serial.printf("New mode: %s\n", (KoreanKeyboard.getCurrentMode() == LANG_MODE_KOREAN) ? "Korean" : "English");
    
    // 모드에 따른 테스트 텍스트 전송
    if (KoreanKeyboard.getCurrentMode() == LANG_MODE_KOREAN) {
        Serial.println("📝 Sending Korean test (dkssudgksepy - 안녕하세요)");
        sendText("dkssudgksepy");
    } else {
        Serial.println("📝 Sending English test");
        sendText("English Mode Active");
    }
    
    sendEnter();
    Serial.println("✅ Hangul toggle test completed");
}

void testDirectHangulKeys() {
    Serial.println("🔑 Direct Hangul Key Test");
    Serial.println("-------------------------");
    
    Serial.println("📝 Sending direct hangul key (0x90)...");
    
    if (KoreanKeyboard.sendKey(0x90)) {
        Serial.println("✅ Direct hangul key sent");
    } else {
        Serial.println("❌ Direct hangul key failed");
    }
    
    delay(1000);
    
    Serial.println("📝 Sending direct hanja key (0x91)...");
    
    if (KoreanKeyboard.sendKey(0x91)) {
        Serial.println("✅ Direct hanja key sent");
    } else {
        Serial.println("❌ Direct hanja key failed");
    }
    
    delay(1000);
    
    Serial.println("✅ Direct hangul key test completed");
}

void testConsumerKeys() {
    Serial.println("🎛️ Consumer Key Test");
    Serial.println("--------------------");
    
    Serial.println("📝 Sending consumer hangul toggle (0x0090)...");
    
    if (KoreanKeyboard.sendConsumerKey(0x0090)) {
        Serial.println("✅ Consumer hangul key sent");
    } else {
        Serial.println("❌ Consumer hangul key failed");
    }
    
    delay(1000);
    
    Serial.println("📝 Sending consumer hanja toggle (0x0091)...");
    
    if (KoreanKeyboard.sendConsumerKey(0x0091)) {
        Serial.println("✅ Consumer hanja key sent");
    } else {
        Serial.println("❌ Consumer hanja key failed");
    }
    
    delay(1000);
    
    Serial.println("✅ Consumer key test completed");
}

void showStatus() {
    Serial.println("📊 Status Report");
    Serial.println("----------------");
    
    KoreanKeyboard.printStatus();
    KoreanKeyboard.printStats();
    
    Serial.println("✅ Status report completed");
}

// 헬퍼 함수들
void sendText(const char* text) {
    for (int i = 0; text[i] != '\0'; i++) {
        char c = text[i];
        uint8_t keycode = charToKeycode(c);
        
        if (keycode != 0) {
            KoreanKeyboard.sendKey(keycode);
            delay(50);
        }
    }
}

void sendEnter() {
    KoreanKeyboard.sendKey(0x28); // Enter key
    delay(100);
}

uint8_t charToKeycode(char c) {
    // 간단한 ASCII to HID keycode 매핑
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
    
    return 0; // 매핑되지 않은 문자
}