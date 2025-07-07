/**
 * Comprehensive Korean Keyboard Mappings
 * Based on standard Dubeolsik (두벌식) layout
 * 
 * Korean keyboard layout reference:
 * - Left hand: Consonants (자음)
 * - Right hand: Vowels (모음)
 * - Shift combinations for double consonants and complex vowels
 */

// === QWERTY to Jamo Mappings ===
export const QWERTY_TO_JAMO = {
    // Basic Consonants (자음) - Left hand
    'q': 'ㅂ',    // bieup
    'w': 'ㅈ',    // jieut
    'e': 'ㄷ',    // digeut
    'r': 'ㄱ',    // giyeok
    't': 'ㅅ',    // siot
    'a': 'ㅁ',    // mieum
    's': 'ㄴ',    // nieun
    'd': 'ㅇ',    // ieung
    'f': 'ㄹ',    // rieul
    'g': 'ㅎ',    // hieut
    'z': 'ㅋ',    // kieuk
    'x': 'ㅌ',    // tieut
    'c': 'ㅊ',    // chieut
    'v': 'ㅍ',    // pieup

    // Basic Vowels (모음) - Right hand
    'y': 'ㅛ',    // yo
    'u': 'ㅕ',    // yeo
    'i': 'ㅑ',    // ya
    'o': 'ㅐ',    // ae
    'p': 'ㅔ',    // e
    'h': 'ㅗ',    // o
    'j': 'ㅓ',    // eo
    'k': 'ㅏ',    // a
    'l': 'ㅣ',    // i
    'b': 'ㅠ',    // yu
    'n': 'ㅜ',    // u
    'm': 'ㅡ',    // eu

    // Shift + Consonants (쌍자음 - Double consonants)
    'Q': 'ㅃ',    // ssangbieup
    'W': 'ㅉ',    // ssangjieut
    'E': 'ㄸ',    // ssangdigeut
    'R': 'ㄲ',    // ssanggiyeok
    'T': 'ㅆ',    // ssangsiot

    // Shift + Vowels (복합모음 - Complex vowels)
    'O': 'ㅒ',    // yae
    'P': 'ㅖ'     // ye
};

// === Jamo Character Arrays for Unicode Decomposition ===
export const JAMO_ARRAYS = {
    // 초성 (Initial consonants) - 19 characters
    CHOSUNG: [
        'ㄱ', 'ㄲ', 'ㄴ', 'ㄷ', 'ㄸ', 'ㄹ', 'ㅁ', 'ㅂ', 'ㅃ', 'ㅅ', 
        'ㅆ', 'ㅇ', 'ㅈ', 'ㅉ', 'ㅊ', 'ㅋ', 'ㅌ', 'ㅍ', 'ㅎ'
    ],
    
    // 중성 (Medial vowels) - 21 characters
    JUNGSUNG: [
        'ㅏ', 'ㅐ', 'ㅑ', 'ㅒ', 'ㅓ', 'ㅔ', 'ㅕ', 'ㅖ', 'ㅗ', 'ㅘ', 
        'ㅙ', 'ㅚ', 'ㅛ', 'ㅜ', 'ㅝ', 'ㅞ', 'ㅟ', 'ㅠ', 'ㅡ', 'ㅢ', 'ㅣ'
    ],
    
    // 종성 (Final consonants) - 28 characters (including empty)
    JONGSUNG: [
        '',     // 0: no final consonant
        'ㄱ',   // 1: giyeok
        'ㄲ',   // 2: ssanggiyeok
        'ㄳ',   // 3: giyeok-siot
        'ㄴ',   // 4: nieun
        'ㄵ',   // 5: nieun-jieut
        'ㄶ',   // 6: nieun-hieut
        'ㄷ',   // 7: digeut
        'ㄹ',   // 8: rieul
        'ㄺ',   // 9: rieul-giyeok
        'ㄻ',   // 10: rieul-mieum
        'ㄼ',   // 11: rieul-bieup
        'ㄽ',   // 12: rieul-siot
        'ㄾ',   // 13: rieul-tieut
        'ㄿ',   // 14: rieul-pieup
        'ㅀ',   // 15: rieul-hieut
        'ㅁ',   // 16: mieum
        'ㅂ',   // 17: bieup
        'ㅄ',   // 18: bieup-siot
        'ㅅ',   // 19: siot
        'ㅆ',   // 20: ssangsiot
        'ㅇ',   // 21: ieung
        'ㅈ',   // 22: jieut
        'ㅊ',   // 23: chieut
        'ㅋ',   // 24: kieuk
        'ㅌ',   // 25: tieut
        'ㅍ',   // 26: pieup
        'ㅎ'    // 27: hieut
    ]
};

// === Complex Jongsung Decomposition Rules ===
export const COMPOUND_JONGSUNG = {
    'ㄳ': ['ㄱ', 'ㅅ'],  // giyeok-siot
    'ㄵ': ['ㄴ', 'ㅈ'],  // nieun-jieut
    'ㄶ': ['ㄴ', 'ㅎ'],  // nieun-hieut
    'ㄺ': ['ㄹ', 'ㄱ'],  // rieul-giyeok
    'ㄻ': ['ㄹ', 'ㅁ'],  // rieul-mieum
    'ㄼ': ['ㄹ', 'ㅂ'],  // rieul-bieup
    'ㄽ': ['ㄹ', 'ㅅ'],  // rieul-siot
    'ㄾ': ['ㄹ', 'ㅌ'],  // rieul-tieut
    'ㄿ': ['ㄹ', 'ㅍ'],  // rieul-pieup
    'ㅀ': ['ㄹ', 'ㅎ'],  // rieul-hieut
    'ㅄ': ['ㅂ', 'ㅅ']   // bieup-siot
};

// === Complex Vowel Decomposition Rules ===
export const COMPOUND_VOWELS = {
    'ㅘ': ['ㅗ', 'ㅏ'],  // o + a
    'ㅙ': ['ㅗ', 'ㅐ'],  // o + ae
    'ㅚ': ['ㅗ', 'ㅣ'],  // o + i
    'ㅝ': ['ㅜ', 'ㅓ'],  // u + eo
    'ㅞ': ['ㅜ', 'ㅔ'],  // u + e
    'ㅟ': ['ㅜ', 'ㅣ'],  // u + i
    'ㅢ': ['ㅡ', 'ㅣ']   // eu + i
};

// === Special Syllable Handling ===
export const SPECIAL_SYLLABLES = {
    // Common irregular syllables that need special treatment
    '돼': 'eho',     // doe -> dwe (되 + ㅗ/ㅏ combination)
    '뭐': 'anjr',    // mwo
    '뒤': 'ehn',     // dwi
    '뒤': 'ehn',     // dwi (duplicate for emphasis)
    '왜': 'hko',     // wae
    '웨': 'ho',      // we
    '위': 'hn',      // wi
    '외': 'hl',      // oe
    '의': 'ml'       // ui
};

// === Reverse Mapping (Jamo to QWERTY) ===
export const JAMO_TO_QWERTY = {};

// Build reverse mapping automatically
Object.entries(QWERTY_TO_JAMO).forEach(([qwerty, jamo]) => {
    JAMO_TO_QWERTY[jamo] = qwerty;
});

// === Unicode Constants ===
export const UNICODE_RANGES = {
    HANGUL_SYLLABLES_START: 0xAC00,  // '가'
    HANGUL_SYLLABLES_END: 0xD7A3,    // '힣'
    HANGUL_JAMO_START: 0x3130,       // 'ㄱ'
    HANGUL_JAMO_END: 0x318F,         // 'ㆎ'
    
    // Calculation constants for syllable decomposition
    CHOSUNG_COUNT: 19,
    JUNGSUNG_COUNT: 21,
    JONGSUNG_COUNT: 28,
    JUNGSUNG_JONGSUNG_COUNT: 588  // 21 * 28
};

// === Validation Sets ===
export const VALIDATION = {
    // All valid initial consonants
    VALID_CHOSUNG: new Set(JAMO_ARRAYS.CHOSUNG),
    
    // All valid vowels (including compounds)
    VALID_JUNGSUNG: new Set(JAMO_ARRAYS.JUNGSUNG),
    
    // All valid final consonants
    VALID_JONGSUNG: new Set(JAMO_ARRAYS.JONGSUNG.filter(j => j !== '')),
    
    // All mappable jamo
    MAPPABLE_JAMO: new Set(Object.values(QWERTY_TO_JAMO))
};

// === Debugging and Testing Utilities ===
export const DEBUG_UTILS = {
    // Test syllables for validation
    TEST_SYLLABLES: [
        '가', '나', '다', '라', '마', '바', '사', '아', '자', '차', '카', '타', '파', '하',
        '강', '낭', '당', '랑', '망', '방', '상', '앙', '장', '창', '캉', '탕', '팡', '항',
        '각', '낙', '닥', '락', '막', '박', '삭', '악', '작', '착', '칵', '탁', '팍', '학',
        '돼', '뭐', '뒤', '왜', '웨', '위', '외', '의',  // Special cases
        '늬', '릐', '긔', '븨', '츼'  // Edge cases
    ],
    
    // Common problem characters
    PROBLEM_CHARS: ['돼', '뭐', '쟤', '걔', '얘'],
    
    // Test if character can be typed on Korean keyboard
    isTypableOnKeyboard: (char) => {
        // Implementation will be in the converter
        return true;
    }
};