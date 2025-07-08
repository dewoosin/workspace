#pragma once
#include <Arduino.h>

// Hangul to QWERTY mapping system for Korean typing
// 한글-QWERTY 매핑 시스템 (한국어 타이핑용)

class HangulQWERTY {
private:
    // Unicode ranges for Hangul syllables (가-힣)
    // 한글 음절 유니코드 범위 (가-힣)
    static const uint16_t HANGUL_BASE = 0xAC00;      // 가
    static const uint16_t HANGUL_END = 0xD7A3;       // 힣
    static const uint16_t HANGUL_COUNT = 11172;      // Total syllables / 총 음절 수
    
    // Jamo counts for syllable calculation
    // 음절 계산을 위한 자모 개수
    static const uint8_t INITIAL_COUNT = 19;         // 초성 개수 (ㄱ-ㅎ)
    static const uint8_t MEDIAL_COUNT = 21;          // 중성 개수 (ㅏ-ㅣ)
    static const uint8_t FINAL_COUNT = 28;           // 종성 개수 (없음, ㄱ-ㅎ)
    
    // QWERTY keyboard layout mapping tables
    // QWERTY 자판 매핑 테이블
    static const char* initialConsonants[INITIAL_COUNT];
    static const char* medialVowels[MEDIAL_COUNT];
    static const char* finalConsonants[FINAL_COUNT];
    
    // Complex jamo handling tables
    // 복합 자모 처리 테이블
    static const char* complexVowels[12][2];         // 복합 모음 분해
    static const char* complexConsonants[11][2];     // 복합 자음 분해
    
public:
    // Main conversion function: Hangul string → QWERTY sequence
    // 메인 변환 함수: 한글 문자열 → QWERTY 키 시퀀스
    static String hangulToQWERTY(const String& hangulText);
    
    // Decompose single Hangul syllable into jamo components
    // 한글 음절을 자모 요소로 분해
    static bool decomposeSyllable(uint16_t syllable, uint8_t& initial, uint8_t& medial, uint8_t& final);
    
    // Convert jamo indices to QWERTY keystrokes
    // 자모 인덱스를 QWERTY 키 입력으로 변환
    static String jamoToQWERTY(uint8_t initial, uint8_t medial, uint8_t final);
    
    // Handle complex jamo (ㅙ, ㄺ etc.) decomposition
    // 복합 자모 (ㅙ, ㄺ 등) 분해 처리
    static String handleComplexJamo(uint8_t jamoIndex, bool isVowel);
    
    // Validation function to verify typing accuracy
    // 타이핑 정확성 검증 함수
    static bool validateConversion(const String& original, const String& qwertyKeys);
    
    // Test functions for edge cases
    // 예외 케이스 테스트 함수
    static bool runTests();
    static void testEdgeCases();
    static void testComplexJamo();
};