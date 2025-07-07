// src/BLEConfig.h
// GHOSTYPE 상품화 버전 - 시스템 설정 상수 (T-Dongle-S3 최적화)

#ifndef BLE_CONFIG_H
#define BLE_CONFIG_H

// 제품 정보
#define PRODUCT_NAME "GHOSTYPE"
#define PRODUCT_VERSION "2.1.0"
#define MANUFACTURER_NAME "GHOSTYPE Inc."
#define HARDWARE_VERSION "T-Dongle-S3"

// BLE 디바이스 설정
#define DEVICE_NAME "GHOSTYPE-" // 뒤에 MAC 주소 마지막 4자리 추가됨
#define MAX_CONNECTED_DEVICES 3  // 최대 3개 디바이스 동시 연결

// BLE 서비스 및 특성 UUID (Nordic UART Service)
#define SERVICE_UUID        "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"  // Write
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"  // Notify

// Generic Access Service (필수)
#define GAP_SERVICE_UUID        "1800"
#define DEVICE_NAME_CHAR_UUID   "2A00"
#define APPEARANCE_CHAR_UUID    "2A01"

// Generic Attribute Service (필수)
#define GATT_SERVICE_UUID       "1801"

// Device Information Service (권장)
#define DIS_SERVICE_UUID        "180A"
#define MANUFACTURER_CHAR_UUID  "2A29"
#define MODEL_CHAR_UUID         "2A24"
#define SERIAL_CHAR_UUID        "2A25"
#define FIRMWARE_CHAR_UUID      "2A26"
#define HARDWARE_CHAR_UUID      "2A27"
#define SOFTWARE_CHAR_UUID      "2A28"

// Battery Service (선택)
#define BATTERY_SERVICE_UUID    "180F"
#define BATTERY_LEVEL_CHAR_UUID "2A19"

// HID 서비스 UUID (향후 구현용)
#define HID_SERVICE_UUID        "1812"
#define HID_REPORT_MAP_UUID     "2A4B"
#define HID_REPORT_UUID         "2A4D"
#define HID_CONTROL_POINT_UUID  "2A4C"
#define HID_INFORMATION_UUID    "2A4A"

// NimBLE 연결 파라미터 (Windows/iOS/Android 호환)
#define BLE_MIN_INTERVAL 12      // 15ms (12 * 1.25ms)
#define BLE_MAX_INTERVAL 24      // 30ms (24 * 1.25ms)
#define BLE_LATENCY      0       // 레이턴시 0
#define BLE_TIMEOUT      400     // 4초 (400 * 10ms)

// 광고 파라미터 (모든 플랫폼 호환)
#define BLE_ADV_MIN_INTERVAL 32  // 20ms (32 * 0.625ms)
#define BLE_ADV_MAX_INTERVAL 244 // 152.5ms (244 * 0.625ms)
#define BLE_SCAN_INTERVAL 80     // 50ms
#define BLE_SCAN_WINDOW   80     // 50ms

// MTU 설정
#define BLE_MTU_SIZE 247         // Windows/Android 기본값
#define BLE_MIN_MTU  23          // BLE 최소 MTU
#define BLE_MAX_MTU  517         // BLE 최대 MTU

// 버퍼 및 큐 크기
#define RX_BUFFER_SIZE 512       // 수신 버퍼 (한글 고려)
#define TX_BUFFER_SIZE 512       // 송신 버퍼
#define HID_QUEUE_SIZE 100       // HID 키 입력 큐
#define MAX_TEXT_LENGTH 256      // 최대 텍스트 길이

// 플래시 저장소 설정 (페어링 정보)
#define NVS_NAMESPACE "ghostype"
#define NVS_PAIRED_DEVICES_KEY "paired_dev"
#define MAX_PAIRED_DEVICES 10    // 최대 저장 페어링 정보

// LED 설정 (T-Dongle-S3)
#ifndef RGB_LED_PIN
#define RGB_LED_PIN 40           // WS2812B RGB LED
#endif
#define LED_BRIGHTNESS 50        // LED 밝기 (0-255)

// 상태 LED 색상 정의 (RGB)
#define COLOR_OFF         0x000000  // 꺼짐
#define COLOR_IDLE        0x0000FF  // 파란색 - 대기
#define COLOR_CONNECTED   0x00FF00  // 초록색 - 연결됨
#define COLOR_TYPING      0x00FFFF  // 하늘색 - 타이핑 중
#define COLOR_ERROR       0xFF0000  // 빨간색 - 에러
#define COLOR_PAIRING     0xFF00FF  // 보라색 - 페어링 모드

// USB HID 설정
#define USB_VID 0x303A           // Espressif VID
#define USB_PID 0x8000           // Custom PID
#define HID_REPORT_SIZE 8        // HID 리포트 크기

// 시스템 타이밍 (밀리초)
#define LED_UPDATE_INTERVAL 50   // LED 업데이트 주기
#define STATUS_UPDATE_INTERVAL 10000  // 상태 출력 주기
#define RECONNECT_DELAY 2000     // 재연결 대기 시간
#define TYPING_LED_DURATION 500  // 타이핑 LED 지속 시간
#define CONNECTION_TIMEOUT 10000 // 연결 타임아웃

// 보안 설정 (플랫폼별 호환성)
#define USE_BONDING false        // 보안 비활성화
#define USE_MITM_PROTECTION false
#define USE_SECURE_CONNECTION false
#define FIXED_PASSKEY -1         // 패스키 미사용

// 디버그 설정
#define DEBUG_BLE 1              // BLE 디버그 출력
#define DEBUG_HID 1              // HID 디버그 출력
#define DEBUG_VERBOSE 0          // 상세 디버그 출력

// 전원 관리
#define USE_LIGHT_SLEEP false    // 라이트 슬립 사용 안함
#define CPU_FREQ_MHZ 240         // CPU 주파수 (240MHz)
#define BLE_TX_POWER ESP_PWR_LVL_P9  // +9dBm (최대 출력)

// 플랫폼별 특수 설정
#define WINDOWS_COMPATIBILITY true   // Windows 호환 모드
#define IOS_COMPATIBILITY true       // iOS 호환 모드
#define ANDROID_COMPATIBILITY true   // Android 호환 모드

// BLE Appearance 정의
#define BLE_APPEARANCE_KEYBOARD 0x03C1      // HID Keyboard
#define BLE_APPEARANCE_GENERIC_HID 0x03C0   // Generic HID
#define COMPANY_ID_GHOSTYPE 0x4847          // "GH" in hex

#endif // BLE_CONFIG_H