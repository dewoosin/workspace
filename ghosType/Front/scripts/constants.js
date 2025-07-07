// BLE Service and Characteristic UUIDs
export const BLE_CONFIG = {
    SERVICE_UUID: '6e400001-b5a3-f393-e0a9-e50e24dcca9e',
    RX_CHAR_UUID: '6e400002-b5a3-f393-e0a9-e50e24dcca9e',
    TX_CHAR_UUID: '6e400003-b5a3-f393-e0a9-e50e24dcca9e'
};

// Protocol Definitions
export const PROTOCOLS = {
    PREFIX: 'GHTYPE_',
    ENGLISH: 'GHTYPE_ENG:',
    KOREAN: 'GHTYPE_KOR:',
    SPECIAL: 'GHTYPE_SPE:',
    CONFIG: 'GHTYPE_CFG:'
};

// Korean Keyboard Mappings (Dubeolsik Standard Layout)
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

// Default Configuration
export const DEFAULT_CONFIG = {
    TYPING_SPEED: 6,
    COUNTDOWN_SECONDS: 5
};