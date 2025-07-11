#ifndef USB_CONFIG_DESCRIPTOR_H
#define USB_CONFIG_DESCRIPTOR_H

#include <stdint.h>
#include "hid/hid_descriptor_korean.h"

/**
 * @file usb_config_descriptor.h
 * @brief USB Configuration Descriptor - 한국어 키보드 전용
 * 
 * HID Country Code = 16 (Korean) 설정이 핵심!
 * Windows에서 한국어 키보드로 인식하는 결정적 요소
 */

// HID Country Code (USB HID 1.11 스펙)
#define HID_COUNTRY_NONE        0     // Not supported
#define HID_COUNTRY_ARABIC      1     // Arabic
#define HID_COUNTRY_BELGIAN     2     // Belgian
#define HID_COUNTRY_CANADIAN_BI 3     // Canadian-Bilingual
#define HID_COUNTRY_CANADIAN_FR 4     // Canadian-French
#define HID_COUNTRY_CZECH       5     // Czech Republic
#define HID_COUNTRY_DANISH      6     // Danish
#define HID_COUNTRY_FINNISH     7     // Finnish
#define HID_COUNTRY_FRENCH      8     // French
#define HID_COUNTRY_GERMAN      9     // German
#define HID_COUNTRY_GREEK       10    // Greek
#define HID_COUNTRY_HEBREW      11    // Hebrew
#define HID_COUNTRY_HUNGARY     12    // Hungary
#define HID_COUNTRY_ISO         13    // International (ISO)
#define HID_COUNTRY_ITALIAN     14    // Italian
#define HID_COUNTRY_JAPANESE    15    // Japan (Katakana)
#define HID_COUNTRY_KOREAN      16    // Korean 🔥🔥🔥
#define HID_COUNTRY_LATIN_AM    17    // Latin American
#define HID_COUNTRY_DUTCH       18    // Netherlands/Dutch
#define HID_COUNTRY_NORWEGIAN   19    // Norwegian
#define HID_COUNTRY_PERSIAN     20    // Persian (Farsi)
#define HID_COUNTRY_POLAND      21    // Poland
#define HID_COUNTRY_PORTUGUESE  22    // Portuguese
#define HID_COUNTRY_RUSSIA      23    // Russia
#define HID_COUNTRY_SLOVAKIA    24    // Slovakia
#define HID_COUNTRY_SPANISH     25    // Spanish
#define HID_COUNTRY_SWEDISH     26    // Swedish
#define HID_COUNTRY_SWISS_FR    27    // Swiss/French
#define HID_COUNTRY_SWISS_GR    28    // Swiss/German
#define HID_COUNTRY_SWISS       29    // Switzerland
#define HID_COUNTRY_TAIWAN      30    // Taiwan
#define HID_COUNTRY_TURKISH_Q   31    // Turkish-Q
#define HID_COUNTRY_UK          32    // United Kingdom
#define HID_COUNTRY_US          33    // United States
#define HID_COUNTRY_YUGOSLAVIA  34    // Yugoslavia
#define HID_COUNTRY_TURKISH_F   35    // Turkish-F

// 전체 Configuration Descriptor 구조체
typedef struct {
    // Configuration Descriptor
    uint8_t  bLength_config;         // 9
    uint8_t  bDescriptorType_config; // 2 (Configuration)
    uint16_t wTotalLength;           // Total length
    uint8_t  bNumInterfaces;         // 1
    uint8_t  bConfigurationValue;    // 1
    uint8_t  iConfiguration;         // String index
    uint8_t  bmAttributes;           // 0xA0 (Remote Wakeup)
    uint8_t  bMaxPower;              // 50 (100mA)
    
    // Interface Descriptor
    uint8_t  bLength_interface;      // 9
    uint8_t  bDescriptorType_interface; // 4 (Interface)
    uint8_t  bInterfaceNumber;       // 0
    uint8_t  bAlternateSetting;      // 0
    uint8_t  bNumEndpoints;          // 2
    uint8_t  bInterfaceClass;        // 3 (HID)
    uint8_t  bInterfaceSubClass;     // 1 (Boot Interface)
    uint8_t  bInterfaceProtocol;     // 1 (Keyboard)
    uint8_t  iInterface;             // String index
    
    // HID Descriptor
    uint8_t  bLength_hid;            // 9
    uint8_t  bDescriptorType_hid;    // 0x21 (HID)
    uint16_t bcdHID;                 // 0x0111 (HID 1.11)
    uint8_t  bCountryCode;           // 16 (Korean) 🔥🔥🔥
    uint8_t  bNumDescriptors;        // 1
    uint8_t  bDescriptorType_report; // 0x22 (Report)
    uint16_t wDescriptorLength;      // Report descriptor length
    
    // Endpoint Descriptor (IN)
    uint8_t  bLength_ep_in;          // 7
    uint8_t  bDescriptorType_ep_in;  // 5 (Endpoint)
    uint8_t  bEndpointAddress_in;    // 0x81 (EP1 IN)
    uint8_t  bmAttributes_in;        // 3 (Interrupt)
    uint16_t wMaxPacketSize_in;      // 8
    uint8_t  bInterval_in;           // 10ms
    
    // Endpoint Descriptor (OUT)
    uint8_t  bLength_ep_out;         // 7
    uint8_t  bDescriptorType_ep_out; // 5 (Endpoint)
    uint8_t  bEndpointAddress_out;   // 0x01 (EP1 OUT)
    uint8_t  bmAttributes_out;       // 3 (Interrupt)
    uint16_t wMaxPacketSize_out;     // 8
    uint8_t  bInterval_out;          // 10ms
    
} __attribute__((packed)) usb_config_descriptor_t;

// 한국어 키보드 Configuration Descriptor
static const usb_config_descriptor_t korean_config_descriptor = {
    // Configuration Descriptor
    .bLength_config = 9,
    .bDescriptorType_config = 2,
    .wTotalLength = sizeof(usb_config_descriptor_t),
    .bNumInterfaces = 1,
    .bConfigurationValue = 1,
    .iConfiguration = 4,             // Config string index
    .bmAttributes = 0xA0,            // Remote Wakeup
    .bMaxPower = 50,                 // 100mA
    
    // Interface Descriptor
    .bLength_interface = 9,
    .bDescriptorType_interface = 4,
    .bInterfaceNumber = 0,
    .bAlternateSetting = 0,
    .bNumEndpoints = 2,
    .bInterfaceClass = 3,            // HID
    .bInterfaceSubClass = 1,         // Boot Interface
    .bInterfaceProtocol = 1,         // Keyboard
    .iInterface = 5,                 // Interface string index
    
    // HID Descriptor
    .bLength_hid = 9,
    .bDescriptorType_hid = 0x21,
    .bcdHID = 0x0111,                // HID 1.11
    .bCountryCode = HID_COUNTRY_KOREAN,  // 🔥 핵심! 한국어 키보드
    .bNumDescriptors = 1,
    .bDescriptorType_report = 0x22,
    .wDescriptorLength = KOREAN_HID_DESC_SIZE,
    
    // Endpoint IN (키보드 → 호스트)
    .bLength_ep_in = 7,
    .bDescriptorType_ep_in = 5,
    .bEndpointAddress_in = 0x81,     // EP1 IN
    .bmAttributes_in = 3,            // Interrupt
    .wMaxPacketSize_in = 8,
    .bInterval_in = 10,              // 10ms
    
    // Endpoint OUT (호스트 → 키보드, LED 제어)
    .bLength_ep_out = 7,
    .bDescriptorType_ep_out = 5,
    .bEndpointAddress_out = 0x01,    // EP1 OUT
    .bmAttributes_out = 3,           // Interrupt
    .wMaxPacketSize_out = 8,
    .bInterval_out = 10              // 10ms
};

// HID Class Descriptor 구조체
typedef struct {
    uint8_t  bLength;                // 9
    uint8_t  bDescriptorType;        // 0x21 (HID)
    uint16_t bcdHID;                 // HID version
    uint8_t  bCountryCode;           // Country code
    uint8_t  bNumDescriptors;        // Number of descriptors
    uint8_t  bDescriptorType_class;  // Descriptor type
    uint16_t wDescriptorLength;      // Descriptor length
} __attribute__((packed)) hid_class_descriptor_t;

// 한국어 키보드 HID Class Descriptor
static const hid_class_descriptor_t korean_hid_class_descriptor = {
    .bLength = 9,
    .bDescriptorType = 0x21,
    .bcdHID = 0x0111,
    .bCountryCode = HID_COUNTRY_KOREAN,  // 🔥 핵심!
    .bNumDescriptors = 1,
    .bDescriptorType_class = 0x22,
    .wDescriptorLength = KOREAN_HID_DESC_SIZE
};

// Configuration Descriptor 접근 함수
const usb_config_descriptor_t* get_config_descriptor(void);
const hid_class_descriptor_t* get_hid_class_descriptor(void);

// HID Report Descriptor 접근 함수
const uint8_t* get_hid_report_descriptor(void);
uint16_t get_hid_report_descriptor_size(void);

#endif // USB_CONFIG_DESCRIPTOR_H