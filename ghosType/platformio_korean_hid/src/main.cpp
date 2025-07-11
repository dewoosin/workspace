/**
 * GHOSTYPE Korean HID - 최소 테스트 버전
 */

#include <Arduino.h>
#include "Adafruit_TinyUSB.h"

// HID Report Descriptor
uint8_t const desc_hid_report[] = {
    0x05, 0x01,                    // Usage Page (Generic Desktop Ctrls)
    0x09, 0x06,                    // Usage (Keyboard)
    0xA1, 0x01,                    // Collection (Application)
    0x05, 0x07,                    //   Usage Page (Kbrd/Keypad)
    0x19, 0xE0,                    //   Usage Minimum (0xE0)
    0x29, 0xE7,                    //   Usage Maximum (0xE7)
    0x15, 0x00,                    //   Logical Minimum (0)
    0x25, 0x01,                    //   Logical Maximum (1)
    0x75, 0x01,                    //   Report Size (1)
    0x95, 0x08,                    //   Report Count (8)
    0x81, 0x02,                    //   Input (Data,Var,Abs)
    0x95, 0x01,                    //   Report Count (1)
    0x75, 0x08,                    //   Report Size (8)
    0x81, 0x01,                    //   Input (Const,Array,Abs)
    0x95, 0x05,                    //   Report Count (5)
    0x75, 0x01,                    //   Report Size (1)
    0x05, 0x08,                    //   Usage Page (LEDs)
    0x19, 0x01,                    //   Usage Minimum (Num Lock)
    0x29, 0x05,                    //   Usage Maximum (Kana)
    0x91, 0x02,                    //   Output (Data,Var,Abs)
    0x95, 0x01,                    //   Report Count (1)
    0x75, 0x03,                    //   Report Size (3)
    0x91, 0x01,                    //   Output (Const,Array,Abs)
    0x95, 0x06,                    //   Report Count (6)
    0x75, 0x08,                    //   Report Size (8)
    0x15, 0x00,                    //   Logical Minimum (0)
    0x25, 0x65,                    //   Logical Maximum (101)
    0x05, 0x07,                    //   Usage Page (Kbrd/Keypad)
    0x19, 0x00,                    //   Usage Minimum (0x00)
    0x29, 0x65,                    //   Usage Maximum (0x65)
    0x81, 0x00,                    //   Input (Data,Array,Abs)
    0xC0,                          // End Collection
};

// HID 객체
Adafruit_USBD_HID usb_hid(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_KEYBOARD, 2, false);

// 키보드 리포트 구조체
typedef struct {
    uint8_t modifier;
    uint8_t reserved;
    uint8_t keycode[6];
} hid_keyboard_report_t;

void setup() {
    Serial.begin(115200);
    
    // USB 설정
    TinyUSBDevice.setManufacturerDescriptor("Samsung Electronics");
    TinyUSBDevice.setProductDescriptor("Korean USB Keyboard");
    TinyUSBDevice.setSerialDescriptor("KR2024KB001");
    TinyUSBDevice.setID(0x04E8, 0x7021);  // Samsung VID/PID
    
    // USB HID 시작
    usb_hid.begin();
    
    // USB 장치 시작
    Serial.println("USB HID 초기화 중...");
    
    // 연결 대기
    while (!TinyUSBDevice.mounted()) {
        delay(1);
    }
    
    Serial.println("✅ USB HID 연결 성공!");
    delay(1000);
}

void loop() {
    // 단순 테스트: 5초마다 'A' 키 전송
    static unsigned long lastSend = 0;
    
    if (millis() - lastSend > 5000) {
        Serial.println("📝 'A' 키 전송 중...");
        
        // 키보드 리포트 생성
        hid_keyboard_report_t report = {0};
        report.keycode[0] = 0x04;  // 'A' 키
        
        // HID 리포트 전송
        usb_hid.sendReport(0, &report, sizeof(report));
        
        delay(100);
        
        // 키 해제
        memset(&report, 0, sizeof(report));
        usb_hid.sendReport(0, &report, sizeof(report));
        
        lastSend = millis();
    }
    
    delay(100);
}