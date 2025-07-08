/**
 * @file config.h
 * @brief GHOSTYPE 시스템 설정 및 상수 정의
 * @version 1.0
 * @date 2024-12-28
 * 
 * 이 파일은 GHOSTYPE 시스템의 모든 설정값과 상수를 중앙에서 관리합니다.
 * 타이핑 속도, BLE 설정, 하드웨어 핀 설정 등을 포함합니다.
 */

#pragma once

// ============================================================================
// 시스템 정보
// ============================================================================
#define FIRMWARE_VERSION "2.0.0"
#define FIRMWARE_NAME "GHOSTYPE"

// ============================================================================
// 하드웨어 설정
// ============================================================================
#define LED_STATUS_PIN 2          // 상태 표시용 LED 핀 (GPIO2)
#define WATCHDOG_TIMEOUT_MS 8000  // 와치독 타임아웃 (8초)

// ============================================================================
// BLE 설정
// ============================================================================
#define BLE_DEVICE_NAME "GHOSTYPE"
#define BLE_SERVICE_UUID "12345678-1234-5678-9012-123456789abc"
#define BLE_CHAR_RX_UUID "12345678-1234-5678-9012-123456789abd"  // 클라이언트 → 서버
#define BLE_CHAR_TX_UUID "12345678-1234-5678-9012-123456789abe"  // 서버 → 클라이언트

// BLE 연결 파라미터
#define BLE_MIN_CONN_INTERVAL 0x06   // 7.5ms
#define BLE_MAX_CONN_INTERVAL 0x12   // 22.5ms
#define BLE_TIMEOUT_MULTIPLIER 0x33  // 510ms

// ============================================================================
// 타이핑 설정
// ============================================================================
// 기본 타이핑 속도 (CPS: Characters Per Second)
#define DEFAULT_TYPING_SPEED_CPS 6
#define MIN_TYPING_SPEED_CPS 1
#define MAX_TYPING_SPEED_CPS 50

// 키 입력 타이밍 (밀리초)
#define KEY_PRESS_DURATION_MS 30     // 키를 누르는 시간
#define KEY_RELEASE_DURATION_MS 20   // 키를 놓는 시간
#define SHIFT_HOLD_DURATION_MS 20    // Shift 키 홀드 시간

// 타이핑 간격 설정
#define DEFAULT_INTERVAL_MS 100      // 기본 간격 지연
#define DEFAULT_INTERVAL_CHARS 5     // 간격 지연을 적용할 문자 수

// ============================================================================
// 프로토콜 설정
// ============================================================================
// 명령어 프로토콜
#define PROTOCOL_PREFIX "GHTYPE_"
#define PROTOCOL_JSON_START '{'
#define PROTOCOL_JSON_END '}'

// 토글 마커
#define TOGGLE_MARKER "⌨HANGUL_TOGGLE⌨"
#define TOGGLE_MARKER_LENGTH 15

// JSON 필드명
#define JSON_FIELD_TEXT "text"
#define JSON_FIELD_SPEED "speed_cps"
#define JSON_FIELD_INTERVAL "interval_ms"

// ============================================================================
// 메모리 및 버퍼 설정
// ============================================================================
#define MAX_MESSAGE_LENGTH 512       // 최대 메시지 길이
#define MAX_TEXT_CHUNK_SIZE 256      // 텍스트 청크 최대 크기
#define TYPING_BUFFER_SIZE 1024      // 타이핑 버퍼 크기

// ============================================================================
// 타임아웃 설정
// ============================================================================
#define CONNECTION_TIMEOUT_MS 30000  // 연결 타임아웃 (30초)
#define TYPING_TIMEOUT_MS 300000     // 타이핑 타임아웃 (5분)
#define HEARTBEAT_INTERVAL_MS 10000  // 하트비트 간격 (10초)

// ============================================================================
// 특수 키 정의
// ============================================================================
#define ASCII_PRINTABLE_START 0x20   // 출력 가능한 ASCII 시작
#define ASCII_PRINTABLE_END 0x7E     // 출력 가능한 ASCII 끝

// 제어 문자
#define CHAR_NEWLINE '\n'
#define CHAR_TAB '\t'
#define CHAR_CARRIAGE_RETURN '\r'
#define CHAR_SPACE ' '

// ============================================================================
// 시스템 상태 정의
// ============================================================================
enum SystemState {
    SYSTEM_INITIALIZING = 0,      // 시스템 초기화 중
    SYSTEM_READY,                 // 시스템 준비 완료
    SYSTEM_CONNECTED,             // BLE 연결됨
    SYSTEM_TYPING,                // 타이핑 중
    SYSTEM_ERROR                  // 오류 상태
};

// 타이핑 모드 정의
enum TypingMode {
    TYPING_MODE_NORMAL = 0,       // 일반 타이핑
    TYPING_MODE_FAST,             // 빠른 타이핑
    TYPING_MODE_CAREFUL           // 신중한 타이핑 (안전성 우선)
};

// ============================================================================
// 오류 코드 정의
// ============================================================================
enum ErrorCode {
    ERROR_NONE = 0,               // 오류 없음
    ERROR_BLE_INIT_FAILED,        // BLE 초기화 실패
    ERROR_HID_INIT_FAILED,        // HID 초기화 실패
    ERROR_INVALID_MESSAGE,        // 잘못된 메시지
    ERROR_TYPING_TIMEOUT,         // 타이핑 타임아웃
    ERROR_MEMORY_ALLOCATION,      // 메모리 할당 실패
    ERROR_UNKNOWN                 // 알 수 없는 오류
};

// ============================================================================
// 유틸리티 매크로
// ============================================================================
// 배열 크기 계산
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

// 최소/최대값 계산
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// 값 범위 제한
#define CLAMP(value, min_val, max_val) (MIN(MAX((value), (min_val)), (max_val)))

// 비트 조작
#define SET_BIT(reg, bit) ((reg) |= (1 << (bit)))
#define CLEAR_BIT(reg, bit) ((reg) &= ~(1 << (bit)))
#define TOGGLE_BIT(reg, bit) ((reg) ^= (1 << (bit)))
#define CHECK_BIT(reg, bit) (((reg) >> (bit)) & 1)

// ============================================================================
// 디버그 설정 (프로덕션에서는 비활성화)
// ============================================================================
#ifdef DEBUG_MODE
    #define DEBUG_PRINT(x) Serial.print(x)
    #define DEBUG_PRINTLN(x) Serial.println(x)
    #define DEBUG_PRINTF(fmt, ...) Serial.printf(fmt, __VA_ARGS__)
#else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTLN(x)
    #define DEBUG_PRINTF(fmt, ...)
#endif