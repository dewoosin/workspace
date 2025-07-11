#include "usb/usb_device_config.h"
#include "esp32-hal-tinyusb.h"
#include <Arduino.h>

/**
 * @file usb_string_descriptors.cpp
 * @brief USB String Descriptor 구현
 * 
 * Windows에서 한국어 키보드로 인식되도록 하는 문자열 설정
 * 한국어 + 영어 String Descriptor 지원
 */

// UTF-8 to UTF-16 변환 함수
uint16_t utf8_to_utf16(const char* utf8_str, uint16_t* utf16_buffer, uint16_t max_len) {
    if (!utf8_str || !utf16_buffer || max_len == 0) {
        return 0;
    }
    
    uint16_t utf16_count = 0;
    const uint8_t* utf8 = (const uint8_t*)utf8_str;
    
    while (*utf8 && utf16_count < max_len) {
        uint8_t c = *utf8++;
        
        if (c < 0x80) {
            // ASCII (0x00-0x7F)
            utf16_buffer[utf16_count++] = c;
        } else if ((c & 0xE0) == 0xC0) {
            // 2-byte UTF-8 (0xC0-0xDF)
            if (*utf8) {
                uint16_t unicode = ((c & 0x1F) << 6) | (*utf8++ & 0x3F);
                utf16_buffer[utf16_count++] = unicode;
            }
        } else if ((c & 0xF0) == 0xE0) {
            // 3-byte UTF-8 (0xE0-0xEF) - 한글 포함
            if (*utf8 && *(utf8 + 1)) {
                uint16_t unicode = ((c & 0x0F) << 12) | 
                                  ((*utf8++ & 0x3F) << 6) | 
                                  (*utf8++ & 0x3F);
                utf16_buffer[utf16_count++] = unicode;
            }
        } else if ((c & 0xF8) == 0xF0) {
            // 4-byte UTF-8 (0xF0-0xF7) - Surrogate pairs
            if (*utf8 && *(utf8 + 1) && *(utf8 + 2)) {
                uint32_t unicode = ((c & 0x07) << 18) | 
                                  ((*utf8++ & 0x3F) << 12) | 
                                  ((*utf8++ & 0x3F) << 6) | 
                                  (*utf8++ & 0x3F);
                
                // Unicode를 UTF-16 Surrogate pairs로 변환
                if (unicode > 0xFFFF && utf16_count < max_len - 1) {
                    unicode -= 0x10000;
                    utf16_buffer[utf16_count++] = 0xD800 + (unicode >> 10);
                    utf16_buffer[utf16_count++] = 0xDC00 + (unicode & 0x3FF);
                }
            }
        }
    }
    
    return utf16_count;
}

// TinyUSB String Descriptor 콜백 함수
extern "C" {

const uint16_t* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    static uint16_t desc_str[64];  // 충분한 버퍼 크기
    uint8_t chr_count = 0;
    
    switch (index) {
        case STRING_IDX_LANGUAGE:
            // Language ID String Descriptor
            desc_str[1] = LANG_ID_KOREAN;   // 한국어 (0x0412)
            desc_str[2] = LANG_ID_ENGLISH_US; // 영어 (0x0409)
            chr_count = 2;
            break;
            
        case STRING_IDX_MANUFACTURER:
            // 제조사명
            if (langid == LANG_ID_KOREAN) {
                // 한국어: "삼성전자"
                chr_count = utf8_to_utf16(manufacturer_string_kr, desc_str + 1, sizeof(desc_str) / 2 - 1);
            } else {
                // 영어: "Samsung Electronics"
                chr_count = utf8_to_utf16(manufacturer_string_en, desc_str + 1, sizeof(desc_str) / 2 - 1);
            }
            break;
            
        case STRING_IDX_PRODUCT:
            // 제품명
            if (langid == LANG_ID_KOREAN) {
                // 한국어: "한글 USB 키보드"
                chr_count = utf8_to_utf16(product_string_kr, desc_str + 1, sizeof(desc_str) / 2 - 1);
            } else {
                // 영어: "Korean USB Keyboard"
                chr_count = utf8_to_utf16(product_string_en, desc_str + 1, sizeof(desc_str) / 2 - 1);
            }
            break;
            
        case STRING_IDX_SERIAL:
            // 시리얼 번호
            chr_count = utf8_to_utf16(serial_number, desc_str + 1, sizeof(desc_str) / 2 - 1);
            break;
            
        case STRING_IDX_CONFIG:
            // Configuration 이름
            chr_count = utf8_to_utf16(config_string, desc_str + 1, sizeof(desc_str) / 2 - 1);
            break;
            
        case STRING_IDX_INTERFACE:
            // Interface 이름
            chr_count = utf8_to_utf16(interface_string, desc_str + 1, sizeof(desc_str) / 2 - 1);
            break;
            
        default:
            return NULL;
    }
    
    // String Descriptor 헤더 설정
    // [0] = Length (2 bytes) + Type (1 byte) + Length field (1 byte)
    // [1~N] = String data in UTF-16LE
    desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);
    
    return desc_str;
}

// Device Descriptor 콜백 함수
const uint8_t* tud_descriptor_device_cb(void) {
    return (const uint8_t*)&korean_device_descriptor;
}

// Configuration Descriptor 콜백 함수
const uint8_t* tud_descriptor_configuration_cb(uint8_t index) {
    (void)index; // 하나의 Configuration만 사용
    return (const uint8_t*)&korean_config_descriptor;
}

// HID Report Descriptor 콜백 함수
const uint8_t* tud_hid_descriptor_report_cb(uint8_t itf) {
    (void)itf; // 하나의 HID 인터페이스만 사용
    return korean_hid_report_desc;
}

} // extern "C"

// Device Descriptor 접근 함수
const usb_device_descriptor_t* get_device_descriptor(void) {
    return &korean_device_descriptor;
}

// String Descriptor 접근 함수 (C++ 인터페이스)
uint8_t get_string_descriptor(uint8_t index, uint16_t langid, uint8_t* buffer, uint16_t buffer_size) {
    const uint16_t* desc = tud_descriptor_string_cb(index, langid);
    
    if (!desc || !buffer || buffer_size == 0) {
        return 0;
    }
    
    // String Descriptor 길이 계산
    uint8_t desc_len = desc[0] & 0xFF;
    
    if (desc_len > buffer_size) {
        desc_len = buffer_size;
    }
    
    // 버퍼에 복사
    memcpy(buffer, desc, desc_len);
    
    return desc_len;
}