#include "HangulQWERTY.h"

// Static member initialization - QWERTY mapping tables
// 정적 멤버 초기화 - QWERTY 매핑 테이블

// Initial consonants (초성) mapping to QWERTY keys
// 초성을 QWERTY 키로 매핑
const char* HangulQWERTY::initialConsonants[INITIAL_COUNT] = {
    "r",    // ㄱ (0)
    "R",    // ㄲ (1) - double ㄱ
    "s",    // ㄴ (2)
    "e",    // ㄷ (3)
    "E",    // ㄸ (4) - double ㄷ
    "f",    // ㄹ (5)
    "a",    // ㅁ (6)
    "q",    // ㅂ (7)
    "Q",    // ㅃ (8) - double ㅂ
    "t",    // ㅅ (9)
    "T",    // ㅆ (10) - double ㅅ
    "d",    // ㅇ (11)
    "w",    // ㅈ (12)
    "W",    // ㅉ (13) - double ㅈ
    "c",    // ㅊ (14)
    "z",    // ㅋ (15)
    "x",    // ㅌ (16)
    "v",    // ㅍ (17)
    "g"     // ㅎ (18)
};

// Medial vowels (중성) mapping to QWERTY keys
// 중성을 QWERTY 키로 매핑
const char* HangulQWERTY::medialVowels[MEDIAL_COUNT] = {
    "k",    // ㅏ (0)
    "o",    // ㅐ (1)
    "i",    // ㅑ (2)
    "O",    // ㅒ (3)
    "j",    // ㅓ (4)
    "p",    // ㅔ (5)
    "u",    // ㅕ (6)
    "P",    // ㅖ (7)
    "h",    // ㅗ (8)
    "hk",   // ㅘ (9) - ㅗ + ㅏ
    "ho",   // ㅙ (10) - ㅗ + ㅐ
    "hl",   // ㅚ (11) - ㅗ + ㅣ
    "y",    // ㅛ (12)
    "n",    // ㅜ (13)
    "nj",   // ㅝ (14) - ㅜ + ㅓ
    "np",   // ㅞ (15) - ㅜ + ㅔ
    "nl",   // ㅟ (16) - ㅜ + ㅣ
    "b",    // ㅠ (17)
    "m",    // ㅡ (18)
    "ml",   // ㅢ (19) - ㅡ + ㅣ
    "l"     // ㅣ (20)
};

// Final consonants (종성) mapping to QWERTY keys
// 종성을 QWERTY 키로 매핑
const char* HangulQWERTY::finalConsonants[FINAL_COUNT] = {
    "",     // (none) (0) - 받침 없음
    "r",    // ㄱ (1)
    "R",    // ㄲ (2) - double ㄱ
    "rt",   // ㄳ (3) - ㄱ + ㅅ
    "s",    // ㄴ (4)
    "sw",   // ㄵ (5) - ㄴ + ㅈ
    "sg",   // ㄶ (6) - ㄴ + ㅎ
    "e",    // ㄷ (7)
    "f",    // ㄹ (8)
    "fr",   // ㄺ (9) - ㄹ + ㄱ
    "fa",   // ㄻ (10) - ㄹ + ㅁ
    "fq",   // ㄼ (11) - ㄹ + ㅂ
    "ft",   // ㄽ (12) - ㄹ + ㅅ
    "fx",   // ㄾ (13) - ㄹ + ㅌ
    "fv",   // ㄿ (14) - ㄹ + ㅍ
    "fg",   // ㅀ (15) - ㄹ + ㅎ
    "a",    // ㅁ (16)
    "q",    // ㅂ (17)
    "qt",   // ㅄ (18) - ㅂ + ㅅ
    "t",    // ㅅ (19)
    "T",    // ㅆ (20) - double ㅅ
    "d",    // ㅇ (21)
    "w",    // ㅈ (22)
    "c",    // ㅊ (23)
    "z",    // ㅋ (24)
    "x",    // ㅌ (25)
    "v",    // ㅍ (26)
    "g"     // ㅎ (27)
};

// Complex vowel decomposition table
// 복합 모음 분해 테이블
const char* HangulQWERTY::complexVowels[12][2] = {
    {"h", "k"},   // ㅘ = ㅗ + ㅏ
    {"h", "o"},   // ㅙ = ㅗ + ㅐ
    {"h", "l"},   // ㅚ = ㅗ + ㅣ
    {"n", "j"},   // ㅝ = ㅜ + ㅓ
    {"n", "p"},   // ㅞ = ㅜ + ㅔ
    {"n", "l"},   // ㅟ = ㅜ + ㅣ
    {"m", "l"},   // ㅢ = ㅡ + ㅣ
    {"i", "o"},   // ㅒ = ㅑ + ㅣ (실제로는 ㅑ + ㅐ)
    {"u", "p"},   // ㅖ = ㅕ + ㅣ (실제로는 ㅕ + ㅔ)
    {"", ""},     // Reserved
    {"", ""},     // Reserved
    {"", ""}      // Reserved
};

// Complex consonant decomposition table
// 복합 자음 분해 테이블
const char* HangulQWERTY::complexConsonants[11][2] = {
    {"r", "t"},   // ㄳ = ㄱ + ㅅ
    {"s", "w"},   // ㄵ = ㄴ + ㅈ
    {"s", "g"},   // ㄶ = ㄴ + ㅎ
    {"f", "r"},   // ㄺ = ㄹ + ㄱ
    {"f", "a"},   // ㄻ = ㄹ + ㅁ
    {"f", "q"},   // ㄼ = ㄹ + ㅂ
    {"f", "t"},   // ㄽ = ㄹ + ㅅ
    {"f", "x"},   // ㄾ = ㄹ + ㅌ
    {"f", "v"},   // ㄿ = ㄹ + ㅍ
    {"f", "g"},   // ㅀ = ㄹ + ㅎ
    {"q", "t"}    // ㅄ = ㅂ + ㅅ
};

// Main conversion function: Hangul string → QWERTY sequence
// 메인 변환 함수: 한글 문자열 → QWERTY 키 시퀀스
String HangulQWERTY::hangulToQWERTY(const String& hangulText) {
    String result = "";
    
    for (int i = 0; i < hangulText.length(); ) {
        uint16_t ch = 0;
        
        // Handle UTF-8 encoding for Korean characters
        // 한국어 문자의 UTF-8 인코딩 처리
        if ((hangulText[i] & 0x80) == 0) {
            // ASCII character - pass through
            // ASCII 문자 - 그대로 통과
            result += hangulText[i];
            i++;
            continue;
        } else if ((hangulText[i] & 0xE0) == 0xC0) {
            // 2-byte UTF-8 sequence
            if (i + 1 < hangulText.length()) {
                ch = ((hangulText[i] & 0x1F) << 6) | (hangulText[i + 1] & 0x3F);
                i += 2;
            } else {
                i++;
                continue;
            }
        } else if ((hangulText[i] & 0xF0) == 0xE0) {
            // 3-byte UTF-8 sequence (Korean characters)
            // 3바이트 UTF-8 시퀀스 (한국어 문자)
            if (i + 2 < hangulText.length()) {
                ch = ((hangulText[i] & 0x0F) << 12) | 
                     ((hangulText[i + 1] & 0x3F) << 6) | 
                     (hangulText[i + 2] & 0x3F);
                i += 3;
            } else {
                i++;
                continue;
            }
        } else {
            // Unsupported encoding - skip
            // 지원하지 않는 인코딩 - 건너뛰기
            i++;
            continue;
        }
        
        // Check if character is in Hangul syllable range
        // 문자가 한글 음절 범위에 있는지 확인
        if (ch >= HANGUL_BASE && ch <= HANGUL_END) {
            uint8_t initial, medial, final;
            if (decomposeSyllable(ch, initial, medial, final)) {
                result += jamoToQWERTY(initial, medial, final);
            }
        } else {
            // Non-Hangul character - handle as single character
            // 한글이 아닌 문자 - 단일 문자로 처리
            if (ch < 256) {
                result += (char)ch;
            }
        }
    }
    
    return result;
}

// Decompose single Hangul syllable into jamo components
// 한글 음절을 자모 요소로 분해
bool HangulQWERTY::decomposeSyllable(uint16_t syllable, uint8_t& initial, uint8_t& medial, uint8_t& final) {
    if (syllable < HANGUL_BASE || syllable > HANGUL_END) {
        return false;
    }
    
    // Calculate jamo indices using standard Korean algorithm
    // 표준 한국어 알고리즘을 사용하여 자모 인덱스 계산
    uint16_t syllableIndex = syllable - HANGUL_BASE;
    
    initial = syllableIndex / (MEDIAL_COUNT * FINAL_COUNT);
    medial = (syllableIndex % (MEDIAL_COUNT * FINAL_COUNT)) / FINAL_COUNT;
    final = syllableIndex % FINAL_COUNT;
    
    return true;
}

// Convert jamo indices to QWERTY keystrokes
// 자모 인덱스를 QWERTY 키 입력으로 변환
String HangulQWERTY::jamoToQWERTY(uint8_t initial, uint8_t medial, uint8_t final) {
    String result = "";
    
    // Add initial consonant (초성)
    // 초성 추가
    if (initial < INITIAL_COUNT) {
        result += initialConsonants[initial];
    }
    
    // Add medial vowel (중성)
    // 중성 추가
    if (medial < MEDIAL_COUNT) {
        result += medialVowels[medial];
    }
    
    // Add final consonant (종성) if present
    // 종성이 있으면 추가
    if (final > 0 && final < FINAL_COUNT) {
        result += finalConsonants[final];
    }
    
    return result;
}

// Handle complex jamo (ㅙ, ㄺ etc.) decomposition
// 복합 자모 (ㅙ, ㄺ 등) 분해 처리
String HangulQWERTY::handleComplexJamo(uint8_t jamoIndex, bool isVowel) {
    String result = "";
    
    if (isVowel && jamoIndex < 12) {
        // Complex vowel decomposition
        // 복합 모음 분해
        result += complexVowels[jamoIndex][0];
        result += complexVowels[jamoIndex][1];
    } else if (!isVowel && jamoIndex < 11) {
        // Complex consonant decomposition
        // 복합 자음 분해
        result += complexConsonants[jamoIndex][0];
        result += complexConsonants[jamoIndex][1];
    }
    
    return result;
}

// Validation function to verify typing accuracy
// 타이핑 정확성 검증 함수
bool HangulQWERTY::validateConversion(const String& original, const String& qwertyKeys) {
    // This is a simplified validation - in practice, you would simulate
    // typing the QWERTY keys and compare the resulting Korean text
    // 이것은 간단한 검증입니다 - 실제로는 QWERTY 키를 시뮬레이션하여
    // 타이핑하고 결과 한국어 텍스트를 비교해야 합니다
    
    String reconverted = hangulToQWERTY(original);
    return reconverted.equals(qwertyKeys);
}

// Test functions for edge cases
// 예외 케이스 테스트 함수
bool HangulQWERTY::runTests() {
    bool allPassed = true;
    
    // Test basic conversions - 기본 변환 테스트
    struct TestCase {
        String hangul;
        String expectedQWERTY;
    };
    
    TestCase testCases[] = {
        {"가", "rk"},           // 가 = ㄱ + ㅏ
        {"윤", "dbs"},          // 윤 = ㅇ + ㅠ + ㄴ  
        {"하늘", "gksrmf"},     // 하 = ㅎ + ㅏ, 늘 = ㄴ + ㅡ + ㄹ
        {"되", "enl"},          // 되 = ㄷ + ㅚ
        {"돼", "eho"},          // 돼 = ㄷ + ㅙ
        {"맑", "akfr"},         // 맑 = ㅁ + ㅏ + ㄺ
        {"띄", "Eml"},          // 띄 = ㄸ + ㅢ
        {"넓", "spfq"},         // 넓 = ㄴ + ㅓ + ㄼ
        {"", ""}
    };
    
    for (int i = 0; testCases[i].hangul != ""; i++) {
        String result = hangulToQWERTY(testCases[i].hangul);
        if (!result.equals(testCases[i].expectedQWERTY)) {
            allPassed = false;
            // Test failed - 테스트 실패
        }
    }
    
    return allPassed;
}

void HangulQWERTY::testEdgeCases() {
    // Test challenging cases - 어려운 케이스 테스트
    
    // "되" vs "돼" distinction test - "되"와 "돼" 구분 테스트
    String doe = hangulToQWERTY("되");    // Should be "enl"
    String dwae = hangulToQWERTY("돼");   // Should be "eho"
    
    // Complex batchim test - 복합 받침 테스트
    String malg = hangulToQWERTY("맑");   // Should be "akfr" (ㅁ+ㅏ+ㄺ)
    String neolb = hangulToQWERTY("넓");  // Should be "spfq" (ㄴ+ㅓ+ㄼ)
    
    // Double consonant test - 된소리 테스트
    String ddui = hangulToQWERTY("띄");   // Should be "Eml" (ㄸ+ㅢ)
}

void HangulQWERTY::testComplexJamo() {
    // Test complex jamo handling - 복합 자모 처리 테스트
    
    // Complex vowels - 복합 모음
    String gwa = hangulToQWERTY("과");    // ㅗ+ㅏ = hk
    String gwae = hangulToQWERTY("괘");   // ㅗ+ㅐ = ho  
    String goe = hangulToQWERTY("괴");    // ㅗ+ㅣ = hl
    
    // Complex final consonants - 복합 종성
    String gags = hangulToQWERTY("갃");   // ㄱ+ㅅ = rt
    String galg = hangulToQWERTY("갏");   // ㄹ+ㄱ = fr
}