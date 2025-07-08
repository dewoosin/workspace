/**
 * @file typing_handler.h
 * @brief 타이핑 실행 및 제어 모듈
 * @version 1.0
 * @date 2024-12-28
 * 
 * 이 모듈은 파싱된 타이핑 명령을 실제 키보드 입력으로 실행합니다.
 * 한영 토글, 타이핑 속도 조절, 안전성 검사 등을 담당합니다.
 */

#pragma once

#include <Arduino.h>
#include "config.h"
#include "parser.h"
#include "hid_utils.h"

/**
 * @brief 타이핑 실행 결과 구조체
 * 
 * 타이핑 작업의 실행 결과를 담습니다.
 */
struct TypingResult {
    size_t chars_processed;   ///< 처리된 총 문자 수
    size_t chars_typed;       ///< 실제 타이핑된 문자 수
    uint32_t execution_time;  ///< 실행 시간 (밀리초)
    bool success;             ///< 실행 성공 여부
    ErrorCode error_code;     ///< 오류 코드 (실패 시)
};

/**
 * @brief 타이핑 상태 구조체
 * 
 * 현재 타이핑 작업의 상태를 추적합니다.
 */
struct TypingState {
    bool is_active;           ///< 타이핑 작업 활성화 여부
    uint32_t start_time;      ///< 타이핑 시작 시간
    size_t current_position;  ///< 현재 처리 위치
    size_t total_length;      ///< 전체 텍스트 길이
    TypingMode mode;          ///< 현재 타이핑 모드
};

/**
 * @brief 타이핑 핸들러 클래스
 * 
 * 파싱된 타이핑 명령을 실제 키보드 입력으로 변환하고 실행합니다.
 */
class TypingHandler {
public:
    /**
     * @brief 타이핑 핸들러 초기화
     * @return true 성공, false 실패
     * 
     * HID 시스템을 초기화하고 타이핑 핸들러를 준비합니다.
     */
    static bool initialize();

    /**
     * @brief 타이핑 핸들러 종료
     * 
     * 진행 중인 타이핑을 중단하고 리소스를 정리합니다.
     */
    static void deinitialize();

    /**
     * @brief 타이핑 명령 실행
     * @param command 실행할 타이핑 명령
     * @return 타이핑 실행 결과
     * 
     * 파싱된 타이핑 명령을 실제 키보드 입력으로 실행합니다.
     * 토글 마커 처리, 속도 조절 등을 포함합니다.
     */
    static TypingResult executeCommand(const TypingCommand& command);

    /**
     * @brief 토글 포함 텍스트 타이핑
     * @param command 타이핑 명령
     * @return 타이핑 실행 결과
     * 
     * 한영 토글 마커가 포함된 텍스트를 처리합니다.
     * 토글 지점에서 적절한 키 조합을 생성합니다.
     */
    static TypingResult executeWithToggle(const TypingCommand& command);

    /**
     * @brief 일반 텍스트 타이핑
     * @param command 타이핑 명령
     * @return 타이핑 실행 결과
     * 
     * 토글 마커가 없는 일반 텍스트를 타이핑합니다.
     */
    static TypingResult executeNormalText(const TypingCommand& command);

    /**
     * @brief 한영 토글 키 전송
     * @return true 성공, false 실패
     * 
     * 한영 전환을 위한 특수 키 조합을 전송합니다.
     * 일반적으로 Alt+한/영 또는 Ctrl+Space를 사용합니다.
     */
    static bool sendToggleKey();

    /**
     * @brief 현재 타이핑 상태 조회
     * @return 현재 타이핑 상태
     * 
     * 진행 중인 타이핑 작업의 상태를 반환합니다.
     */
    static TypingState getCurrentState();

    /**
     * @brief 타이핑 작업 중단
     * @return true 성공, false 실패
     * 
     * 현재 진행 중인 타이핑 작업을 안전하게 중단합니다.
     */
    static bool abortTyping();

    /**
     * @brief 타이핑 모드 설정
     * @param mode 설정할 타이핑 모드
     * 
     * 타이핑의 속도와 안전성 수준을 조절합니다.
     */
    static void setTypingMode(TypingMode mode);

    /**
     * @brief 안전 모드 토글
     * @param enabled true 활성화, false 비활성화
     * 
     * 안전 모드에서는 추가적인 검증과 지연을 적용합니다.
     */
    static void setSafeMode(bool enabled);

    /**
     * @brief 타이핑 진행률 계산
     * @return 진행률 (0-100)
     * 
     * 현재 타이핑 작업의 진행률을 백분율로 반환합니다.
     */
    static uint8_t getProgress();

    /**
     * @brief 예상 완료 시간 계산
     * @param command 타이핑 명령
     * @return 예상 완료 시간 (밀리초)
     * 
     * 주어진 명령의 예상 실행 시간을 계산합니다.
     */
    static uint32_t estimateCompletionTime(const TypingCommand& command);

private:
    static bool initialized;           ///< 초기화 상태
    static TypingState current_state;  ///< 현재 타이핑 상태
    static TypingMode typing_mode;     ///< 현재 타이핑 모드
    static bool safe_mode;             ///< 안전 모드 활성화 여부

    /**
     * @brief 타이핑 상태 초기화
     * @param text_length 전체 텍스트 길이
     */
    static void initializeState(size_t text_length);

    /**
     * @brief 타이핑 상태 업데이트
     * @param chars_processed 처리된 문자 수
     */
    static void updateState(size_t chars_processed);

    /**
     * @brief 타이핑 상태 리셋
     */
    static void resetState();

    /**
     * @brief 타이핑 시간 초과 검사
     * @return true 시간 초과, false 정상
     */
    static bool isTimedOut();

    /**
     * @brief 안전 검사 수행
     * @param command 검사할 명령
     * @return true 안전, false 위험
     */
    static bool performSafetyCheck(const TypingCommand& command);

    /**
     * @brief 타이핑 모드에 따른 지연 시간 계산
     * @param base_delay 기본 지연 시간
     * @return 조정된 지연 시간
     */
    static uint32_t adjustDelayForMode(uint32_t base_delay);

    /**
     * @brief 청크 단위 타이핑 실행
     * @param chunk 타이핑할 청크
     * @param speed_cps 타이핑 속도
     * @param interval_ms 간격 지연
     * @return 타이핑된 문자 수
     */
    static size_t executeChunk(const TextChunk& chunk, uint8_t speed_cps, uint16_t interval_ms);

    /**
     * @brief 오류 코드를 결과로 변환
     * @param error_code 오류 코드
     * @param chars_typed 타이핑된 문자 수
     * @param execution_time 실행 시간
     * @return 타이핑 결과
     */
    static TypingResult createErrorResult(ErrorCode error_code, size_t chars_typed, uint32_t execution_time);
};