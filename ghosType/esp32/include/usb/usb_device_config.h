#ifndef USB_DEVICE_CONFIG_H
#define USB_DEVICE_CONFIG_H

#include <stdint.h>

/**
 * @file usb_device_config.h
 * @brief USB Device Descriptor 설정 - 한국어 키보드 전용
 * 
 * Windows에서 한국어 키보드로 인식되도록 하는 핵심 설정
 */

// 한국어 키보드 Vendor/Product ID
#define USB_VID_SAMSUNG         0x04E8    // 삼성전자
#define USB_VID_LG              0x1FD2    // LG전자  
#define USB_VID_HANSUNG         0x0C45    // 한성컴퓨터
#define USB_VID_ABKO            0x0D8C    // 앱코

// 실제 한국어 키보드 Product ID
#define USB_PID_KOREAN_KB       0x7021    // 삼성 한국어 키보드
#define USB_PID_KOREAN_KB_ALT   0x7022    // 대체 PID

// 기본 설정 (삼성전자 기준)
#define USB_VENDOR_ID           USB_VID_SAMSUNG
#define USB_PRODUCT_ID          USB_PID_KOREAN_KB

// Language ID (Windows 중요!)
#define LANG_ID_KOREAN          0x0412    // 한국어 (대한민국)
#define LANG_ID_ENGLISH_US      0x0409    // 영어 (미국)

// String Descriptor Index
#define STRING_IDX_LANGUAGE     0
#define STRING_IDX_MANUFACTURER 1
#define STRING_IDX_PRODUCT      2
#define STRING_IDX_SERIAL       3
#define STRING_IDX_CONFIG       4
#define STRING_IDX_INTERFACE    5

// USB Device Descriptor 구조체
typedef struct {
    uint8_t  bLength;            // 18
    uint8_t  bDescriptorType;    // 1 (Device)
    uint16_t bcdUSB;             // 0x0200 (USB 2.0)
    uint8_t  bDeviceClass;       // 0 (Class defined in interface)
    uint8_t  bDeviceSubClass;    // 0
    uint8_t  bDeviceProtocol;    // 0
    uint8_t  bMaxPacketSize0;    // 64
    uint16_t idVendor;           // Vendor ID
    uint16_t idProduct;          // Product ID
    uint16_t bcdDevice;          // Device release
    uint8_t  iManufacturer;      // Manufacturer string index
    uint8_t  iProduct;           // Product string index
    uint8_t  iSerialNumber;      // Serial number string index
    uint8_t  bNumConfigurations; // 1
} __attribute__((packed)) usb_device_descriptor_t;

// 한국어 키보드 Device Descriptor
static const usb_device_descriptor_t korean_device_descriptor = {
    .bLength = 18,
    .bDescriptorType = 0x01,     // Device
    .bcdUSB = 0x0200,            // USB 2.0
    .bDeviceClass = 0x00,        // Class defined in interface
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = 64,
    .idVendor = USB_VENDOR_ID,
    .idProduct = USB_PRODUCT_ID,
    .bcdDevice = 0x0100,         // Version 1.0
    .iManufacturer = STRING_IDX_MANUFACTURER,
    .iProduct = STRING_IDX_PRODUCT,
    .iSerialNumber = STRING_IDX_SERIAL,
    .bNumConfigurations = 1
};

// Manufacturer String (한국어 + 영어)
static const char* manufacturer_string_kr = "삼성전자";
static const char* manufacturer_string_en = "Samsung Electronics";

// Product String (한국어 + 영어)
static const char* product_string_kr = "한글 USB 키보드";
static const char* product_string_en = "Korean USB Keyboard";

// Serial Number
static const char* serial_number = "KR2024KB001";

// Configuration String
static const char* config_string = "Korean Keyboard Configuration";

// Interface String
static const char* interface_string = "Korean HID Interface";

// USB String Descriptor 헬퍼 함수
uint8_t get_string_descriptor(uint8_t index, uint16_t langid, uint8_t* buffer, uint16_t buffer_size);

// UTF-8 to UTF-16 변환 함수
uint16_t utf8_to_utf16(const char* utf8_str, uint16_t* utf16_buffer, uint16_t max_len);

// Device Descriptor 접근 함수
const usb_device_descriptor_t* get_device_descriptor(void);

#endif // USB_DEVICE_CONFIG_H