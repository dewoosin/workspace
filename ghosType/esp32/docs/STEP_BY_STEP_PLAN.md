# USB HID Descriptor 기반 한국어 키보드 구현 - 단계별 상세 계획

## 🎯 최종 목표
ESP32-S3를 Windows가 **진짜 한국어 키보드**로 인식하도록 만들어서 한영 전환이 완벽하게 작동하게 하기

## 📋 전체 로드맵

### Phase 1: 준비 및 분석 (1-2단계)
- 현재 코드 백업 및 분석
- 필요한 라이브러리 및 도구 준비

### Phase 2: 기본 구현 (3-5단계)  
- USB Descriptor 기본 수정
- TinyUSB 기반 재작성
- 첫 번째 테스트

### Phase 3: 고급 구현 (6-8단계)
- 다중 Report 구조
- 12가지 한영 전환 방식 구현
- 진단 도구 통합

### Phase 4: 최적화 및 완성 (9-10단계)
- 성능 최적화
- 최종 테스트 및 검증

---

## 🔧 STEP 1: 현재 코드 백업 및 분석

### 1.1 백업 생성
```bash
# 현재 작업 디렉토리 백업
cp -r /Users/workspace/ghosType/esp32 /Users/workspace/ghosType/esp32_backup_$(date +%Y%m%d_%H%M%S)
```

### 1.2 현재 코드 분석 체크리스트
- [ ] 현재 사용 중인 라이브러리 목록 확인
- [ ] BLE 통신 부분 분석
- [ ] 한영 전환 로직 분석
- [ ] 프로토콜 명령 처리 부분 분석
- [ ] 메모리 사용량 확인

### 1.3 분석 결과 문서화
- 현재 코드의 장단점 정리
- 유지해야 할 기능 목록
- 수정해야 할 부분 목록

---

## 🔧 STEP 2: 개발 환경 및 도구 준비

### 2.1 PlatformIO 설정 업데이트
```ini
# platformio.ini 수정 사항
[env:esp32-s3-devkitc-1]
platform = espressif32@6.4.0
board = esp32-s3-devkitc-1
framework = arduino

# TinyUSB 관련 플래그 추가
board_build.arduino.memory_type = qio_opi
board_build.flash_size = 8MB
board_build.psram_type = opi

# USB 커스터마이징 플래그
board_flags = 
    -DARDUINO_USB_MODE=1
    -DARDUINO_USB_CDC_ON_BOOT=1
    -DARDUINO_USB_MSC_ON_BOOT=0
    -DARDUINO_USB_DFU_ON_BOOT=0

# 한국어 키보드 설정
build_flags = 
    -DUSB_VID=0x04E8
    -DUSB_PID=0x7021
    -DUSB_MANUFACTURER="Samsung Electronics"
    -DUSB_PRODUCT="Korean USB Keyboard"
    -DUSB_SERIAL="KR2024KB001"
    -DUSE_TINYUSB=1
    -DCFG_TUD_HID=2
    -DCFG_TUD_HID_EP_BUFSIZE=64

lib_deps = 
    adafruit/Adafruit TinyUSB Library@^2.2.6
    NimBLE-Arduino@^1.4.1
```

### 2.2 필요한 헤더 파일 준비
- [ ] `hid_descriptor_korean.h` 생성
- [ ] `usb_device_config.h` 생성  
- [ ] `usb_config_descriptor.h` 생성
- [ ] `esp32_usb_hid_korean.h` 생성

### 2.3 디버깅 도구 준비
- [ ] 시리얼 모니터 설정
- [ ] USB 장치 분석 도구 (USBlyzer 등)
- [ ] Windows 장치 관리자 모니터링

---

## 🔧 STEP 3: USB Descriptor 기본 구현

### 3.1 한국어 키보드 HID Report Descriptor 생성
```cpp
// include/hid_descriptor_korean.h
#ifndef HID_DESCRIPTOR_KOREAN_H
#define HID_DESCRIPTOR_KOREAN_H

#include <stdint.h>

// 한국어 키보드 전용 HID Report Descriptor
static const uint8_t korean_hid_report_desc[] = {
    // 표준 키보드 부분
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x06,        // Usage (Keyboard)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        // Report ID (1)
    
    // Modifier keys (Ctrl, Shift, Alt 등)
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
    
    // 일반 키 배열 (6개 동시 입력)
    0x05, 0x07,        // Usage Page (Keyboard)
    0x19, 0x00,        // Usage Minimum (0)
    0x2A, 0xFF, 0x00,  // Usage Maximum (255)
    0x15, 0x00,        // Logical Minimum (0)
    0x26, 0xFF, 0x00,  // Logical Maximum (255)
    0x75, 0x08,        // Report Size (8)
    0x95, 0x06,        // Report Count (6)
    0x81, 0x00,        // Input (Data,Array)
    
    // LED 출력
    0x05, 0x08,        // Usage Page (LEDs)
    0x19, 0x01,        // Usage Minimum (Num Lock)
    0x29, 0x05,        // Usage Maximum (Kana)
    0x75, 0x01,        // Report Size (1)
    0x95, 0x05,        // Report Count (5)
    0x91, 0x02,        // Output (Data,Var,Abs)
    
    // LED 패딩
    0x75, 0x03,        // Report Size (3)
    0x95, 0x01,        // Report Count (1)
    0x91, 0x01,        // Output (Const)
    
    0xC0,              // End Collection
    
    // Consumer Control (한/영, 한자용)
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

#endif // HID_DESCRIPTOR_KOREAN_H
```

### 3.2 테스트 체크리스트
- [ ] 컴파일 성공 확인
- [ ] ESP32 업로드 성공 확인
- [ ] Windows 장치 관리자에서 인식 확인
- [ ] 기존 BLE 기능 정상 동작 확인

---

## 🔧 STEP 4: Device Descriptor 커스터마이징

### 4.1 한국어 키보드 Device Descriptor 설정
```cpp
// include/usb_device_config.h
#ifndef USB_DEVICE_CONFIG_H
#define USB_DEVICE_CONFIG_H

#include <stdint.h>

// 한국어 키보드로 인식되는 Vendor/Product ID
#define VENDOR_ID_SAMSUNG    0x04E8  // 삼성전자
#define PRODUCT_ID_KOREAN_KB 0x7021  // 한국어 키보드

// Language ID 설정
#define LANGUAGE_ID_KOREAN   0x0412  // 한국어 언어 ID
#define LANGUAGE_ID_ENGLISH  0x0409  // 영어 언어 ID

// String Descriptor 인덱스
#define STRING_INDEX_LANGUAGE    0
#define STRING_INDEX_MANUFACTURER 1
#define STRING_INDEX_PRODUCT     2
#define STRING_INDEX_SERIAL      3

// USB Device Descriptor 구조체
typedef struct {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdUSB;
    uint8_t bDeviceClass;
    uint8_t bDeviceSubClass;
    uint8_t bDeviceProtocol;
    uint8_t bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t iManufacturer;
    uint8_t iProduct;
    uint8_t iSerialNumber;
    uint8_t bNumConfigurations;
} __attribute__((packed)) usb_device_descriptor_t;

// 한국어 키보드용 Device Descriptor
static const usb_device_descriptor_t korean_device_desc = {
    .bLength = sizeof(usb_device_descriptor_t),
    .bDescriptorType = 0x01,  // Device
    .bcdUSB = 0x0200,         // USB 2.0
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = 64,
    .idVendor = VENDOR_ID_SAMSUNG,
    .idProduct = PRODUCT_ID_KOREAN_KB,
    .bcdDevice = 0x0100,
    .iManufacturer = STRING_INDEX_MANUFACTURER,
    .iProduct = STRING_INDEX_PRODUCT,
    .iSerialNumber = STRING_INDEX_SERIAL,
    .bNumConfigurations = 1
};

#endif // USB_DEVICE_CONFIG_H
```

### 4.2 테스트 체크리스트
- [ ] Windows에서 "Samsung Electronics" 제조사로 인식 확인
- [ ] 장치 관리자에서 "Korean USB Keyboard" 표시 확인
- [ ] 하드웨어 ID가 "USB\VID_04E8&PID_7021" 확인
- [ ] 기본 키보드 입력 동작 확인

---

## 🔧 STEP 5: Configuration Descriptor 및 Country Code 설정

### 5.1 Configuration Descriptor 구현
```cpp
// include/usb_config_descriptor.h
#ifndef USB_CONFIG_DESCRIPTOR_H
#define USB_CONFIG_DESCRIPTOR_H

#include <stdint.h>

// Country Code 정의 (HID 1.11 spec)
#define HID_COUNTRY_KOREAN 16  // 한국 Country Code

// Configuration Descriptor 구조체
typedef struct {
    // Configuration Descriptor
    uint8_t bLength_config;
    uint8_t bDescriptorType_config;
    uint16_t wTotalLength;
    uint8_t bNumInterfaces;
    uint8_t bConfigurationValue;
    uint8_t iConfiguration;
    uint8_t bmAttributes;
    uint8_t bMaxPower;
    
    // Interface Descriptor  
    uint8_t bLength_interface;
    uint8_t bDescriptorType_interface;
    uint8_t bInterfaceNumber;
    uint8_t bAlternateSetting;
    uint8_t bNumEndpoints;
    uint8_t bInterfaceClass;
    uint8_t bInterfaceSubClass;
    uint8_t bInterfaceProtocol;
    uint8_t iInterface;
    
    // HID Descriptor
    uint8_t bLength_hid;
    uint8_t bDescriptorType_hid;
    uint16_t bcdHID;
    uint8_t bCountryCode;  // 🔥 이 부분이 핵심!
    uint8_t bNumDescriptors;
    uint8_t bDescriptorType_report;
    uint16_t wDescriptorLength;
    
    // Endpoint Descriptor (IN)
    uint8_t bLength_ep_in;
    uint8_t bDescriptorType_ep_in;
    uint8_t bEndpointAddress_in;
    uint8_t bmAttributes_in;
    uint16_t wMaxPacketSize_in;
    uint8_t bInterval_in;
    
    // Endpoint Descriptor (OUT)
    uint8_t bLength_ep_out;
    uint8_t bDescriptorType_ep_out;
    uint8_t bEndpointAddress_out;
    uint8_t bmAttributes_out;
    uint16_t wMaxPacketSize_out;
    uint8_t bInterval_out;
    
} __attribute__((packed)) config_descriptor_t;

// 한국어 키보드용 Configuration Descriptor
static const config_descriptor_t korean_config_desc = {
    // Configuration Descriptor
    .bLength_config = 9,
    .bDescriptorType_config = 0x02,
    .wTotalLength = sizeof(config_descriptor_t),
    .bNumInterfaces = 1,
    .bConfigurationValue = 1,
    .iConfiguration = 0,
    .bmAttributes = 0xA0,
    .bMaxPower = 50,
    
    // Interface Descriptor
    .bLength_interface = 9,
    .bDescriptorType_interface = 0x04,
    .bInterfaceNumber = 0,
    .bAlternateSetting = 0,
    .bNumEndpoints = 2,
    .bInterfaceClass = 0x03,     // HID Class
    .bInterfaceSubClass = 0x01,  // Boot Interface
    .bInterfaceProtocol = 0x01,  // Keyboard
    .iInterface = 0,
    
    // HID Descriptor
    .bLength_hid = 9,
    .bDescriptorType_hid = 0x21,
    .bcdHID = 0x0111,
    .bCountryCode = HID_COUNTRY_KOREAN,  // 🔥 한국 Country Code!
    .bNumDescriptors = 1,
    .bDescriptorType_report = 0x22,
    .wDescriptorLength = KOREAN_HID_DESC_SIZE,
    
    // Endpoint IN
    .bLength_ep_in = 7,
    .bDescriptorType_ep_in = 0x05,
    .bEndpointAddress_in = 0x81,
    .bmAttributes_in = 0x03,
    .wMaxPacketSize_in = 8,
    .bInterval_in = 10,
    
    // Endpoint OUT
    .bLength_ep_out = 7,
    .bDescriptorType_ep_out = 0x05,
    .bEndpointAddress_out = 0x01,
    .bmAttributes_out = 0x03,
    .wMaxPacketSize_out = 8,
    .bInterval_out = 10
};

#endif // USB_CONFIG_DESCRIPTOR_H
```

### 5.2 테스트 체크리스트
- [ ] HID Descriptor에서 Country Code = 16 확인
- [ ] USB Device Tree에서 한국어 키보드 속성 확인
- [ ] Windows 지역 설정에서 키보드 인식 확인

---

## 🔧 STEP 6: TinyUSB 기반 메인 클래스 구현

### 6.1 KoreanUSBHID 클래스 기본 구조
```cpp
// include/esp32_usb_hid_korean.h
#ifndef ESP32_USB_HID_KOREAN_H
#define ESP32_USB_HID_KOREAN_H

#include <Arduino.h>
#include "USB.h"
#include "USBHID.h"
#include "esp32-hal-tinyusb.h"
#include "tusb.h"
#include "hid_descriptor_korean.h"

class KoreanUSBHID : public USBHID {
private:
    // HID Report 구조체
    typedef struct {
        uint8_t modifiers;    // Ctrl, Shift, Alt 등
        uint8_t reserved;     // 예약 바이트
        uint8_t keys[6];      // 동시 입력 가능한 6개 키
    } hid_keyboard_report_t;
    
    // Consumer Control Report
    typedef struct {
        uint16_t usage_code;  // Consumer usage code
    } hid_consumer_report_t;
    
    hid_keyboard_report_t _keyReport;
    hid_consumer_report_t _consumerReport;
    
    // 한국어 키보드 전용 Usage Code
    static const uint16_t CONSUMER_HANGUL = 0x090;
    static const uint16_t CONSUMER_HANJA = 0x091;
    
    // 상태 변수
    bool _isInitialized;
    bool _isKoreanMode;
    
public:
    KoreanUSBHID();
    
    // 초기화
    void begin();
    
    // HID Report Descriptor 설정
    void setHIDReportDescriptor();
    
    // 한/영 키 전송 메소드들
    bool sendHangulKey();
    bool sendHangulConsumer();
    bool sendHangulCombo(uint8_t modifier, uint8_t key);
    
    // 키 릴리즈
    bool releaseAll();
    
    // 상태 확인
    bool isKoreanMode() const { return _isKoreanMode; }
    
    // 디버깅
    void printStatus();
};

// 전역 인스턴스
extern KoreanUSBHID KoreanKeyboard;

#endif // ESP32_USB_HID_KOREAN_H
```

### 6.2 구현 우선순위
1. **기본 생성자 및 초기화**
2. **HID Report Descriptor 등록**
3. **간단한 키 전송 메소드**
4. **상태 관리**
5. **디버깅 기능**

### 6.3 테스트 체크리스트
- [ ] 클래스 인스턴스 생성 성공
- [ ] HID Report Descriptor 등록 성공
- [ ] 기본 키 입력 테스트 성공
- [ ] 상태 변수 정상 동작 확인

---

## 🔧 STEP 7: 한영 전환 메소드 구현

### 7.1 12가지 한영 전환 방식 정의
```cpp
// 한영 전환 방식 열거형
enum HangulToggleMethod {
    HANGUL_TOGGLE_RIGHT_ALT = 1,      // 오른쪽 Alt
    HANGUL_TOGGLE_ALT_SHIFT = 2,      // Alt + Shift
    HANGUL_TOGGLE_CTRL_SPACE = 3,     // Ctrl + Space
    HANGUL_TOGGLE_SHIFT_SPACE = 4,    // Shift + Space
    HANGUL_TOGGLE_HANGUL_KEY = 5,     // 한/영 키 (0xF2)
    HANGUL_TOGGLE_LEFT_ALT = 6,       // 왼쪽 Alt
    HANGUL_TOGGLE_WIN_SPACE = 7,      // Win + Space
    HANGUL_TOGGLE_LANG1_KEY = 8,      // HID Language 1 (0x90)
    HANGUL_TOGGLE_LANG2_KEY = 9,      // HID Language 2 (0x91)
    HANGUL_TOGGLE_F9_KEY = 10,        // F9 키
    HANGUL_TOGGLE_MENU_KEY = 11,      // Menu 키
    HANGUL_TOGGLE_APPLICATION = 12    // Application 키
};
```

### 7.2 각 방식별 구현 메소드
```cpp
class HangulToggleExecutor {
private:
    KoreanUSBHID* _keyboard;
    int _currentMethod;
    
public:
    HangulToggleExecutor(KoreanUSBHID* keyboard) : _keyboard(keyboard), _currentMethod(1) {}
    
    // 특정 방식 실행
    bool executeMethod(HangulToggleMethod method);
    
    // 모든 방식 순차 테스트
    void testAllMethods();
    
    // 성공한 방식 찾기
    int findWorkingMethod();
    
    // 현재 방식 설정
    void setCurrentMethod(int method) { _currentMethod = method; }
};
```

### 7.3 테스트 체크리스트
- [ ] 각 방식별 HID Report 생성 확인
- [ ] 키 조합 정확성 확인
- [ ] 타이밍 적절성 확인
- [ ] 릴리즈 정상 동작 확인

---

## 🔧 STEP 8: 진단 및 테스트 도구 구현

### 8.1 자동 진단 시스템
```cpp
class HangulDiagnostic {
private:
    struct TestResult {
        int method;
        bool success;
        String response;
        uint32_t responseTime;
    };
    
    TestResult _results[12];
    int _resultCount;
    
public:
    // 전체 진단 실행
    void runFullDiagnostic();
    
    // 각 방식별 테스트
    void testDirectKeycodes();
    void testKeyCombinations();
    void testConsumerControl();
    void testTimingVariations();
    
    // 결과 분석
    void analyzeResults();
    void printReport();
    
    // 권장 방식 제안
    int getRecommendedMethod();
};
```

### 8.2 사용자 인터페이스
```cpp
// 시리얼 명령 인터페이스
class DiagnosticUI {
public:
    void showMainMenu();
    void handleUserInput();
    void runSelectedTest(int testNumber);
    void showResults();
};
```

### 8.3 테스트 체크리스트
- [ ] 진단 도구 정상 실행
- [ ] 각 테스트 결과 정확성 확인
- [ ] 사용자 인터페이스 직관성 확인
- [ ] 로그 데이터 완전성 확인

---

## 🔧 STEP 9: 기존 코드와 통합

### 9.1 BLE 기능 유지
```cpp
// 기존 BLE 코드를 새로운 아키텍처에 통합
class IntegratedSystem {
private:
    KoreanUSBHID* _usbHID;
    BLEManager* _bleManager;
    
public:
    // BLE 명령 처리
    void processBLECommand(String command);
    
    // USB HID 명령 처리
    void processUSBCommand(String command);
    
    // 통합 상태 관리
    void syncStates();
};
```

### 9.2 프로토콜 호환성 유지
```cpp
// 기존 프로토콜 명령 지원
bool processProtocolCommand(const String& line) {
    if (line.equals("#CMD:HANGUL")) {
        return KoreanKeyboard.sendHangulKey();
    }
    // ... 기존 로직 유지
}
```

### 9.3 테스트 체크리스트
- [ ] BLE 연결 정상 동작
- [ ] 기존 프로토콜 명령 호환성 확인
- [ ] 웹 인터페이스 연동 확인
- [ ] iOS 앱 연동 확인

---

## 🔧 STEP 10: 최종 검증 및 최적화

### 10.1 전체 시스템 테스트
```cpp
// 통합 테스트 시나리오
class SystemIntegrationTest {
public:
    // 기본 기능 테스트
    bool testBasicFunctionality();
    
    // 한영 전환 테스트
    bool testHangulToggle();
    
    // 성능 테스트
    bool testPerformance();
    
    // 안정성 테스트
    bool testStability();
    
    // 호환성 테스트
    bool testCompatibility();
};
```

### 10.2 성능 최적화
- [ ] 메모리 사용량 최적화
- [ ] 응답 시간 최적화
- [ ] 전력 소모 최적화
- [ ] 안정성 향상

### 10.3 최종 체크리스트
- [ ] 모든 기능 정상 동작
- [ ] 성능 요구사항 만족
- [ ] 메모리 사용량 적절
- [ ] 장시간 안정성 확인

---

## 📊 진행 상황 체크리스트

### 전체 진행률: 0/10 완료

- [ ] **STEP 1**: 백업 및 분석 (0%)
- [ ] **STEP 2**: 환경 준비 (0%)
- [ ] **STEP 3**: USB Descriptor 기본 (0%)
- [ ] **STEP 4**: Device Descriptor (0%)
- [ ] **STEP 5**: Configuration Descriptor (0%)
- [ ] **STEP 6**: TinyUSB 클래스 (0%)
- [ ] **STEP 7**: 한영 전환 구현 (0%)
- [ ] **STEP 8**: 진단 도구 (0%)
- [ ] **STEP 9**: 기존 코드 통합 (0%)
- [ ] **STEP 10**: 최종 검증 (0%)

---

## 🚨 중요 주의사항

### 안전장치
1. **매 단계마다 백업** 생성
2. **테스트 후 진행** 원칙
3. **문제 발생 시 즉시 롤백** 계획
4. **디버깅 로그 상세히** 기록

### 예상 위험점
1. **USB 드라이버 충돌** 가능성
2. **Windows 재부팅** 필요할 수 있음
3. **기존 BLE 기능 중단** 위험
4. **메모리 부족** 발생 가능

### 대응 방안
1. **가상 머신 테스트** 우선
2. **점진적 기능 추가**
3. **백업 복원 절차** 숙지
4. **문제 발생 시 연락** 즉시

---

## 🎯 다음 단계

**STEP 1부터 시작하겠습니다.**

준비되셨으면 "STEP 1 시작"이라고 말씀해 주세요. 
각 단계를 완료한 후 다음 단계로 넘어가겠습니다.

**절대 서두르지 않고, 꼼꼼하게 진행하겠습니다!**