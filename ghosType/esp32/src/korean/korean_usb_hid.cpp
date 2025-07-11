#include "korean/korean_usb_hid.h"

/**
 * @file korean_usb_hid.cpp
 * @brief 한국어 키보드 전용 USB HID 클래스 구현
 * 
 * TinyUSB를 직접 제어하여 완벽한 한국어 키보드 구현
 * Windows에서 Samsung Korean USB Keyboard로 인식
 */

// 전역 인스턴스
KoreanUSBHID KoreanKeyboard;

// 디버그 매크로
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINTF(fmt, ...) Serial.printf(fmt, __VA_ARGS__)

// 생성자
KoreanUSBHID::KoreanUSBHID() : USBHID() {
    _init_state();
    _reset_reports();
}

// 상태 초기화
void KoreanUSBHID::_init_state(void) {
    _state.is_initialized = false;
    _state.current_mode = LANG_MODE_ENGLISH;
    _state.toggle_method = HANGUL_TOGGLE_RIGHT_ALT;
    _state.last_toggle_time = 0;
    _state.toggle_count = 0;
    _state.debug_enabled = true;
}

// Report 버퍼 초기화
void KoreanUSBHID::_reset_reports(void) {
    memset(&_keyboard_report, 0, sizeof(_keyboard_report));
    memset(&_consumer_report, 0, sizeof(_consumer_report));
    memset(&_system_report, 0, sizeof(_system_report));
}

// HID Report 전송
bool KoreanUSBHID::_send_report(uint8_t report_id, const void* data, uint16_t len) {
    if (!_state.is_initialized) {
        DEBUG_PRINTLN("Error: HID not initialized");
        return false;
    }
    
    if (_state.debug_enabled) {
        DEBUG_PRINTF("Sending Report ID %d, Length %d\n", report_id, len);
    }
    
    return HID().SendReport(report_id, data, len);
}

// 초기화
bool KoreanUSBHID::begin(void) {
    DEBUG_PRINTLN("KoreanUSBHID: Initializing...");
    
    // USB 초기화
    if (!USB.begin()) {
        DEBUG_PRINTLN("Error: USB initialization failed");
        return false;
    }
    
    // USB 장치 정보 설정
    USB.VID(USB_VENDOR_ID);
    USB.PID(USB_PRODUCT_ID);
    USB.productName("Korean USB Keyboard");
    USB.manufacturerName("Samsung Electronics");
    USB.serialNumber("KR2024KB001");
    
    // USBHID 초기화
    if (!USBHID::begin()) {
        DEBUG_PRINTLN("Error: USBHID initialization failed");
        return false;
    }
    
    // Custom HID Report Descriptor 설정
    if (!setCustomHIDReportDescriptor()) {
        DEBUG_PRINTLN("Error: Custom HID Report Descriptor failed");
        return false;
    }
    
    // 상태 업데이트
    _state.is_initialized = true;
    _state.current_mode = LANG_MODE_ENGLISH;
    
    DEBUG_PRINTLN("KoreanUSBHID: Initialization complete");
    DEBUG_PRINTF("VID: 0x%04X, PID: 0x%04X\n", USB_VENDOR_ID, USB_PRODUCT_ID);
    DEBUG_PRINTF("Current Mode: %s\n", (_state.current_mode == LANG_MODE_KOREAN) ? "Korean" : "English");
    
    return true;
}

// 종료
void KoreanUSBHID::end(void) {
    DEBUG_PRINTLN("KoreanUSBHID: Ending...");
    
    releaseAll();
    USBHID::end();
    USB.end();
    
    _state.is_initialized = false;
    DEBUG_PRINTLN("KoreanUSBHID: Ended");
}

// Custom HID Report Descriptor 설정
bool KoreanUSBHID::setCustomHIDReportDescriptor(void) {
    DEBUG_PRINTLN("Setting Custom HID Report Descriptor...");
    
    // HID Report Descriptor 등록
    static HIDSubDescriptor node(korean_hid_report_desc, KOREAN_HID_DESC_SIZE);
    
    if (!HID().AppendDescriptor(&node)) {
        DEBUG_PRINTLN("Error: Failed to append HID descriptor");
        return false;
    }
    
    DEBUG_PRINTF("HID Report Descriptor Size: %d bytes\n", KOREAN_HID_DESC_SIZE);
    DEBUG_PRINTLN("Custom HID Report Descriptor set successfully");
    
    return true;
}

// 한/영 전환 방식 설정
void KoreanUSBHID::setToggleMethod(hangul_toggle_method_t method) {
    _state.toggle_method = method;
    
    if (_state.debug_enabled) {
        DEBUG_PRINTF("Toggle method set to: %d\n", method);
    }
}

// 현재 한/영 전환 방식 조회
hangul_toggle_method_t KoreanUSBHID::getToggleMethod(void) const {
    return _state.toggle_method;
}

// 한/영 전환 실행
bool KoreanUSBHID::toggleLanguage(void) {
    return toggleLanguageWithMethod(_state.toggle_method);
}

// 특정 방식으로 한/영 전환 실행
bool KoreanUSBHID::toggleLanguageWithMethod(hangul_toggle_method_t method) {
    if (!_state.is_initialized) {
        DEBUG_PRINTLN("Error: HID not initialized");
        return false;
    }
    
    if (_state.debug_enabled) {
        DEBUG_PRINTF("Toggling language with method %d\n", method);
    }
    
    bool success = false;
    
    switch (method) {
        case HANGUL_TOGGLE_RIGHT_ALT:
            success = sendKeyCombo(0, 0); // Right Alt는 별도 처리
            _keyboard_report.modifiers = 0x40; // Right Alt modifier
            success = _send_report(HID_REPORT_ID_KEYBOARD, &_keyboard_report, sizeof(_keyboard_report));
            delay(50);
            _keyboard_report.modifiers = 0;
            success &= _send_report(HID_REPORT_ID_KEYBOARD, &_keyboard_report, sizeof(_keyboard_report));
            break;
            
        case HANGUL_TOGGLE_ALT_SHIFT:
            success = sendKeyCombo(0x06, 0); // Alt + Shift
            break;
            
        case HANGUL_TOGGLE_CTRL_SPACE:
            success = sendKeyCombo(0x01, 0x2C); // Ctrl + Space
            break;
            
        case HANGUL_TOGGLE_SHIFT_SPACE:
            success = sendKeyCombo(0x02, 0x2C); // Shift + Space
            break;
            
        case HANGUL_TOGGLE_HANGUL_KEY:
            success = sendKey(0xF2); // 한/영 키 직접
            break;
            
        case HANGUL_TOGGLE_LEFT_ALT:
            success = sendKeyCombo(0x04, 0); // Left Alt
            break;
            
        case HANGUL_TOGGLE_WIN_SPACE:
            success = sendKeyCombo(0x08, 0x2C); // Win + Space
            break;
            
        case HANGUL_TOGGLE_LANG1_KEY:
            success = sendKey(HID_KEY_LANG1); // Language 1 (0x90)
            break;
            
        case HANGUL_TOGGLE_LANG2_KEY:
            success = sendKey(HID_KEY_LANG2); // Language 2 (0x91)
            break;
            
        case HANGUL_TOGGLE_F9_KEY:
            success = sendKey(0x42); // F9
            break;
            
        case HANGUL_TOGGLE_MENU_KEY:
            success = sendKey(0x76); // Menu key
            break;
            
        case HANGUL_TOGGLE_APPLICATION:
            success = sendKey(0x65); // Application key
            break;
            
        default:
            DEBUG_PRINTF("Error: Unknown toggle method %d\n", method);
            return false;
    }
    
    if (success) {
        // 상태 업데이트
        _state.current_mode = (_state.current_mode == LANG_MODE_KOREAN) ? LANG_MODE_ENGLISH : LANG_MODE_KOREAN;
        _update_toggle_stats();
        
        if (_state.debug_enabled) {
            DEBUG_PRINTF("Language toggled to: %s\n", (_state.current_mode == LANG_MODE_KOREAN) ? "Korean" : "English");
        }
    }
    
    return success;
}

// 통계 업데이트
void KoreanUSBHID::_update_toggle_stats(void) {
    _state.last_toggle_time = millis();
    _state.toggle_count++;
}

// 한국어 모드로 전환
bool KoreanUSBHID::switchToKorean(void) {
    if (_state.current_mode == LANG_MODE_KOREAN) {
        return true; // 이미 한국어 모드
    }
    
    return toggleLanguage();
}

// 영어 모드로 전환
bool KoreanUSBHID::switchToEnglish(void) {
    if (_state.current_mode == LANG_MODE_ENGLISH) {
        return true; // 이미 영어 모드
    }
    
    return toggleLanguage();
}

// 현재 언어 모드 조회
language_mode_t KoreanUSBHID::getCurrentMode(void) const {
    return _state.current_mode;
}

// 표준 키 전송
bool KoreanUSBHID::sendKey(uint8_t keycode, uint8_t modifiers) {
    if (!_state.is_initialized) {
        return false;
    }
    
    _keyboard_report.modifiers = modifiers;
    _keyboard_report.keys[0] = keycode;
    
    bool success = _send_report(HID_REPORT_ID_KEYBOARD, &_keyboard_report, sizeof(_keyboard_report));
    
    delay(50);
    
    // 키 릴리즈
    _keyboard_report.modifiers = 0;
    _keyboard_report.keys[0] = 0;
    success &= _send_report(HID_REPORT_ID_KEYBOARD, &_keyboard_report, sizeof(_keyboard_report));
    
    return success;
}

// 키 조합 전송
bool KoreanUSBHID::sendKeyCombo(uint8_t modifier, uint8_t keycode) {
    if (!_state.is_initialized) {
        return false;
    }
    
    _keyboard_report.modifiers = modifier;
    _keyboard_report.keys[0] = keycode;
    
    bool success = _send_report(HID_REPORT_ID_KEYBOARD, &_keyboard_report, sizeof(_keyboard_report));
    
    delay(50);
    
    // 키 릴리즈
    _keyboard_report.modifiers = 0;
    _keyboard_report.keys[0] = 0;
    success &= _send_report(HID_REPORT_ID_KEYBOARD, &_keyboard_report, sizeof(_keyboard_report));
    
    return success;
}

// Consumer Control 키 전송
bool KoreanUSBHID::sendConsumerKey(uint16_t usage_code) {
    if (!_state.is_initialized) {
        return false;
    }
    
    _consumer_report.usage_code = usage_code;
    
    bool success = _send_report(HID_REPORT_ID_CONSUMER, &_consumer_report, sizeof(_consumer_report));
    
    delay(50);
    
    // 키 릴리즈
    _consumer_report.usage_code = 0;
    success &= _send_report(HID_REPORT_ID_CONSUMER, &_consumer_report, sizeof(_consumer_report));
    
    return success;
}

// 한/영 키 직접 전송
bool KoreanUSBHID::sendHangulKey(void) {
    // 방법 1: 표준 키보드 Report
    bool success1 = sendKey(HID_KEY_HANGUL);
    
    // 방법 2: Consumer Control
    bool success2 = sendConsumerKey(CONSUMER_HANGUL_TOGGLE);
    
    return success1 || success2;
}

// 한자 키 전송
bool KoreanUSBHID::sendHanjaKey(void) {
    // 방법 1: 표준 키보드 Report
    bool success1 = sendKey(HID_KEY_HANJA);
    
    // 방법 2: Consumer Control
    bool success2 = sendConsumerKey(CONSUMER_HANJA_TOGGLE);
    
    return success1 || success2;
}

// 모든 키 릴리즈
bool KoreanUSBHID::releaseAll(void) {
    if (!_state.is_initialized) {
        return false;
    }
    
    _reset_reports();
    
    bool success = true;
    success &= _send_report(HID_REPORT_ID_KEYBOARD, &_keyboard_report, sizeof(_keyboard_report));
    success &= _send_report(HID_REPORT_ID_CONSUMER, &_consumer_report, sizeof(_consumer_report));
    success &= _send_report(HID_REPORT_ID_SYSTEM, &_system_report, sizeof(_system_report));
    
    return success;
}

// 키보드 상태 출력
void KoreanUSBHID::printStatus(void) {
    DEBUG_PRINTLN("=== Korean USB HID Status ===");
    DEBUG_PRINTF("Initialized: %s\n", _state.is_initialized ? "Yes" : "No");
    DEBUG_PRINTF("Current Mode: %s\n", (_state.current_mode == LANG_MODE_KOREAN) ? "Korean" : "English");
    DEBUG_PRINTF("Toggle Method: %d\n", _state.toggle_method);
    DEBUG_PRINTF("Debug Enabled: %s\n", _state.debug_enabled ? "Yes" : "No");
    DEBUG_PRINTF("Toggle Count: %d\n", _state.toggle_count);
    
    if (_state.last_toggle_time > 0) {
        DEBUG_PRINTF("Last Toggle: %d ms ago\n", millis() - _state.last_toggle_time);
    }
    
    DEBUG_PRINTLN("============================");
}

// 디버그 모드 설정
void KoreanUSBHID::setDebugMode(bool enabled) {
    _state.debug_enabled = enabled;
    DEBUG_PRINTF("Debug mode: %s\n", enabled ? "Enabled" : "Disabled");
}

// 통계 정보 출력
void KoreanUSBHID::printStats(void) {
    DEBUG_PRINTLN("=== Korean USB HID Statistics ===");
    DEBUG_PRINTF("Total Toggles: %d\n", _state.toggle_count);
    
    if (_state.toggle_count > 0) {
        DEBUG_PRINTF("Average Toggle Interval: %d ms\n", 
                    (_state.last_toggle_time > 0) ? (_state.last_toggle_time / _state.toggle_count) : 0);
    }
    
    DEBUG_PRINTLN("=================================");
}

// 지원되는 한/영 전환 방식 목록 출력
void KoreanUSBHID::printSupportedMethods(void) {
    DEBUG_PRINTLN("=== Supported Toggle Methods ===");
    DEBUG_PRINTLN("1. Right Alt");
    DEBUG_PRINTLN("2. Alt + Shift");
    DEBUG_PRINTLN("3. Ctrl + Space");
    DEBUG_PRINTLN("4. Shift + Space");
    DEBUG_PRINTLN("5. Hangul Key (0xF2)");
    DEBUG_PRINTLN("6. Left Alt");
    DEBUG_PRINTLN("7. Win + Space");
    DEBUG_PRINTLN("8. Language 1 (0x90)");
    DEBUG_PRINTLN("9. Language 2 (0x91)");
    DEBUG_PRINTLN("10. F9 Key");
    DEBUG_PRINTLN("11. Menu Key");
    DEBUG_PRINTLN("12. Application Key");
    DEBUG_PRINTLN("===============================");
}

// 12가지 방식 순차 테스트
int KoreanUSBHID::testAllToggleMethods(void) {
    DEBUG_PRINTLN("=== Testing All Toggle Methods ===");
    
    for (int method = 1; method <= 12; method++) {
        DEBUG_PRINTF("Testing method %d...\n", method);
        
        bool success = toggleLanguageWithMethod((hangul_toggle_method_t)method);
        
        if (success) {
            DEBUG_PRINTF("Method %d: SUCCESS\n", method);
            delay(1000); // 테스트 간 간격
        } else {
            DEBUG_PRINTF("Method %d: FAILED\n", method);
        }
    }
    
    DEBUG_PRINTLN("=== Test Complete ===");
    return 0; // 추후 성공한 방식 반환 로직 추가
}

// USB 연결 상태 확인
bool KoreanUSBHID::isConnected(void) const {
    return USB.isConnected();
}

// 초기화 상태 확인
bool KoreanUSBHID::isInitialized(void) const {
    return _state.is_initialized;
}