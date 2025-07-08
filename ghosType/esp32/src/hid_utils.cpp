/**
 * @file hid_utils.cpp
 * @brief USB HID 키보드 제어 유틸리티 구현
 * @version 1.0
 * @date 2024-12-28
 */

#include "hid_utils.h"
#include <esp_random.h>

// 정적 멤버 변수 초기화
USBHIDKeyboard HIDUtils::keyboard;
bool HIDUtils::initialized = false;
bool HIDUtils::hid_ready = false;

bool HIDUtils::initialize() {
    // 이미 초기화된 경우 성공 반환
    if (initialized && hid_ready) {
        return true;
    }

    // USB 서브시스템 초기화
    USB.begin();
    safeDelay(1000);  // USB 열거 대기

    // HID 키보드 인터페이스 초기화
    keyboard.begin();
    safeDelay(500);   // HID 등록 대기

    // 초기화 상태 업데이트
    initialized = true;
    hid_ready = true;

    return true;
}

void HIDUtils::deinitialize() {
    if (!initialized) {
        return;
    }

    // 모든 키 해제
    releaseAllKeys();
    safeDelay(100);

    // HID 인터페이스 종료
    keyboard.end();
    
    // USB 서브시스템 종료
    // ESP32-S3에서는 USB.end()가 지원되지 않음
    // USB는 시스템 레벨에서 관리되므로 별도 종료 불필요
    
    // 상태 리셋
    initialized = false;
    hid_ready = false;
}

bool HIDUtils::typeCharacter(char character, uint16_t hold_duration_ms) {
    // HID 준비 상태 확인
    if (!hid_ready) {
        return false;
    }

    // 출력 가능한 문자인지 확인
    if (!isPrintableASCII(character)) {
        // 특수 문자 처리 시도
        return handleSpecialCharacter(character);
    }

    // 대문자 처리 (Shift 키 사용)
    if (isUpperCase(character)) {
        return typeWithShift(character, SHIFT_HOLD_DURATION_MS);
    }

    // 일반 문자 타이핑
    keyboard.write(character);
    safeDelay(hold_duration_ms);

    return true;
}

bool HIDUtils::typeSpecialKey(uint8_t key_code, uint16_t hold_duration_ms) {
    if (!hid_ready) {
        return false;
    }

    // 특수 키 입력
    keyboard.press(key_code);
    safeDelay(hold_duration_ms);
    keyboard.releaseAll();
    safeDelay(KEY_RELEASE_DURATION_MS);

    return true;
}

size_t HIDUtils::typeString(const char* text, 
                          uint8_t chars_per_second,
                          uint8_t interval_chars,
                          uint16_t interval_ms) {
    if (!hid_ready || text == nullptr) {
        return 0;
    }

    // 타이핑 속도 검증 및 제한
    chars_per_second = CLAMP(chars_per_second, MIN_TYPING_SPEED_CPS, MAX_TYPING_SPEED_CPS);
    
    size_t chars_typed = 0;
    size_t text_length = strlen(text);
    
    // 텍스트 길이 제한 (안전성)
    if (text_length > MAX_TEXT_CHUNK_SIZE) {
        text_length = MAX_TEXT_CHUNK_SIZE;
    }

    // 각 문자 순차 타이핑
    for (size_t i = 0; i < text_length; i++) {
        char current_char = text[i];
        
        // 현재 문자 타이핑
        if (typeCharacter(current_char)) {
            chars_typed++;
        }

        // 타이핑 속도에 따른 지연
        uint32_t delay_ms = calculateTypingDelay(chars_per_second, true);
        safeDelay(delay_ms);

        // 간격 지연 적용 (자연스러운 타이핑을 위해)
        if (interval_chars > 0 && (chars_typed % interval_chars) == 0) {
            safeDelay(interval_ms);
        }

        // 와치독 타임아웃 방지를 위한 주기적 yield
        if ((i % 50) == 0) {
            yield();
        }
    }

    return chars_typed;
}

bool HIDUtils::typeWithShift(char character, uint16_t shift_hold_ms) {
    if (!hid_ready) {
        return false;
    }

    // Shift 키 누르기
    keyboard.press(KEY_LEFT_SHIFT);
    safeDelay(shift_hold_ms);

    // 문자 입력
    keyboard.press(character);
    safeDelay(KEY_PRESS_DURATION_MS);

    // 모든 키 해제
    keyboard.releaseAll();
    safeDelay(KEY_RELEASE_DURATION_MS);

    return true;
}

void HIDUtils::releaseAllKeys() {
    if (hid_ready) {
        keyboard.releaseAll();
        safeDelay(10);  // 키 해제 안정화 시간
    }
}

bool HIDUtils::isConnected() {
    return initialized && hid_ready;
}

uint32_t HIDUtils::calculateTypingDelay(uint8_t chars_per_second, bool add_variance) {
    // 기본 지연 시간 계산 (1초 / CPS)
    uint32_t base_delay = 1000 / chars_per_second;
    
    // 최소/최대 지연 시간 제한
    base_delay = CLAMP(base_delay, 20, 2000);

    // 변동성 추가 (자연스러운 타이핑을 위해)
    if (add_variance) {
        // ±20% 범위의 랜덤 변동
        uint32_t variance_range = base_delay / 5;  // 20%
        int32_t variance = (esp_random() % (variance_range * 2)) - variance_range;
        
        base_delay = MAX(10, (int32_t)base_delay + variance);
    }

    return base_delay;
}

void HIDUtils::safeDelay(uint32_t delay_ms) {
    // 긴 지연의 경우 yield를 주기적으로 호출
    if (delay_ms > 100) {
        uint32_t remaining = delay_ms;
        while (remaining > 0) {
            uint32_t chunk = MIN(remaining, 50);
            delay(chunk);
            yield();  // 와치독 타임아웃 방지
            remaining -= chunk;
        }
    } else {
        delay(delay_ms);
    }
}

bool HIDUtils::isPrintableASCII(char character) {
    return (character >= ASCII_PRINTABLE_START && character <= ASCII_PRINTABLE_END);
}

bool HIDUtils::isUpperCase(char character) {
    return (character >= 'A' && character <= 'Z');
}

bool HIDUtils::handleSpecialCharacter(char character) {
    if (!hid_ready) {
        return false;
    }

    switch (character) {
        case CHAR_NEWLINE:
            return typeSpecialKey(KEY_RETURN);
            
        case CHAR_TAB:
            return typeSpecialKey(KEY_TAB);
            
        case CHAR_CARRIAGE_RETURN:
            return typeSpecialKey(KEY_RETURN);
            
        default:
            // 처리할 수 없는 문자는 무시
            return false;
    }
}