/**
 * @file main.cpp
 * @brief GHOSTYPE 펌웨어 메인 엔트리 포인트
 * @version 2.0.0
 * @date 2024-12-28
 * 
 * 이 파일은 GHOSTYPE 시스템의 메인 엔트리 포인트입니다.
 * BLE 통신을 통해 수신한 타이핑 명령을 HID 키보드로 실행합니다.
 * 
 * 주요 기능:
 * - BLE 서버 운영
 * - 타이핑 명령 파싱 및 실행
 * - USB HID 키보드 에뮬레이션
 * - 한영 토글 처리
 */

#include <Arduino.h>
#include <ArduinoJson.h>

// GHOSTYPE 모듈들
#include "config.h"
#include "ble_manager.h"
#include "parser.h"
#include "typing_handler.h"
#include "hid_utils.h"

// ============================================================================
// 전역 변수 및 상태 관리
// ============================================================================

/** @brief 현재 시스템 상태 */
static SystemState current_system_state = SYSTEM_INITIALIZING;

/** @brief 상태 표시 LED 제어를 위한 타이머 */
static unsigned long last_led_update = 0;
static bool led_state = false;

/** @brief 성능 모니터링을 위한 변수들 */
static unsigned long last_performance_check = 0;
static uint32_t loop_counter = 0;
static uint32_t successful_commands = 0;
static uint32_t failed_commands = 0;

// ============================================================================
// 함수 선언
// ============================================================================

/**
 * @brief 시스템 전체 초기화
 * @return true 성공, false 실패
 * 
 * 모든 하위 시스템을 순차적으로 초기화합니다.
 */
bool initializeSystem();

/**
 * @brief 시스템 전체 종료
 * 
 * 모든 하위 시스템을 안전하게 종료하고 리소스를 정리합니다.
 */
void deinitializeSystem();

/**
 * @brief 수신된 BLE 데이터 처리
 * 
 * BLE를 통해 수신된 데이터를 파싱하고 타이핑 명령으로 실행합니다.
 */
void processBLEData();

/**
 * @brief 시스템 상태 업데이트
 * @param new_state 새로운 시스템 상태
 * 
 * 시스템 상태를 변경하고 관련 하드웨어 제어를 수행합니다.
 */
void updateSystemState(SystemState new_state);

/**
 * @brief 상태 표시 LED 제어
 * 
 * 현재 시스템 상태에 따라 LED 패턴을 제어합니다.
 */
void updateStatusLED();

/**
 * @brief 시스템 성능 모니터링
 * 
 * 메모리 사용량, 루프 성능 등을 주기적으로 확인합니다.
 */
void monitorSystemPerformance();

/**
 * @brief 와치독 피드 및 시스템 안정성 검사
 * 
 * 시스템이 정상적으로 동작하는지 확인하고 와치독을 리셋합니다.
 */
void feedWatchdog();

// ============================================================================
// 메인 함수들
// ============================================================================

/**
 * @brief 시스템 초기화 (Arduino setup 함수)
 * 
 * ESP32 부팅 후 한 번만 실행됩니다.
 * 모든 하위 시스템을 초기화하고 준비 상태로 만듭니다.
 */
void setup() {
    // 시스템 상태를 초기화 중으로 설정
    current_system_state = SYSTEM_INITIALIZING;
    
    // 상태 표시 LED 핀 초기화
    pinMode(LED_STATUS_PIN, OUTPUT);
    digitalWrite(LED_STATUS_PIN, LOW);
    
    // 시스템 안정화를 위한 초기 지연
    delay(1000);
    
    // 전체 시스템 초기화 시도
    if (initializeSystem()) {
        // 초기화 성공 - 준비 상태로 전환
        updateSystemState(SYSTEM_READY);
        
        // 초기화 성공 신호 (LED 3회 깜빡임)
        for (int i = 0; i < 3; i++) {
            digitalWrite(LED_STATUS_PIN, HIGH);
            delay(200);
            digitalWrite(LED_STATUS_PIN, LOW);
            delay(200);
        }
    } else {
        // 초기화 실패 - 오류 상태로 전환
        updateSystemState(SYSTEM_ERROR);
        
        // 초기화 실패 신호 (LED 빠른 깜빡임)
        for (int i = 0; i < 10; i++) {
            digitalWrite(LED_STATUS_PIN, HIGH);
            delay(100);
            digitalWrite(LED_STATUS_PIN, LOW);
            delay(100);
        }
    }
}

/**
 * @brief 메인 루프 (Arduino loop 함수)
 * 
 * 시스템이 동작하는 동안 지속적으로 실행됩니다.
 * BLE 데이터 처리, 상태 관리, 성능 모니터링을 수행합니다.
 */
void loop() {
    loop_counter++;
    
    // 시스템이 오류 상태인 경우 기본 동작만 수행
    if (current_system_state == SYSTEM_ERROR) {
        updateStatusLED();
        delay(1000);
        return;
    }
    
    // BLE 연결 상태에 따른 시스템 상태 업데이트
    BLEConnectionState ble_state = BLEManager::getConnectionState();
    switch (ble_state) {
        case BLE_STATE_CONNECTED:
            if (current_system_state != SYSTEM_CONNECTED && current_system_state != SYSTEM_TYPING) {
                updateSystemState(SYSTEM_CONNECTED);
            }
            break;
            
        case BLE_STATE_ADVERTISING:
            if (current_system_state == SYSTEM_CONNECTED || current_system_state == SYSTEM_TYPING) {
                updateSystemState(SYSTEM_READY);
            }
            break;
            
        case BLE_STATE_ERROR:
            updateSystemState(SYSTEM_ERROR);
            break;
            
        default:
            break;
    }
    
    // BLE 데이터 처리 (연결된 상태에서만)
    if (current_system_state == SYSTEM_CONNECTED || current_system_state == SYSTEM_TYPING) {
        processBLEData();
    }
    
    // 상태 표시 LED 업데이트
    updateStatusLED();
    
    // 주기적 시스템 모니터링
    monitorSystemPerformance();
    
    // 와치독 피드
    feedWatchdog();
    
    // CPU 부하 조절을 위한 짧은 지연
    delay(10);
}

// ============================================================================
// 시스템 초기화 및 종료
// ============================================================================

bool initializeSystem() {
    bool success = true;
    
    // 1. Parser 모듈 초기화
    if (!Parser::initialize()) {
        success = false;
    }
    
    // 2. HID 유틸리티 초기화
    if (success && !HIDUtils::initialize()) {
        success = false;
    }
    
    // 3. 타이핑 핸들러 초기화
    if (success && !TypingHandler::initialize()) {
        success = false;
    }
    
    // 4. BLE 매니저 초기화 (마지막에 수행)
    if (success && !BLEManager::initialize()) {
        success = false;
    }
    
    // 초기화 실패 시 부분적으로 초기화된 모듈들 정리
    if (!success) {
        deinitializeSystem();
    }
    
    return success;
}

void deinitializeSystem() {
    // 역순으로 모듈들 종료
    BLEManager::deinitialize();
    TypingHandler::deinitialize();
    HIDUtils::deinitialize();
    Parser::deinitialize();
}

// ============================================================================
// BLE 데이터 처리
// ============================================================================

void processBLEData() {
    // 수신된 데이터가 있는지 확인
    if (!BLEManager::hasReceivedData()) {
        return;
    }
    
    // 타이핑 중인 경우 새로운 명령 무시 (안전성)
    if (current_system_state == SYSTEM_TYPING) {
        // 수신된 데이터는 버리고 에러 응답 전송
        BLEReceivedData data = BLEManager::getReceivedData();
        if (data.valid && data.data) {
            delete[] data.data;
        }
        BLEManager::sendResponse("ERR:BUSY");
        return;
    }
    
    // 수신된 데이터 가져오기
    BLEReceivedData received_data = BLEManager::getReceivedData();
    
    if (!received_data.valid || !received_data.data) {
        BLEManager::sendResponse("ERR:INVALID_DATA");
        failed_commands++;
        return;
    }
    
    // 타이핑 상태로 전환
    updateSystemState(SYSTEM_TYPING);
    
    // 데이터 파싱
    TypingCommand command = Parser::parseMessage(received_data.data, received_data.length);
    
    // 수신 데이터 메모리 해제
    delete[] received_data.data;
    
    String response;
    
    if (command.valid) {
        // 유효한 명령인 경우 타이핑 실행
        TypingResult result = TypingHandler::executeCommand(command);
        
        if (result.success) {
            // 성공 응답 생성
            response = Parser::generateResponse(result.chars_typed, true);
            successful_commands++;
        } else {
            // 실패 응답 생성
            response = Parser::generateResponse(result.chars_typed, false);
            failed_commands++;
        }
    } else {
        // 무효한 명령인 경우
        response = "ERR:INVALID_COMMAND";
        failed_commands++;
    }
    
    // 응답 전송
    BLEManager::sendResponse(response);
    
    // 연결 상태로 복귀
    updateSystemState(SYSTEM_CONNECTED);
}

// ============================================================================
// 시스템 상태 및 하드웨어 제어
// ============================================================================

void updateSystemState(SystemState new_state) {
    if (current_system_state != new_state) {
        current_system_state = new_state;
        
        // 상태 변경에 따른 추가 처리
        switch (new_state) {
            case SYSTEM_READY:
                // 준비 상태 - 안전 모드 활성화
                TypingHandler::setSafeMode(true);
                break;
                
            case SYSTEM_CONNECTED:
                // 연결 상태 - 일반 모드로 전환
                TypingHandler::setSafeMode(false);
                break;
                
            case SYSTEM_TYPING:
                // 타이핑 중 - 신중한 모드로 전환
                TypingHandler::setTypingMode(TYPING_MODE_CAREFUL);
                break;
                
            case SYSTEM_ERROR:
                // 오류 상태 - 모든 타이핑 중단
                TypingHandler::abortTyping();
                break;
                
            default:
                break;
        }
    }
}

void updateStatusLED() {
    unsigned long current_time = millis();
    
    switch (current_system_state) {
        case SYSTEM_INITIALIZING:
            // 초기화 중 - 빠른 깜빡임
            if (current_time - last_led_update > 100) {
                led_state = !led_state;
                digitalWrite(LED_STATUS_PIN, led_state);
                last_led_update = current_time;
            }
            break;
            
        case SYSTEM_READY:
            // 준비 상태 - 천천히 깜빡임
            if (current_time - last_led_update > 1000) {
                led_state = !led_state;
                digitalWrite(LED_STATUS_PIN, led_state);
                last_led_update = current_time;
            }
            break;
            
        case SYSTEM_CONNECTED:
            // 연결됨 - 계속 켜짐
            digitalWrite(LED_STATUS_PIN, HIGH);
            break;
            
        case SYSTEM_TYPING:
            // 타이핑 중 - 매우 빠른 깜빡임
            if (current_time - last_led_update > 50) {
                led_state = !led_state;
                digitalWrite(LED_STATUS_PIN, led_state);
                last_led_update = current_time;
            }
            break;
            
        case SYSTEM_ERROR:
            // 오류 상태 - SOS 패턴 (3짧음-3긺음-3짧음)
            // 구현 복잡도로 인해 단순 빠른 깜빡임으로 대체
            if (current_time - last_led_update > 200) {
                led_state = !led_state;
                digitalWrite(LED_STATUS_PIN, led_state);
                last_led_update = current_time;
            }
            break;
    }
}

// ============================================================================
// 시스템 모니터링 및 유지보수
// ============================================================================

void monitorSystemPerformance() {
    unsigned long current_time = millis();
    
    // 10초마다 성능 체크 수행
    if (current_time - last_performance_check < 10000) {
        return;
    }
    
    last_performance_check = current_time;
    
    // 메모리 사용량 확인
    size_t free_heap = ESP.getFreeHeap();
    if (free_heap < 50000) {  // 50KB 미만인 경우 경고
        // 메모리 부족 상황 - 안전 모드 강화
        TypingHandler::setSafeMode(true);
        TypingHandler::setTypingMode(TYPING_MODE_CAREFUL);
    }
    
    // BLE 시스템 건강성 확인
    if (!BLEManager::isSystemHealthy()) {
        // BLE 시스템 문제 감지 - 재시작 시도
        if (current_system_state != SYSTEM_ERROR) {
            BLEManager::restartAdvertising();
        }
    }
    
    // 통계 정보 업데이트 (필요시 사용)
    uint32_t bytes_received, bytes_sent, connection_count;
    BLEManager::getStatistics(bytes_received, bytes_sent, connection_count);
    
    // 성능 카운터 리셋
    loop_counter = 0;
}

void feedWatchdog() {
    // ESP32 내장 와치독은 자동으로 관리되지만
    // yield() 호출로 시스템 안정성 확보
    yield();
}