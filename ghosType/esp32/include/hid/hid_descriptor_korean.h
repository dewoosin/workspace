#ifndef HID_DESCRIPTOR_KOREAN_H
#define HID_DESCRIPTOR_KOREAN_H

#include <stdint.h>

/**
 * @file hid_descriptor_korean.h
 * @brief 한국어 키보드 전용 HID Report Descriptor
 * 
 * USB HID 1.11 스펙에 따른 한국어 키보드 구현
 * - Report ID 1: 표준 키보드 (8바이트)
 * - Report ID 2: Consumer Control (한/영, 한자)
 * - Report ID 3: System Control (전원 관리)
 */

// 한국어 키보드 HID Report Descriptor
static const uint8_t korean_hid_report_desc[] = {
    //=== Report ID 1: 표준 키보드 ===
    0x05, 0x01,        // Usage Page (Generic Desktop) 
    0x09, 0x06,        // Usage (Keyboard)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        // Report ID (1)
    
    // Modifier Keys (8비트)
    0x05, 0x07,        // Usage Page (Keyboard/Keypad)
    0x19, 0xE0,        // Usage Minimum (Left Control)
    0x29, 0xE7,        // Usage Maximum (Right GUI)
    0x15, 0x00,        // Logical Minimum (0)
    0x25, 0x01,        // Logical Maximum (1)
    0x75, 0x01,        // Report Size (1)
    0x95, 0x08,        // Report Count (8)
    0x81, 0x02,        // Input (Data,Var,Abs)
    
    // Reserved byte
    0x75, 0x08,        // Report Size (8)
    0x95, 0x01,        // Report Count (1)
    0x81, 0x01,        // Input (Const,Array,Abs)
    
    // Key Array (6개 동시 입력)
    0x05, 0x07,        // Usage Page (Keyboard/Keypad)
    0x19, 0x00,        // Usage Minimum (0)
    0x2A, 0xFF, 0x00,  // Usage Maximum (255)
    0x15, 0x00,        // Logical Minimum (0)
    0x26, 0xFF, 0x00,  // Logical Maximum (255)
    0x75, 0x08,        // Report Size (8)
    0x95, 0x06,        // Report Count (6)
    0x81, 0x00,        // Input (Data,Array,Abs)
    
    // LED Output (5비트)
    0x05, 0x08,        // Usage Page (LEDs)
    0x19, 0x01,        // Usage Minimum (Num Lock)
    0x29, 0x05,        // Usage Maximum (Kana)
    0x75, 0x01,        // Report Size (1)
    0x95, 0x05,        // Report Count (5)
    0x91, 0x02,        // Output (Data,Var,Abs)
    
    // LED Padding (3비트)
    0x75, 0x03,        // Report Size (3)
    0x95, 0x01,        // Report Count (1)
    0x91, 0x01,        // Output (Const,Array,Abs)
    
    0xC0,              // End Collection
    
    //=== Report ID 2: Consumer Control ===
    0x05, 0x0C,        // Usage Page (Consumer Devices)
    0x09, 0x01,        // Usage (Consumer Control)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x02,        // Report ID (2)
    
    // 한/영, 한자 키 포함
    0x15, 0x00,        // Logical Minimum (0)
    0x26, 0xFF, 0x03,  // Logical Maximum (1023)
    0x19, 0x00,        // Usage Minimum (0)
    0x2A, 0xFF, 0x03,  // Usage Maximum (1023)
    0x75, 0x10,        // Report Size (16)
    0x95, 0x01,        // Report Count (1)
    0x81, 0x00,        // Input (Data,Array,Abs)
    
    0xC0,              // End Collection
    
    //=== Report ID 3: System Control ===
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x80,        // Usage (System Control)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x03,        // Report ID (3)
    
    // 시스템 제어 (전원, 절전 등)
    0x15, 0x00,        // Logical Minimum (0)
    0x25, 0x01,        // Logical Maximum (1)
    0x19, 0x81,        // Usage Minimum (System Power Down)
    0x29, 0x83,        // Usage Maximum (System Wake Up)
    0x75, 0x01,        // Report Size (1)
    0x95, 0x03,        // Report Count (3)
    0x81, 0x06,        // Input (Data,Var,Rel)
    
    // Padding
    0x75, 0x05,        // Report Size (5)
    0x95, 0x01,        // Report Count (1)
    0x81, 0x01,        // Input (Const,Array,Abs)
    
    0xC0               // End Collection
};

// HID Descriptor 크기
#define KOREAN_HID_DESC_SIZE sizeof(korean_hid_report_desc)

// HID Report 구조체 정의
typedef struct {
    uint8_t modifiers;     // Modifier keys
    uint8_t reserved;      // Reserved byte
    uint8_t keys[6];       // Key array
} __attribute__((packed)) hid_keyboard_report_t;

typedef struct {
    uint16_t usage_code;   // Consumer usage code
} __attribute__((packed)) hid_consumer_report_t;

typedef struct {
    uint8_t system_keys;   // System control keys
} __attribute__((packed)) hid_system_report_t;

// 한국어 키보드 전용 Usage Code
#define CONSUMER_HANGUL_TOGGLE   0x0090   // 한/영 전환
#define CONSUMER_HANJA_TOGGLE    0x0091   // 한자 전환
#define CONSUMER_LANG_SWITCH     0x01F1   // 언어 전환
#define CONSUMER_LANG_TOGGLE     0x01F2   // 언어 토글

// 한국어 키보드 키코드
#define HID_KEY_HANGUL          0x90      // 한/영 키
#define HID_KEY_HANJA           0x91      // 한자 키
#define HID_KEY_LANG1           0x90      // Language 1
#define HID_KEY_LANG2           0x91      // Language 2

// Report ID 정의
#define HID_REPORT_ID_KEYBOARD   1
#define HID_REPORT_ID_CONSUMER   2
#define HID_REPORT_ID_SYSTEM     3

#endif // HID_DESCRIPTOR_KOREAN_H