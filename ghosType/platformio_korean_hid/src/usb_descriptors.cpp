#include "usb_descriptors.h"

/**
 * @file usb_descriptors.cpp
 * @brief 한국어 키보드 클래스 구현 (간소화)
 */

// 생성자
KoreanUSBHID::KoreanUSBHID() {
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
    _state.debug_enabled = DEBUG_ENABLED;
}

// Report 초기화
void KoreanUSBHID::_reset_reports(void) {
    memset(&_keyboard_report, 0, sizeof(_keyboard_report));
    memset(&_consumer_report, 0, sizeof(_consumer_report));
}

// Report 전송
bool KoreanUSBHID::_send_report(uint8_t report_id, const void* data, uint16_t len) {
    if (!_state.is_initialized) {
        return false;
    }
    
    return _usb_hid.sendReport(report_id, data, len);
}

// 초기화
bool KoreanUSBHID::begin(void) {
    // USB 설정
    TinyUSBDevice.setManufacturerDescriptor("Samsung Electronics");
    TinyUSBDevice.setProductDescriptor("Korean USB Keyboard");
    TinyUSBDevice.setSerialDescriptor("KR2024KB001");
    TinyUSBDevice.setID(USB_VENDOR_ID, USB_PRODUCT_ID);
    
    // HID Descriptor 설정
    _usb_hid.setPollInterval(2);
    _usb_hid.setReportDescriptor(korean_hid_report_desc, KOREAN_HID_DESC_SIZE);
    _usb_hid.setStringDescriptor("Korean USB HID Keyboard");
    
    // HID 초기화
    if (!_usb_hid.begin()) {
        return false;
    }
    
    // USB 시작
    if (!TinyUSBDevice.begin(0)) {
        return false;
    }
    
    _state.is_initialized = true;
    return true;
}

// 종료
void KoreanUSBHID::end(void) {
    releaseAll();
    _state.is_initialized = false;
}

// 한영 전환
bool KoreanUSBHID::toggleLanguage(void) {
    if (!_state.is_initialized) {
        return false;
    }
    
    bool success = false;
    
    // Right Alt 방식으로 한영 전환
    _keyboard_report.modifiers = 0x40; // Right Alt
    success = _send_report(HID_REPORT_ID_KEYBOARD, &_keyboard_report, sizeof(_keyboard_report));
    delay(50);
    
    _keyboard_report.modifiers = 0;
    success &= _send_report(HID_REPORT_ID_KEYBOARD, &_keyboard_report, sizeof(_keyboard_report));
    
    if (success) {
        _state.current_mode = (_state.current_mode == LANG_MODE_KOREAN) ? LANG_MODE_ENGLISH : LANG_MODE_KOREAN;
        _state.last_toggle_time = millis();
        _state.toggle_count++;
    }
    
    return success;
}

// 한국어 모드로 전환
bool KoreanUSBHID::switchToKorean(void) {
    if (_state.current_mode == LANG_MODE_KOREAN) {
        return true;
    }
    return toggleLanguage();
}

// 영어 모드로 전환
bool KoreanUSBHID::switchToEnglish(void) {
    if (_state.current_mode == LANG_MODE_ENGLISH) {
        return true;
    }
    return toggleLanguage();
}

// 현재 모드 반환
language_mode_t KoreanUSBHID::getCurrentMode(void) const {
    return _state.current_mode;
}

// 키 전송
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

// Consumer 키 전송
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

// 모든 키 릴리즈
bool KoreanUSBHID::releaseAll(void) {
    if (!_state.is_initialized) {
        return false;
    }
    
    _reset_reports();
    return _send_report(HID_REPORT_ID_KEYBOARD, &_keyboard_report, sizeof(_keyboard_report));
}

// 상태 출력
void KoreanUSBHID::printStatus(void) {
    Serial.println("=== Korean USB HID Status ===");
    Serial.printf("Initialized: %s\n", _state.is_initialized ? "Yes" : "No");
    Serial.printf("Current Mode: %s\n", (_state.current_mode == LANG_MODE_KOREAN) ? "Korean" : "English");
    Serial.printf("Toggle Method: %d\n", _state.toggle_method);
    Serial.printf("Toggle Count: %d\n", _state.toggle_count);
    Serial.println("=============================");
}

// 통계 출력
void KoreanUSBHID::printStats(void) {
    Serial.println("=== Statistics ===");
    Serial.printf("Total Toggles: %d\n", _state.toggle_count);
    if (_state.last_toggle_time > 0) {
        Serial.printf("Last Toggle: %d ms ago\n", millis() - _state.last_toggle_time);
    }
    Serial.println("=================");
}

// 연결 상태 확인
bool KoreanUSBHID::isConnected(void) const {
    return TinyUSBDevice.mounted();
}

// 초기화 상태 확인
bool KoreanUSBHID::isInitialized(void) const {
    return _state.is_initialized;
}