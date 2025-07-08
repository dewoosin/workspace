/**
 * @file parser.cpp
 * @brief 텍스트 파싱 및 명령어 감지 모듈 구현
 * @version 1.0
 * @date 2024-12-28
 */

#include "parser.h"

// 정적 멤버 변수 초기화
bool Parser::initialized = false;
DynamicJsonDocument* Parser::json_document = nullptr;

bool Parser::initialize() {
    if (initialized) {
        return true;
    }

    // JSON 파서 초기화 (1KB 버퍼)
    json_document = new DynamicJsonDocument(1024);
    if (json_document == nullptr) {
        return false;
    }

    initialized = true;
    return true;
}

void Parser::deinitialize() {
    if (!initialized) {
        return;
    }

    // JSON 문서 메모리 해제
    if (json_document != nullptr) {
        delete json_document;
        json_document = nullptr;
    }

    initialized = false;
}

TypingCommand Parser::parseMessage(const char* raw_data, size_t data_length) {
    TypingCommand result = {
        .text = "",
        .speed_cps = DEFAULT_TYPING_SPEED_CPS,
        .interval_ms = DEFAULT_INTERVAL_MS,
        .has_toggle = false,
        .valid = false
    };

    // 입력 데이터 유효성 검사
    if (raw_data == nullptr || data_length == 0 || data_length > MAX_MESSAGE_LENGTH) {
        return result;
    }

    // 안전한 문자열 생성 (null 종료 보장)
    String message;
    message.reserve(data_length + 1);
    for (size_t i = 0; i < data_length; i++) {
        message += raw_data[i];
    }

    // 메시지 유형 감지 및 적절한 파서 호출
    char message_type = detectMessageType(raw_data, data_length);
    
    switch (message_type) {
        case 'J':  // JSON 메시지
            result = parseJsonMessage(message);
            break;
            
        case 'T':  // 일반 텍스트
            result = parseTextMessage(message);
            break;
            
        default:   // 알 수 없는 형식
            return result;  // valid = false 상태로 반환
    }

    // 명령 유효성 최종 검증
    result.valid = validateCommand(result);
    
    return result;
}

TypingCommand Parser::parseJsonMessage(const String& json_string) {
    TypingCommand result = {
        .text = "",
        .speed_cps = DEFAULT_TYPING_SPEED_CPS,
        .interval_ms = DEFAULT_INTERVAL_MS,
        .has_toggle = false,
        .valid = false
    };

    if (!initialized || json_document == nullptr) {
        return result;
    }

    // JSON 파싱
    json_document->clear();
    DeserializationError error = deserializeJson(*json_document, json_string);
    
    if (error) {
        return result;  // JSON 파싱 실패
    }

    // 필수 필드 추출 및 검증
    if (!json_document->containsKey(JSON_FIELD_TEXT)) {
        return result;  // 텍스트 필드 없음
    }

    // 텍스트 추출 및 정제
    String raw_text = safeGetJsonValue(*json_document, JSON_FIELD_TEXT, String(""));
    result.text = sanitizeText(raw_text);
    
    if (result.text.length() == 0) {
        return result;  // 유효한 텍스트 없음
    }

    // 선택적 필드 추출
    int speed = safeGetJsonValue(*json_document, JSON_FIELD_SPEED, (int)DEFAULT_TYPING_SPEED_CPS);
    int interval = safeGetJsonValue(*json_document, JSON_FIELD_INTERVAL, (int)DEFAULT_INTERVAL_MS);

    // 값 정규화
    result.speed_cps = normalizeTypingSpeed(speed);
    result.interval_ms = normalizeInterval(interval);

    // 토글 마커 감지
    result.has_toggle = hasToggleMarker(result.text);

    result.valid = true;
    return result;
}

TypingCommand Parser::parseTextMessage(const String& text_string) {
    TypingCommand result = {
        .text = sanitizeText(text_string),
        .speed_cps = DEFAULT_TYPING_SPEED_CPS,
        .interval_ms = DEFAULT_INTERVAL_MS,
        .has_toggle = hasToggleMarker(text_string),
        .valid = (text_string.length() > 0)
    };

    return result;
}

bool Parser::hasToggleMarker(const String& text) {
    return text.indexOf(TOGGLE_MARKER) >= 0;
}

size_t Parser::splitTextByToggle(const String& text, TextChunk* chunks, size_t max_chunks) {
    if (chunks == nullptr || max_chunks == 0) {
        return 0;
    }

    // 토글 마커가 없는 경우 전체 텍스트를 하나의 청크로 처리
    if (!hasToggleMarker(text)) {
        chunks[0] = {
            .content = text,
            .position = 0,
            .is_last = true,
            .has_toggle_before = false,
            .has_toggle_after = false
        };
        return 1;
    }

    size_t chunk_count = 0;
    int start_pos = 0;
    int toggle_pos = 0;
    
    // 토글 마커로 텍스트 분할
    while (chunk_count < max_chunks) {
        toggle_pos = text.indexOf(TOGGLE_MARKER, start_pos);
        
        if (toggle_pos == -1) {
            // 마지막 청크 처리
            if (start_pos < (int)text.length()) {
                chunks[chunk_count] = {
                    .content = text.substring(start_pos),
                    .position = (size_t)start_pos,
                    .is_last = true,
                    .has_toggle_before = (chunk_count > 0),
                    .has_toggle_after = false
                };
                chunk_count++;
            }
            break;
        }
        
        // 현재 청크 생성 (토글 마커 전까지)
        if (toggle_pos > start_pos) {
            chunks[chunk_count] = {
                .content = text.substring(start_pos, toggle_pos),
                .position = (size_t)start_pos,
                .is_last = false,
                .has_toggle_before = (chunk_count > 0),
                .has_toggle_after = true
            };
            chunk_count++;
        }
        
        // 다음 청크 시작 위치 설정 (토글 마커 다음)
        start_pos = toggle_pos + TOGGLE_MARKER_LENGTH;
    }

    // 마지막 청크 표시 업데이트
    if (chunk_count > 0) {
        chunks[chunk_count - 1].is_last = true;
    }

    return chunk_count;
}

bool Parser::validateCommand(const TypingCommand& command) {
    // 텍스트 유효성 검사
    if (command.text.length() == 0 || command.text.length() > MAX_TEXT_CHUNK_SIZE) {
        return false;
    }

    // 타이핑 속도 유효성 검사
    if (command.speed_cps < MIN_TYPING_SPEED_CPS || command.speed_cps > MAX_TYPING_SPEED_CPS) {
        return false;
    }

    // 간격 시간 유효성 검사 (최대 30초)
    if (command.interval_ms > 30000) {
        return false;
    }

    return true;
}

String Parser::sanitizeText(const String& input) {
    if (input.length() == 0) {
        return "";
    }

    String result;
    result.reserve(input.length());

    // 각 문자 검사 및 필터링
    for (size_t i = 0; i < input.length(); i++) {
        char ch = input.charAt(i);
        
        // 출력 가능한 문자, 개행, 탭만 허용
        if ((ch >= 0x20 && ch <= 0x7E) ||  // 출력 가능한 ASCII
            ch == '\n' || ch == '\t' ||     // 개행, 탭
            (ch & 0x80)) {                  // UTF-8 멀티바이트 문자
            result += ch;
        }
        // 기타 제어 문자는 무시
    }

    return result;
}

String Parser::generateResponse(size_t typed_chars, bool success) {
    if (success) {
        return "OK:" + String(typed_chars);
    } else {
        return "ERR:TYPING_FAILED";
    }
}

char Parser::detectMessageType(const char* data, size_t length) {
    if (data == nullptr || length == 0) {
        return 'U';  // Unknown
    }

    // JSON 형식 감지 ('{' 로 시작하는지 확인)
    if (data[0] == PROTOCOL_JSON_START) {
        // JSON 종료 문자 확인
        for (size_t i = length - 1; i > 0; i--) {
            if (data[i] == PROTOCOL_JSON_END) {
                return 'J';  // JSON
            } else if (data[i] != ' ' && data[i] != '\n' && data[i] != '\r') {
                break;  // 공백이 아닌 문자가 나오면 중단
            }
        }
    }

    return 'T';  // Text
}

bool Parser::isValidJson(const String& json_string) {
    if (!initialized || json_document == nullptr) {
        return false;
    }

    json_document->clear();
    DeserializationError error = deserializeJson(*json_document, json_string);
    
    return (error == DeserializationError::Ok);
}

template<typename T>
T Parser::safeGetJsonValue(const JsonDocument& doc, const char* key, T default_value) {
    if (doc.containsKey(key)) {
        return doc[key].as<T>();
    }
    return default_value;
}

uint8_t Parser::normalizeTypingSpeed(int speed) {
    return (uint8_t)CLAMP(speed, MIN_TYPING_SPEED_CPS, MAX_TYPING_SPEED_CPS);
}

uint16_t Parser::normalizeInterval(int interval) {
    return (uint16_t)CLAMP(interval, 0, 30000);  // 최대 30초
}