/**
 * @file typing_handler.cpp
 * @brief 타이핑 실행 및 제어 모듈 구현
 * @version 1.0
 * @date 2024-12-28
 */

#include "typing_handler.h"

// 정적 멤버 변수 초기화
bool TypingHandler::initialized = false;
TypingState TypingHandler::current_state = {};
TypingMode TypingHandler::typing_mode = TYPING_MODE_NORMAL;
bool TypingHandler::safe_mode = true;

bool TypingHandler::initialize() {
    if (initialized) {
        return true;
    }

    // HID 시스템 초기화
    if (!HIDUtils::initialize()) {
        return false;
    }

    // 타이핑 상태 초기화
    resetState();
    
    // 기본 설정
    typing_mode = TYPING_MODE_NORMAL;
    safe_mode = true;

    initialized = true;
    return true;
}

void TypingHandler::deinitialize() {
    if (!initialized) {
        return;
    }

    // 진행 중인 타이핑 중단
    abortTyping();

    // HID 시스템 종료
    HIDUtils::deinitialize();

    // 상태 리셋
    resetState();
    initialized = false;
}

TypingResult TypingHandler::executeCommand(const TypingCommand& command) {
    uint32_t start_time = millis();
    
    // 초기화 확인
    if (!initialized) {
        return createErrorResult(ERROR_BLE_INIT_FAILED, 0, millis() - start_time);
    }

    // 명령 유효성 검사
    if (!command.valid) {
        return createErrorResult(ERROR_INVALID_MESSAGE, 0, millis() - start_time);
    }

    // 안전 모드에서 추가 검사
    if (safe_mode && !performSafetyCheck(command)) {
        return createErrorResult(ERROR_INVALID_MESSAGE, 0, millis() - start_time);
    }

    // HID 연결 상태 확인
    if (!HIDUtils::isConnected()) {
        return createErrorResult(ERROR_HID_INIT_FAILED, 0, millis() - start_time);
    }

    // 타이핑 상태 초기화
    initializeState(command.text.length());

    TypingResult result;

    // 토글 마커 포함 여부에 따른 처리 분기
    if (command.has_toggle) {
        result = executeWithToggle(command);
    } else {
        result = executeNormalText(command);
    }

    // 실행 시간 계산
    result.execution_time = millis() - start_time;

    // 타이핑 상태 리셋
    resetState();

    return result;
}

TypingResult TypingHandler::executeWithToggle(const TypingCommand& command) {
    const size_t MAX_CHUNKS = 10;
    TextChunk chunks[MAX_CHUNKS];
    
    // 토글 기준으로 텍스트 분할
    size_t chunk_count = Parser::splitTextByToggle(command.text, chunks, MAX_CHUNKS);
    
    if (chunk_count == 0) {
        return createErrorResult(ERROR_INVALID_MESSAGE, 0, 0);
    }

    size_t total_chars_typed = 0;
    size_t total_chars_processed = 0;

    // 각 청크 순차 처리
    for (size_t i = 0; i < chunk_count; i++) {
        // 타임아웃 검사
        if (isTimedOut()) {
            return createErrorResult(ERROR_TYPING_TIMEOUT, total_chars_typed, 0);
        }

        const TextChunk& chunk = chunks[i];
        
        // 토글 키 전송 (첫 번째 청크가 아니고 토글이 필요한 경우)
        if (chunk.has_toggle_before) {
            if (!sendToggleKey()) {
                // 토글 키 전송 실패 시 계속 진행 (치명적이지 않음)
            }
            
            // 토글 키 전송 후 안정화 대기
            HIDUtils::safeDelay(100);
        }

        // 현재 청크 타이핑
        size_t chunk_chars = executeChunk(chunk, command.speed_cps, command.interval_ms);
        
        total_chars_typed += chunk_chars;
        total_chars_processed += chunk.content.length();
        
        // 타이핑 상태 업데이트
        updateState(total_chars_processed);

        // 청크 간 짧은 대기 (자연스러운 타이핑을 위해)
        if (!chunk.is_last) {
            uint32_t inter_chunk_delay = adjustDelayForMode(50);
            HIDUtils::safeDelay(inter_chunk_delay);
        }
    }

    TypingResult result = {
        .chars_processed = total_chars_processed,
        .chars_typed = total_chars_typed,
        .execution_time = 0,  // 호출한 곳에서 설정
        .success = (total_chars_typed > 0),
        .error_code = ERROR_NONE
    };

    return result;
}

TypingResult TypingHandler::executeNormalText(const TypingCommand& command) {
    // 단일 청크로 처리
    TextChunk single_chunk = {
        .content = command.text,
        .position = 0,
        .is_last = true,
        .has_toggle_before = false,
        .has_toggle_after = false
    };

    size_t chars_typed = executeChunk(single_chunk, command.speed_cps, command.interval_ms);

    TypingResult result = {
        .chars_processed = command.text.length(),
        .chars_typed = chars_typed,
        .execution_time = 0,  // 호출한 곳에서 설정
        .success = (chars_typed > 0),
        .error_code = ERROR_NONE
    };

    return result;
}

bool TypingHandler::sendToggleKey() {
    if (!initialized || !HIDUtils::isConnected()) {
        return false;
    }

    // 한영 전환 키 조합 전송 (Alt + 한/영)
    // 시스템에 따라 다를 수 있으므로 가장 호환성 높은 방식 사용
    
    // 방법 1: Right Alt + Space (가장 호환성 높음)
    bool result = HIDUtils::typeSpecialKey(KEY_RIGHT_ALT);
    HIDUtils::safeDelay(50);
    
    return result;
}

TypingState TypingHandler::getCurrentState() {
    return current_state;
}

bool TypingHandler::abortTyping() {
    if (!current_state.is_active) {
        return true;  // 이미 비활성 상태
    }

    // 모든 키 해제
    HIDUtils::releaseAllKeys();
    
    // 상태 리셋
    resetState();
    
    return true;
}

void TypingHandler::setTypingMode(TypingMode mode) {
    typing_mode = mode;
}

void TypingHandler::setSafeMode(bool enabled) {
    safe_mode = enabled;
}

uint8_t TypingHandler::getProgress() {
    if (!current_state.is_active || current_state.total_length == 0) {
        return 0;
    }

    uint32_t progress = (current_state.current_position * 100) / current_state.total_length;
    return (uint8_t)MIN(progress, 100);
}

uint32_t TypingHandler::estimateCompletionTime(const TypingCommand& command) {
    if (!command.valid || command.text.length() == 0) {
        return 0;
    }

    // 기본 타이핑 시간 계산
    uint32_t base_time = (command.text.length() * 1000) / command.speed_cps;
    
    // 토글 처리 시간 추가
    if (command.has_toggle) {
        size_t toggle_count = 0;
        int pos = 0;
        while ((pos = command.text.indexOf(TOGGLE_MARKER, pos)) >= 0) {
            toggle_count++;
            pos += TOGGLE_MARKER_LENGTH;
        }
        
        // 토글당 200ms 추가
        base_time += toggle_count * 200;
    }

    // 간격 지연 시간 추가
    size_t interval_count = command.text.length() / DEFAULT_INTERVAL_CHARS;
    base_time += interval_count * command.interval_ms;

    // 타이핑 모드에 따른 조정
    base_time = adjustDelayForMode(base_time);

    return base_time;
}

void TypingHandler::initializeState(size_t text_length) {
    current_state.is_active = true;
    current_state.start_time = millis();
    current_state.current_position = 0;
    current_state.total_length = text_length;
    current_state.mode = typing_mode;
}

void TypingHandler::updateState(size_t chars_processed) {
    if (current_state.is_active) {
        current_state.current_position = chars_processed;
    }
}

void TypingHandler::resetState() {
    current_state.is_active = false;
    current_state.start_time = 0;
    current_state.current_position = 0;
    current_state.total_length = 0;
    current_state.mode = TYPING_MODE_NORMAL;
}

bool TypingHandler::isTimedOut() {
    if (!current_state.is_active) {
        return false;
    }

    uint32_t elapsed = millis() - current_state.start_time;
    return (elapsed > TYPING_TIMEOUT_MS);
}

bool TypingHandler::performSafetyCheck(const TypingCommand& command) {
    // 텍스트 길이 검사
    if (command.text.length() > MAX_TEXT_CHUNK_SIZE) {
        return false;
    }

    // 타이핑 속도 검사
    if (command.speed_cps > MAX_TYPING_SPEED_CPS) {
        return false;
    }

    // 예상 실행 시간 검사 (최대 10분)
    uint32_t estimated_time = estimateCompletionTime(command);
    if (estimated_time > 600000) {  // 10분
        return false;
    }

    return true;
}

uint32_t TypingHandler::adjustDelayForMode(uint32_t base_delay) {
    switch (typing_mode) {
        case TYPING_MODE_FAST:
            return base_delay * 80 / 100;  // 20% 빠르게
            
        case TYPING_MODE_CAREFUL:
            return base_delay * 150 / 100; // 50% 느리게
            
        case TYPING_MODE_NORMAL:
        default:
            return base_delay;
    }
}

size_t TypingHandler::executeChunk(const TextChunk& chunk, uint8_t speed_cps, uint16_t interval_ms) {
    if (chunk.content.length() == 0) {
        return 0;
    }

    // HID를 통한 문자열 타이핑
    return HIDUtils::typeString(
        chunk.content.c_str(),
        speed_cps,
        DEFAULT_INTERVAL_CHARS,
        interval_ms
    );
}

TypingResult TypingHandler::createErrorResult(ErrorCode error_code, size_t chars_typed, uint32_t execution_time) {
    TypingResult result = {
        .chars_processed = 0,
        .chars_typed = chars_typed,
        .execution_time = execution_time,
        .success = false,
        .error_code = error_code
    };
    
    return result;
}