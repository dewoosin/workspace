#include <Arduino.h>
#include "korean/korean_usb_hid.h"

/**
 * @file test_korean_hid.cpp
 * @brief 한국어 키보드 기본 테스트
 * 
 * STEP 3 테스트용 기본 코드
 * - USB HID 초기화 테스트
 * - 기본 키 입력 테스트
 * - 한영 전환 테스트
 */

// 테스트 상태
bool test_initialized = false;
unsigned long last_test_time = 0;
int test_step = 0;

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n=== Korean USB HID Test v1.0 ===");
    Serial.println("STEP 3: Basic USB Descriptor Implementation Test");
    Serial.println("================================================");
    
    // 한국어 키보드 초기화
    Serial.println("1. Initializing Korean USB HID...");
    
    if (KoreanKeyboard.begin()) {
        Serial.println("   ✓ Korean USB HID initialized successfully");
        test_initialized = true;
        
        // 상태 출력
        KoreanKeyboard.printStatus();
        
        // 지원되는 전환 방식 출력
        KoreanKeyboard.printSupportedMethods();
        
    } else {
        Serial.println("   ✗ Korean USB HID initialization failed");
        return;
    }
    
    Serial.println("\n2. Waiting for USB connection...");
    Serial.println("   Please check Windows Device Manager:");
    Serial.println("   - Look for 'Samsung Electronics' under Keyboards");
    Serial.println("   - Hardware ID should be 'USB\\VID_04E8&PID_7021'");
    Serial.println("   - Product name should be 'Korean USB Keyboard'");
    Serial.println("");
    
    // 연결 대기
    while (!KoreanKeyboard.isConnected()) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("\n   ✓ USB connected!");
    Serial.println("\n3. Starting basic tests...");
    Serial.println("   Open Notepad and observe the output");
    Serial.println("");
    
    last_test_time = millis();
}

void loop() {
    if (!test_initialized) {
        delay(1000);
        return;
    }
    
    // 5초마다 테스트 실행
    if (millis() - last_test_time > 5000) {
        runBasicTest();
        last_test_time = millis();
    }
    
    delay(100);
}

void runBasicTest() {
    test_step++;
    
    Serial.printf("=== Test Step %d ===\n", test_step);
    
    switch (test_step) {
        case 1:
            testBasicKeyInput();
            break;
            
        case 2:
            testHangulToggle();
            break;
            
        case 3:
            testAllToggleMethods();
            break;
            
        case 4:
            testConsumerKeys();
            break;
            
        case 5:
            testKeyboardStatus();
            break;
            
        default:
            // 테스트 완료 - 리셋
            Serial.println("=== All Tests Complete ===");
            Serial.println("Results should be visible in Notepad");
            Serial.println("Restarting tests in 10 seconds...");
            Serial.println("");
            
            delay(10000);
            test_step = 0;
            break;
    }
}

void testBasicKeyInput() {
    Serial.println("Test 1: Basic Key Input");
    Serial.println("Expected: 'Hello World' in Notepad");
    
    // 기본 키 입력 테스트
    KoreanKeyboard.sendKey(0x0B); // 'H'
    delay(100);
    KoreanKeyboard.sendKey(0x08); // 'e'
    delay(100);
    KoreanKeyboard.sendKey(0x0F); // 'l'
    delay(100);
    KoreanKeyboard.sendKey(0x0F); // 'l'
    delay(100);
    KoreanKeyboard.sendKey(0x12); // 'o'
    delay(100);
    KoreanKeyboard.sendKey(0x2C); // Space
    delay(100);
    KoreanKeyboard.sendKey(0x1A); // 'W'
    delay(100);
    KoreanKeyboard.sendKey(0x12); // 'o'
    delay(100);
    KoreanKeyboard.sendKey(0x15); // 'r'
    delay(100);
    KoreanKeyboard.sendKey(0x0F); // 'l'
    delay(100);
    KoreanKeyboard.sendKey(0x07); // 'd'
    delay(100);
    KoreanKeyboard.sendKey(0x28); // Enter
    delay(500);
    
    Serial.println("✓ Basic key input test completed");
}

void testHangulToggle() {
    Serial.println("Test 2: Hangul Toggle");
    Serial.println("Expected: Language should toggle");
    
    // 현재 모드 출력
    Serial.printf("Current mode: %s\n", (KoreanKeyboard.getCurrentMode() == LANG_MODE_KOREAN) ? "Korean" : "English");
    
    // 한영 전환 테스트
    if (KoreanKeyboard.toggleLanguage()) {
        Serial.println("✓ Hangul toggle successful");
    } else {
        Serial.println("✗ Hangul toggle failed");
    }
    
    // 변경된 모드 출력
    Serial.printf("New mode: %s\n", (KoreanKeyboard.getCurrentMode() == LANG_MODE_KOREAN) ? "Korean" : "English");
    
    delay(1000);
}

void testAllToggleMethods() {
    Serial.println("Test 3: All Toggle Methods");
    Serial.println("Expected: Each method should be attempted");
    
    // 모든 전환 방식 테스트
    for (int method = 1; method <= 12; method++) {
        Serial.printf("Testing method %d...\n", method);
        
        KoreanKeyboard.setToggleMethod((hangul_toggle_method_t)method);
        
        if (KoreanKeyboard.toggleLanguage()) {
            Serial.printf("✓ Method %d: SUCCESS\n", method);
        } else {
            Serial.printf("✗ Method %d: FAILED\n", method);
        }
        
        delay(1000);
    }
    
    // 기본 방식으로 복원
    KoreanKeyboard.setToggleMethod(HANGUL_TOGGLE_RIGHT_ALT);
    Serial.println("✓ All toggle methods test completed");
}

void testConsumerKeys() {
    Serial.println("Test 4: Consumer Keys");
    Serial.println("Expected: Consumer control keys sent");
    
    // Consumer Control 키 테스트
    if (KoreanKeyboard.sendConsumerKey(CONSUMER_HANGUL_TOGGLE)) {
        Serial.println("✓ Consumer Hangul key sent");
    } else {
        Serial.println("✗ Consumer Hangul key failed");
    }
    
    delay(500);
    
    if (KoreanKeyboard.sendConsumerKey(CONSUMER_HANJA_TOGGLE)) {
        Serial.println("✓ Consumer Hanja key sent");
    } else {
        Serial.println("✗ Consumer Hanja key failed");
    }
    
    delay(500);
    
    Serial.println("✓ Consumer keys test completed");
}

void testKeyboardStatus() {
    Serial.println("Test 5: Keyboard Status");
    
    // 상태 정보 출력
    KoreanKeyboard.printStatus();
    KoreanKeyboard.printStats();
    
    Serial.println("✓ Status test completed");
}