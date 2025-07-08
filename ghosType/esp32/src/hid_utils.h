/**
 * @file hid_utils.h
 * @brief USB HID 키보드 제어 유틸리티
 * @version 1.0
 * @date 2024-12-28
 * 
 * 이 모듈은 ESP32-S3의 USB HID 기능을 사용하여 키보드 입력을 시뮬레이션합니다.
 * 다양한 키 입력 패턴과 타이밍 제어를 제공합니다.
 */

#pragma once

#include <Arduino.h>
#include <USB.h>
#include <USBHIDKeyboard.h>
#include "config.h"

/**
 * @brief HID 키보드 관리 클래스
 * 
 * ESP32-S3의 USB HID 기능을 추상화하여 안전하고 효율적인
 * 키보드 입력을 제공합니다.
 */
class HIDUtils {
public:
    /**
     * @brief HID 시스템 초기화
     * @return true 초기화 성공, false 실패
     * 
     * USB HID 인터페이스를 초기화하고 시스템이 키보드로
     * 인식되도록 설정합니다.
     */
    static bool initialize();

    /**
     * @brief HID 시스템 종료
     * 
     * USB HID 인터페이스를 안전하게 종료하고
     * 리소스를 정리합니다.
     */
    static void deinitialize();

    /**
     * @brief 단일 문자 타이핑
     * @param character 타이핑할 문자
     * @param hold_duration_ms 키를 누르고 있을 시간 (밀리초)
     * @return true 성공, false 실패
     * 
     * 지정된 문자를 안전하게 타이핑합니다.
     * 대문자의 경우 자동으로 Shift 키를 처리합니다.
     */
    static bool typeCharacter(char character, uint16_t hold_duration_ms = KEY_PRESS_DURATION_MS);

    /**
     * @brief 특수 키 입력
     * @param key_code 특수 키 코드 (예: KEY_ENTER, KEY_TAB)
     * @param hold_duration_ms 키를 누르고 있을 시간 (밀리초)
     * @return true 성공, false 실패
     * 
     * Enter, Tab, Backspace 등의 특수 키를 입력합니다.
     */
    static bool typeSpecialKey(uint8_t key_code, uint16_t hold_duration_ms = KEY_PRESS_DURATION_MS);

    /**
     * @brief 문자열 타이핑
     * @param text 타이핑할 문자열
     * @param chars_per_second 초당 문자 수 (타이핑 속도)
     * @param interval_chars 간격 지연을 적용할 문자 수
     * @param interval_ms 간격 지연 시간 (밀리초)
     * @return 실제로 타이핑된 문자 수
     * 
     * 주어진 문자열을 지정된 속도로 타이핑합니다.
     * 자연스러운 타이핑을 위한 변동성도 포함합니다.
     */
    static size_t typeString(const char* text, 
                           uint8_t chars_per_second = DEFAULT_TYPING_SPEED_CPS,
                           uint8_t interval_chars = DEFAULT_INTERVAL_CHARS,
                           uint16_t interval_ms = DEFAULT_INTERVAL_MS);

    /**
     * @brief Shift + 문자 조합 입력
     * @param character 입력할 문자
     * @param shift_hold_ms Shift 키를 누르고 있을 시간
     * @return true 성공, false 실패
     * 
     * Shift 키와 함께 문자를 입력합니다.
     * 대문자나 특수 기호 입력에 사용됩니다.
     */
    static bool typeWithShift(char character, uint16_t shift_hold_ms = SHIFT_HOLD_DURATION_MS);

    /**
     * @brief 모든 키 해제
     * 
     * 현재 눌려진 모든 키를 안전하게 해제합니다.
     * 키가 눌린 상태로 남아있는 것을 방지합니다.
     */
    static void releaseAllKeys();

    /**
     * @brief HID 연결 상태 확인
     * @return true 연결됨, false 연결 안됨
     * 
     * USB HID 인터페이스가 호스트에 제대로 연결되어
     * 키 입력이 가능한 상태인지 확인합니다.
     */
    static bool isConnected();

    /**
     * @brief 타이핑 지연 시간 계산
     * @param chars_per_second 초당 문자 수
     * @param add_variance 변동성 추가 여부
     * @return 계산된 지연 시간 (밀리초)
     * 
     * 지정된 타이핑 속도에 맞는 지연 시간을 계산합니다.
     * 자연스러운 타이핑을 위한 랜덤 변동성을 추가할 수 있습니다.
     */
    static uint32_t calculateTypingDelay(uint8_t chars_per_second, bool add_variance = true);

    /**
     * @brief 안전한 지연 함수
     * @param delay_ms 지연 시간 (밀리초)
     * 
     * 시스템 안정성을 위한 안전한 지연을 수행합니다.
     * 와치독 타임아웃을 방지하기 위한 yield 호출을 포함합니다.
     */
    static void safeDelay(uint32_t delay_ms);

private:
    static USBHIDKeyboard keyboard;  ///< USB HID 키보드 인스턴스
    static bool initialized;         ///< 초기화 상태
    static bool hid_ready;          ///< HID 준비 상태

    /**
     * @brief 문자가 출력 가능한 ASCII인지 확인
     * @param character 확인할 문자
     * @return true 출력 가능, false 불가능
     */
    static bool isPrintableASCII(char character);

    /**
     * @brief 문자가 대문자인지 확인
     * @param character 확인할 문자
     * @return true 대문자, false 아님
     */
    static bool isUpperCase(char character);

    /**
     * @brief 특수 문자 처리
     * @param character 처리할 특수 문자
     * @return true 처리됨, false 일반 문자
     */
    static bool handleSpecialCharacter(char character);
};