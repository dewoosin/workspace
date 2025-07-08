/**
 * @file parser.h
 * @brief 텍스트 파싱 및 명령어 감지 모듈
 * @version 1.0
 * @date 2024-12-28
 * 
 * 이 모듈은 BLE를 통해 수신된 데이터를 파싱하고 해석합니다.
 * JSON 형식의 타이핑 명령과 토글 마커를 처리합니다.
 */

#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"

/**
 * @brief 파싱된 타이핑 명령 구조체
 * 
 * JSON에서 추출된 타이핑 관련 정보를 담습니다.
 */
struct TypingCommand {
    String text;              ///< 타이핑할 텍스트
    uint8_t speed_cps;        ///< 타이핑 속도 (문자/초)
    uint16_t interval_ms;     ///< 간격 지연 시간 (밀리초)
    bool has_toggle;          ///< 토글 마커 포함 여부
    bool valid;               ///< 명령 유효성
};

/**
 * @brief 텍스트 청크 정보 구조체
 * 
 * 긴 텍스트를 처리하기 위한 청크 정보를 담습니다.
 */
struct TextChunk {
    String content;           ///< 청크 내용
    size_t position;          ///< 전체 텍스트 내 위치
    bool is_last;             ///< 마지막 청크 여부
    bool has_toggle_before;   ///< 청크 앞에 토글 마커가 있는지
    bool has_toggle_after;    ///< 청크 뒤에 토글 마커가 있는지
};

/**
 * @brief 텍스트 파서 클래스
 * 
 * BLE로 수신된 데이터를 파싱하고 타이핑 명령으로 변환합니다.
 */
class Parser {
public:
    /**
     * @brief 파서 초기화
     * @return true 성공, false 실패
     * 
     * JSON 파서를 초기화하고 내부 버퍼를 설정합니다.
     */
    static bool initialize();

    /**
     * @brief 파서 종료
     * 
     * 사용된 메모리를 정리하고 파서를 종료합니다.
     */
    static void deinitialize();

    /**
     * @brief 수신 데이터 파싱
     * @param raw_data 원시 수신 데이터
     * @param data_length 데이터 길이
     * @return 파싱된 타이핑 명령
     * 
     * BLE로 수신된 원시 데이터를 파싱하여 타이핑 명령으로 변환합니다.
     * JSON 형식과 일반 텍스트 형식을 모두 지원합니다.
     */
    static TypingCommand parseMessage(const char* raw_data, size_t data_length);

    /**
     * @brief JSON 메시지 파싱
     * @param json_string JSON 문자열
     * @return 파싱된 타이핑 명령
     * 
     * JSON 형식의 타이핑 명령을 파싱합니다.
     * 텍스트, 속도, 간격 등의 정보를 추출합니다.
     */
    static TypingCommand parseJsonMessage(const String& json_string);

    /**
     * @brief 일반 텍스트 파싱
     * @param text_string 일반 텍스트
     * @return 파싱된 타이핑 명령
     * 
     * 일반 텍스트를 기본 설정으로 타이핑 명령으로 변환합니다.
     */
    static TypingCommand parseTextMessage(const String& text_string);

    /**
     * @brief 토글 마커 감지
     * @param text 검사할 텍스트
     * @return true 토글 마커 포함, false 없음
     * 
     * 텍스트에 한영 토글 마커가 포함되어 있는지 확인합니다.
     */
    static bool hasToggleMarker(const String& text);

    /**
     * @brief 텍스트를 토글 기준으로 분할
     * @param text 분할할 텍스트
     * @param chunks 분할된 청크를 저장할 배열
     * @param max_chunks 최대 청크 개수
     * @return 실제 분할된 청크 개수
     * 
     * 토글 마커를 기준으로 텍스트를 여러 청크로 분할합니다.
     * 각 청크는 단일 언어 모드를 유지합니다.
     */
    static size_t splitTextByToggle(const String& text, TextChunk* chunks, size_t max_chunks);

    /**
     * @brief 타이핑 명령 유효성 검증
     * @param command 검증할 명령
     * @return true 유효, false 무효
     * 
     * 파싱된 타이핑 명령이 실행 가능한지 검증합니다.
     * 텍스트 길이, 속도 범위 등을 확인합니다.
     */
    static bool validateCommand(const TypingCommand& command);

    /**
     * @brief 안전한 텍스트 정제
     * @param input 입력 텍스트
     * @return 정제된 텍스트
     * 
     * 입력 텍스트에서 안전하지 않은 문자를 제거하거나
     * 적절한 문자로 대체합니다.
     */
    static String sanitizeText(const String& input);

    /**
     * @brief 응답 메시지 생성
     * @param typed_chars 타이핑된 문자 수
     * @param success 성공 여부
     * @return 응답 메시지
     * 
     * 타이핑 완료 후 클라이언트에게 보낼 응답 메시지를 생성합니다.
     */
    static String generateResponse(size_t typed_chars, bool success);

    /**
     * @brief 메시지 유형 감지
     * @param data 원시 데이터
     * @param length 데이터 길이
     * @return 메시지 유형 ('J': JSON, 'T': Text, 'U': Unknown)
     * 
     * 수신된 데이터가 JSON인지 일반 텍스트인지 판단합니다.
     */
    static char detectMessageType(const char* data, size_t length);

private:
    static bool initialized;                    ///< 초기화 상태
    static DynamicJsonDocument* json_document;  ///< JSON 파싱용 문서

    /**
     * @brief JSON 유효성 검사
     * @param json_string JSON 문자열
     * @return true 유효한 JSON, false 무효
     */
    static bool isValidJson(const String& json_string);

    /**
     * @brief 안전한 JSON 값 추출
     * @param doc JSON 문서
     * @param key 키 이름
     * @param default_value 기본값
     * @return 추출된 값 또는 기본값
     */
    template<typename T>
    static T safeGetJsonValue(const JsonDocument& doc, const char* key, T default_value);

    /**
     * @brief 타이핑 속도 정규화
     * @param speed 입력 속도
     * @return 정규화된 속도 (유효 범위 내)
     */
    static uint8_t normalizeTypingSpeed(int speed);

    /**
     * @brief 간격 시간 정규화
     * @param interval 입력 간격
     * @return 정규화된 간격 (유효 범위 내)
     */
    static uint16_t normalizeInterval(int interval);
};