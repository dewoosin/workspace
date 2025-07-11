#ifndef KOREAN_USB_HID_H
#define KOREAN_USB_HID_H

#include <Arduino.h>
#include <USB.h>
#include <USBHID.h>
#include "esp32-hal-tinyusb.h"
#include "tusb.h"

// 로컬 헤더 파일
#include "hid/hid_descriptor_korean.h"
#include "usb/usb_device_config.h"
#include "usb/usb_config_descriptor.h"

/**
 * @file korean_usb_hid.h
 * @brief 한국어 키보드 전용 USB HID 클래스
 * 
 * TinyUSB를 직접 제어하여 한국어 키보드로 완벽히 인식
 * Windows에서 Samsung Korean USB Keyboard로 표시
 */

// 한영 전환 방식 열거형
typedef enum {
    HANGUL_TOGGLE_RIGHT_ALT = 1,      // 오른쪽 Alt (가장 일반적)
    HANGUL_TOGGLE_ALT_SHIFT = 2,      // Alt + Shift
    HANGUL_TOGGLE_CTRL_SPACE = 3,     // Ctrl + Space (MS IME)
    HANGUL_TOGGLE_SHIFT_SPACE = 4,    // Shift + Space
    HANGUL_TOGGLE_HANGUL_KEY = 5,     // 한/영 키 (0xF2)
    HANGUL_TOGGLE_LEFT_ALT = 6,       // 왼쪽 Alt
    HANGUL_TOGGLE_WIN_SPACE = 7,      // Win + Space
    HANGUL_TOGGLE_LANG1_KEY = 8,      // HID Language 1 (0x90)
    HANGUL_TOGGLE_LANG2_KEY = 9,      // HID Language 2 (0x91)
    HANGUL_TOGGLE_F9_KEY = 10,        // F9 키
    HANGUL_TOGGLE_MENU_KEY = 11,      // Menu 키
    HANGUL_TOGGLE_APPLICATION = 12    // Application 키
} hangul_toggle_method_t;

// 한영 전환 상태
typedef enum {
    LANG_MODE_ENGLISH = 0,
    LANG_MODE_KOREAN = 1
} language_mode_t;

// 키보드 상태 구조체
typedef struct {
    bool is_initialized;
    language_mode_t current_mode;
    hangul_toggle_method_t toggle_method;
    uint32_t last_toggle_time;
    uint32_t toggle_count;
    bool debug_enabled;
} keyboard_state_t;

/**
 * @class KoreanUSBHID
 * @brief 한국어 키보드 전용 USB HID 클래스
 */
class KoreanUSBHID : public USBHID {
private:
    // 상태 변수
    keyboard_state_t _state;
    
    // HID Report 버퍼
    hid_keyboard_report_t _keyboard_report;
    hid_consumer_report_t _consumer_report;
    hid_system_report_t _system_report;
    
    // 내부 메소드
    void _init_state(void);
    void _reset_reports(void);
    void _update_toggle_stats(void);
    bool _send_report(uint8_t report_id, const void* data, uint16_t len);
    
public:
    /**
     * @brief 생성자
     */
    KoreanUSBHID();
    
    /**
     * @brief 초기화
     * @return 성공 여부
     */
    bool begin(void);
    
    /**
     * @brief 종료
     */
    void end(void);
    
    /**
     * @brief HID Report Descriptor 설정
     * @return 성공 여부
     */
    bool setCustomHIDReportDescriptor(void);
    
    /**
     * @brief 한/영 전환 방식 설정
     * @param method 전환 방식
     */
    void setToggleMethod(hangul_toggle_method_t method);
    
    /**
     * @brief 현재 한/영 전환 방식 조회
     * @return 현재 전환 방식
     */
    hangul_toggle_method_t getToggleMethod(void) const;
    
    /**
     * @brief 한/영 전환 실행
     * @return 성공 여부
     */
    bool toggleLanguage(void);
    
    /**
     * @brief 특정 방식으로 한/영 전환 실행
     * @param method 전환 방식
     * @return 성공 여부
     */
    bool toggleLanguageWithMethod(hangul_toggle_method_t method);
    
    /**
     * @brief 한국어 모드로 전환
     * @return 성공 여부
     */
    bool switchToKorean(void);
    
    /**
     * @brief 영어 모드로 전환
     * @return 성공 여부
     */
    bool switchToEnglish(void);
    
    /**
     * @brief 현재 언어 모드 조회
     * @return 현재 언어 모드
     */
    language_mode_t getCurrentMode(void) const;
    
    /**
     * @brief 표준 키 전송
     * @param keycode 키코드
     * @param modifiers 수정자 키
     * @return 성공 여부
     */
    bool sendKey(uint8_t keycode, uint8_t modifiers = 0);
    
    /**
     * @brief 키 조합 전송
     * @param modifier 수정자 키
     * @param keycode 키코드
     * @return 성공 여부
     */
    bool sendKeyCombo(uint8_t modifier, uint8_t keycode);
    
    /**
     * @brief Consumer Control 키 전송
     * @param usage_code Consumer usage code
     * @return 성공 여부
     */
    bool sendConsumerKey(uint16_t usage_code);
    
    /**
     * @brief 한/영 키 직접 전송
     * @return 성공 여부
     */
    bool sendHangulKey(void);
    
    /**
     * @brief 한자 키 전송
     * @return 성공 여부
     */
    bool sendHanjaKey(void);
    
    /**
     * @brief 모든 키 릴리즈
     * @return 성공 여부
     */
    bool releaseAll(void);
    
    /**
     * @brief 키보드 상태 출력
     */
    void printStatus(void);
    
    /**
     * @brief 디버그 모드 설정
     * @param enabled 활성화 여부
     */
    void setDebugMode(bool enabled);
    
    /**
     * @brief 통계 정보 출력
     */
    void printStats(void);
    
    /**
     * @brief 지원되는 한/영 전환 방식 목록 출력
     */
    void printSupportedMethods(void);
    
    /**
     * @brief 12가지 방식 순차 테스트
     * @return 성공한 방식 번호 (0이면 실패)
     */
    int testAllToggleMethods(void);
    
    /**
     * @brief USB 연결 상태 확인
     * @return 연결 여부
     */
    bool isConnected(void) const;
    
    /**
     * @brief 초기화 상태 확인
     * @return 초기화 여부
     */
    bool isInitialized(void) const;
};

// 전역 인스턴스
extern KoreanUSBHID KoreanKeyboard;

// 편의 함수들
inline bool hangul_toggle() { return KoreanKeyboard.toggleLanguage(); }
inline bool switch_to_korean() { return KoreanKeyboard.switchToKorean(); }
inline bool switch_to_english() { return KoreanKeyboard.switchToEnglish(); }
inline language_mode_t get_current_mode() { return KoreanKeyboard.getCurrentMode(); }

#endif // KOREAN_USB_HID_H