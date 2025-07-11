#ifndef CONFIG_H
#define CONFIG_H

/**
 * @file config.h
 * @brief 한국어 키보드 설정 파일
 */

// USB 장치 정보
#define USB_VENDOR_ID           0x04E8    // Samsung Electronics
#define USB_PRODUCT_ID          0x7021    // Korean USB Keyboard

// Language ID
#define LANG_ID_KOREAN          0x0412    // 한국어
#define LANG_ID_ENGLISH         0x0409    // 영어

// HID Country Code
#define HID_COUNTRY_KOREAN      16        // 한국

// 한영 전환 방식
typedef enum {
    HANGUL_TOGGLE_RIGHT_ALT = 1,
    HANGUL_TOGGLE_ALT_SHIFT = 2,
    HANGUL_TOGGLE_CTRL_SPACE = 3,
    HANGUL_TOGGLE_SHIFT_SPACE = 4,
    HANGUL_TOGGLE_HANGUL_KEY = 5,
    HANGUL_TOGGLE_LEFT_ALT = 6,
    HANGUL_TOGGLE_WIN_SPACE = 7,
    HANGUL_TOGGLE_LANG1_KEY = 8,
    HANGUL_TOGGLE_LANG2_KEY = 9,
    HANGUL_TOGGLE_F9_KEY = 10,
    HANGUL_TOGGLE_MENU_KEY = 11,
    HANGUL_TOGGLE_APPLICATION = 12
} hangul_toggle_method_t;

// 언어 모드
typedef enum {
    LANG_MODE_ENGLISH = 0,
    LANG_MODE_KOREAN = 1
} language_mode_t;

// Consumer Control Usage Code
#define CONSUMER_HANGUL_TOGGLE   0x0090
#define CONSUMER_HANJA_TOGGLE    0x0091

// HID 키코드
#define HID_KEY_HANGUL          0x90
#define HID_KEY_HANJA           0x91
#define HID_KEY_LANG1           0x90
#define HID_KEY_LANG2           0x91

// Report ID
#define HID_REPORT_ID_KEYBOARD   1
#define HID_REPORT_ID_CONSUMER   2
#define HID_REPORT_ID_SYSTEM     3

// 디버그 설정
#define DEBUG_ENABLED           true

#endif // CONFIG_H