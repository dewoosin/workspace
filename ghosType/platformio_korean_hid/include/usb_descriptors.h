#ifndef USB_DESCRIPTORS_H
#define USB_DESCRIPTORS_H

#include <stdint.h>
#include <Arduino.h>
#include "Adafruit_TinyUSB.h"
#include "config.h"

/**
 * @file usb_descriptors.h
 * @brief USB Descriptor 정의
 */

// 한국어 키보드 HID Report Descriptor
static const uint8_t korean_hid_report_desc[] = {
    // 표준 키보드 Report ID 1
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x06,        // Usage (Keyboard)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        // Report ID (1)
    
    // Modifier Keys
    0x05, 0x07,        // Usage Page (Keyboard)
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
    0x81, 0x01,        // Input (Const)
    
    // Key Array
    0x05, 0x07,        // Usage Page (Keyboard)
    0x19, 0x00,        // Usage Minimum (0)
    0x2A, 0xFF, 0x00,  // Usage Maximum (255)
    0x15, 0x00,        // Logical Minimum (0)
    0x26, 0xFF, 0x00,  // Logical Maximum (255)
    0x75, 0x08,        // Report Size (8)
    0x95, 0x06,        // Report Count (6)
    0x81, 0x00,        // Input (Data,Array)
    
    // LED Output
    0x05, 0x08,        // Usage Page (LEDs)
    0x19, 0x01,        // Usage Minimum (Num Lock)
    0x29, 0x05,        // Usage Maximum (Kana)
    0x75, 0x01,        // Report Size (1)
    0x95, 0x05,        // Report Count (5)
    0x91, 0x02,        // Output (Data,Var,Abs)
    
    // LED Padding
    0x75, 0x03,        // Report Size (3)
    0x95, 0x01,        // Report Count (1)
    0x91, 0x01,        // Output (Const)
    
    0xC0,              // End Collection
    
    // Consumer Control Report ID 2
    0x05, 0x0C,        // Usage Page (Consumer)
    0x09, 0x01,        // Usage (Consumer Control)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x02,        // Report ID (2)
    
    0x15, 0x00,        // Logical Minimum (0)
    0x26, 0xFF, 0x03,  // Logical Maximum (1023)
    0x19, 0x00,        // Usage Minimum (0)
    0x2A, 0xFF, 0x03,  // Usage Maximum (1023)
    0x75, 0x10,        // Report Size (16)
    0x95, 0x01,        // Report Count (1)
    0x81, 0x00,        // Input (Data,Array)
    
    0xC0               // End Collection
};

#define KOREAN_HID_DESC_SIZE sizeof(korean_hid_report_desc)

// HID Report 구조체 (TinyUSB와 충돌 방지)
typedef struct {
    uint8_t modifiers;
    uint8_t reserved;
    uint8_t keys[6];
} __attribute__((packed)) korean_hid_keyboard_report_t;

typedef struct {
    uint16_t usage_code;
} __attribute__((packed)) hid_consumer_report_t;

// 키보드 상태 구조체
typedef struct {
    bool is_initialized;
    language_mode_t current_mode;
    hangul_toggle_method_t toggle_method;
    uint32_t last_toggle_time;
    uint32_t toggle_count;
    bool debug_enabled;
} keyboard_state_t;

// 한국어 키보드 클래스
class KoreanUSBHID {
private:
    keyboard_state_t _state;
    korean_hid_keyboard_report_t _keyboard_report;
    hid_consumer_report_t _consumer_report;
    Adafruit_USBD_HID _usb_hid;
    
    void _init_state(void);
    void _reset_reports(void);
    bool _send_report(uint8_t report_id, const void* data, uint16_t len);
    
public:
    KoreanUSBHID();
    bool begin(void);
    void end(void);
    
    // 한영 전환
    bool toggleLanguage(void);
    bool switchToKorean(void);
    bool switchToEnglish(void);
    language_mode_t getCurrentMode(void) const;
    
    // 키 전송
    bool sendKey(uint8_t keycode, uint8_t modifiers = 0);
    bool sendKeyCombo(uint8_t modifier, uint8_t keycode);
    bool sendConsumerKey(uint16_t usage_code);
    bool releaseAll(void);
    
    // 상태 관리
    void printStatus(void);
    void printStats(void);
    bool isConnected(void) const;
    bool isInitialized(void) const;
};

#endif // USB_DESCRIPTORS_H