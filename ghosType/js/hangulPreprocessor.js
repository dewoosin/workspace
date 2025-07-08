/**
 * Hangul Preprocessor for ESP32 HID Typing
 * Formats user input with automatic Korean-English language switching
 * 한글 전처리기 - ESP32 HID 타이핑을 위한 한영 자동 전환
 */

class HangulPreprocessor {
    constructor() {
        // Unicode ranges for Hangul
        this.HANGUL_BASE = 0xAC00;  // 가
        this.HANGUL_END = 0xD7A3;   // 힣
        
        // Jamo counts for decomposition
        this.INITIAL_COUNT = 19;
        this.MEDIAL_COUNT = 21;
        this.FINAL_COUNT = 28;
        
        // Language toggle marker
        this.HANGUL_TOGGLE = '⌨HANGUL_TOGGLE⌨';
        
        // Initialize conversion tables
        this.initializeConversionTables();
    }
    
    /**
     * Initialize QWERTY mapping tables for Korean jamo
     * 한국어 자모를 위한 QWERTY 매핑 테이블 초기화
     */
    initializeConversionTables() {
        // Initial consonants (초성)
        this.initialConsonants = [
            'r',    // ㄱ (0)
            'R',    // ㄲ (1)
            's',    // ㄴ (2)
            'e',    // ㄷ (3)
            'E',    // ㄸ (4)
            'f',    // ㄹ (5)
            'a',    // ㅁ (6)
            'q',    // ㅂ (7)
            'Q',    // ㅃ (8)
            't',    // ㅅ (9)
            'T',    // ㅆ (10)
            'd',    // ㅇ (11)
            'w',    // ㅈ (12)
            'W',    // ㅉ (13)
            'c',    // ㅊ (14)
            'z',    // ㅋ (15)
            'x',    // ㅌ (16)
            'v',    // ㅍ (17)
            'g'     // ㅎ (18)
        ];
        
        // Medial vowels (중성)
        this.medialVowels = [
            'k',    // ㅏ (0)
            'o',    // ㅐ (1)
            'i',    // ㅑ (2)
            'O',    // ㅒ (3)
            'j',    // ㅓ (4)
            'p',    // ㅔ (5)
            'u',    // ㅕ (6)
            'P',    // ㅖ (7)
            'h',    // ㅗ (8)
            'hk',   // ㅘ (9)
            'ho',   // ㅙ (10)
            'hl',   // ㅚ (11)
            'y',    // ㅛ (12)
            'n',    // ㅜ (13)
            'nj',   // ㅝ (14)
            'np',   // ㅞ (15)
            'nl',   // ㅟ (16)
            'b',    // ㅠ (17)
            'm',    // ㅡ (18)
            'ml',   // ㅢ (19)
            'l'     // ㅣ (20)
        ];
        
        // Final consonants (종성)
        this.finalConsonants = [
            '',     // (none) (0)
            'r',    // ㄱ (1)
            'R',    // ㄲ (2)
            'rt',   // ㄳ (3)
            's',    // ㄴ (4)
            'sw',   // ㄵ (5)
            'sg',   // ㄶ (6)
            'e',    // ㄷ (7)
            'f',    // ㄹ (8)
            'fr',   // ㄺ (9)
            'fa',   // ㄻ (10)
            'fq',   // ㄼ (11)
            'ft',   // ㄽ (12)
            'fx',   // ㄾ (13)
            'fv',   // ㄿ (14)
            'fg',   // ㅀ (15)
            'a',    // ㅁ (16)
            'q',    // ㅂ (17)
            'qt',   // ㅄ (18)
            't',    // ㅅ (19)
            'T',    // ㅆ (20)
            'd',    // ㅇ (21)
            'w',    // ㅈ (22)
            'c',    // ㅊ (23)
            'z',    // ㅋ (24)
            'x',    // ㅌ (25)
            'v',    // ㅍ (26)
            'g'     // ㅎ (27)
        ];
    }
    
    /**
     * Check if a character is Hangul
     * 문자가 한글인지 확인
     */
    isHangul(char) {
        // 안전한 입력 검증
        if (!char || typeof char !== 'string' || char.length === 0) {
            return false;
        }
        
        try {
            const code = char.charCodeAt(0);
            return code >= this.HANGUL_BASE && code <= this.HANGUL_END;
        } catch (error) {
            console.warn('isHangul 오류:', error);
            return false;
        }
    }
    
    /**
     * Check if a character is ASCII (English/symbols)
     * 문자가 ASCII (영어/기호)인지 확인
     */
    isAscii(char) {
        // 안전한 입력 검증
        if (!char || typeof char !== 'string' || char.length === 0) {
            return false;
        }
        
        try {
            return char.charCodeAt(0) < 128;
        } catch (error) {
            console.warn('isAscii 오류:', error);
            return false;
        }
    }
    
    /**
     * Decompose Hangul syllable into jamo components
     * 한글 음절을 자모 요소로 분해
     */
    decomposeSyllable(syllable) {
        // 입력 검증
        if (!syllable || typeof syllable !== 'string' || syllable.length === 0) {
            return null;
        }
        
        try {
            const code = syllable.charCodeAt(0);
            if (code < this.HANGUL_BASE || code > this.HANGUL_END) {
                return null;
            }
            
            const syllableIndex = code - this.HANGUL_BASE;
            
            // 안전한 범위 확인
            if (syllableIndex < 0 || syllableIndex >= 11172) {
                return null;
            }
            
            const initial = Math.floor(syllableIndex / (this.MEDIAL_COUNT * this.FINAL_COUNT));
            const medial = Math.floor((syllableIndex % (this.MEDIAL_COUNT * this.FINAL_COUNT)) / this.FINAL_COUNT);
            const final = syllableIndex % this.FINAL_COUNT;
            
            // 인덱스 범위 검증
            if (initial >= this.INITIAL_COUNT || medial >= this.MEDIAL_COUNT || final >= this.FINAL_COUNT) {
                return null;
            }
            
            return { initial, medial, final };
        } catch (error) {
            console.warn('decomposeSyllable 오류:', error);
            return null;
        }
    }
    
    /**
     * Convert Hangul text to QWERTY keystrokes
     * 한글 텍스트를 QWERTY 키 입력으로 변환
     */
    hangulToQwerty(text) {
        // 입력 검증
        if (!text || typeof text !== 'string') {
            return '';
        }
        
        // 최대 길이 제한 (메모리 보호)
        const MAX_TEXT_LENGTH = 100000;
        if (text.length > MAX_TEXT_LENGTH) {
            console.warn(`텍스트가 너무 깁니다: ${text.length} > ${MAX_TEXT_LENGTH}`);
            text = text.substring(0, MAX_TEXT_LENGTH);
        }
        
        let result = '';
        let processedChars = 0;
        
        try {
            for (const char of text) {
                // 무한 루프 방지
                if (++processedChars > MAX_TEXT_LENGTH) {
                    console.warn('최대 처리 문자 수 초과');
                    break;
                }
                
                // null 또는 undefined 문자 건너뛰기
                if (!char) {
                    continue;
                }
                
                if (this.isHangul(char)) {
                    const jamo = this.decomposeSyllable(char);
                    if (jamo) {
                        // 배열 인덱스 안전성 검증
                        if (jamo.initial < this.initialConsonants.length) {
                            result += this.initialConsonants[jamo.initial] || '';
                        }
                        if (jamo.medial < this.medialVowels.length) {
                            result += this.medialVowels[jamo.medial] || '';
                        }
                        if (jamo.final > 0 && jamo.final < this.finalConsonants.length) {
                            result += this.finalConsonants[jamo.final] || '';
                        }
                    }
                } else {
                    // 안전한 문자만 통과
                    if (typeof char === 'string' && char.length === 1) {
                        result += char;
                    }
                }
            }
        } catch (error) {
            console.error('hangulToQwerty 오류:', error);
            return text; // 실패 시 원본 반환
        }
        
        return result;
    }
    
    /**
     * Process text with automatic language detection and switching
     * 자동 언어 감지 및 전환으로 텍스트 처리
     */
    processText(input) {
        // 입력 검증
        if (!input || typeof input !== 'string' || input.length === 0) {
            return '';
        }
        
        // 최대 길이 제한
        const MAX_INPUT_LENGTH = 100000;
        if (input.length > MAX_INPUT_LENGTH) {
            console.warn(`입력이 너무 깁니다: ${input.length} > ${MAX_INPUT_LENGTH}`);
            input = input.substring(0, MAX_INPUT_LENGTH);
        }
        
        let result = '';
        let currentMode = null; // 'korean' or 'english'
        let buffer = '';
        let processedChars = 0;
        
        try {
            for (let i = 0; i < input.length; i++) {
                // 무한 루프 방지
                if (++processedChars > MAX_INPUT_LENGTH) {
                    console.warn('최대 처리 문자 수 초과');
                    break;
                }
                
                const char = input[i];
                
                // null 또는 undefined 문자 건너뛰기
                if (!char) {
                    continue;
                }
                
                const isKorean = this.isHangul(char);
                const isEnglish = this.isAscii(char);
                
                // Determine the mode for this character
                let charMode = null;
                if (isKorean) {
                    charMode = 'korean';
                } else if (isEnglish) {
                    charMode = 'english';
                } else {
                    // Other characters (symbols, etc.) - maintain current mode
                    charMode = currentMode || 'english';
                }
                
                // Check if we need to switch modes
                if (currentMode !== null && currentMode !== charMode) {
                    // Process buffered text before switching
                    if (buffer.length > 0) {
                        if (currentMode === 'korean') {
                            result += this.hangulToQwerty(buffer);
                        } else {
                            result += buffer;
                        }
                        buffer = '';
                    }
                    
                    // Add toggle marker (안전한 길이 확인)
                    if (result.length + this.HANGUL_TOGGLE.length < MAX_INPUT_LENGTH * 2) {
                        result += this.HANGUL_TOGGLE;
                    }
                }
                
                // Update current mode
                currentMode = charMode;
                
                // Add character to buffer (버퍼 크기 제한)
                if (buffer.length < 10000) {
                    buffer += char;
                }
            }
            
            // Process remaining buffer
            if (buffer.length > 0) {
                if (currentMode === 'korean') {
                    result += this.hangulToQwerty(buffer);
                } else {
                    result += buffer;
                }
            }
        } catch (error) {
            console.error('processText 오류:', error);
            return input; // 실패 시 원본 반환
        }
        
        return result;
    }
    
    /**
     * Format input for ESP32 HID typing with language detection
     * ESP32 HID 타이핑을 위한 언어 감지를 포함한 입력 포맷
     */
    formatForESP32(input) {
        // Process the text with automatic language switching
        const processed = this.processText(input);
        
        // Return formatted data for ESP32
        return {
            text: processed,
            hasToggle: processed.includes(this.HANGUL_TOGGLE),
            toggleMarker: this.HANGUL_TOGGLE
        };
    }
    
    /**
     * Test the preprocessor with various inputs
     * 다양한 입력으로 전처리기 테스트
     */
    runTests() {
        const testCases = [
            {
                input: "Hello 안녕하세요 World!",
                expected: "Hello ⌨HANGUL_TOGGLE⌨dkssudgktpdy ⌨HANGUL_TOGGLE⌨World!"
            },
            {
                input: "대한민국 Korea 화이팅!",
                expected: "eogksalsrnr ⌨HANGUL_TOGGLE⌨Korea ⌨HANGUL_TOGGLE⌨ghkdlxld!"
            },
            {
                input: "윤",
                expected: "dbs"
            },
            {
                input: "Test 되 vs 돼 example",
                expected: "Test ⌨HANGUL_TOGGLE⌨enl vs eho ⌨HANGUL_TOGGLE⌨example"
            },
            {
                input: "맑은 sky 넓은 sea",
                expected: "akfrms ⌨HANGUL_TOGGLE⌨sky ⌨HANGUL_TOGGLE⌨sjfdms ⌨HANGUL_TOGGLE⌨sea"
            }
        ];
        
        console.log("Running Hangul Preprocessor Tests...\n");
        
        testCases.forEach((test, index) => {
            const result = this.processText(test.input);
            const passed = result === test.expected;
            
            console.log(`Test ${index + 1}: ${passed ? '✅ PASS' : '❌ FAIL'}`);
            console.log(`  Input:    "${test.input}"`);
            console.log(`  Expected: "${test.expected}"`);
            console.log(`  Result:   "${result}"`);
            console.log('');
        });
    }
}

// Export for use in browser or Node.js
if (typeof module !== 'undefined' && module.exports) {
    module.exports = HangulPreprocessor;
} else if (typeof window !== 'undefined') {
    window.HangulPreprocessor = HangulPreprocessor;
}