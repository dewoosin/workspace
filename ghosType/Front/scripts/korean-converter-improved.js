/**
 * Improved Korean Text Converter
 * Comprehensive Hangul syllable decomposition and QWERTY mapping
 * 
 * Features:
 * - Accurate Unicode decomposition
 * - Proper compound jongsung handling
 * - Complex vowel decomposition
 * - Special syllable support
 * - Comprehensive error handling
 * - Extensive validation
 */

import {
    QWERTY_TO_JAMO,
    JAMO_TO_QWERTY,
    JAMO_ARRAYS,
    COMPOUND_JONGSUNG,
    COMPOUND_VOWELS,
    SPECIAL_SYLLABLES,
    UNICODE_RANGES,
    VALIDATION,
    DEBUG_UTILS
} from './korean-mappings.js';

// === Enhanced Hangul Decomposition ===
export function decomposeHangul(char) {
    const code = char.charCodeAt(0);
    
    // Check if character is in Hangul syllables range
    if (code < UNICODE_RANGES.HANGUL_SYLLABLES_START || code > UNICODE_RANGES.HANGUL_SYLLABLES_END) {
        // Check if it's already a jamo character
        if (code >= UNICODE_RANGES.HANGUL_JAMO_START && code <= UNICODE_RANGES.HANGUL_JAMO_END) {
            return [char]; // Return as single jamo
        }
        return null; // Not a Hangul character
    }
    
    // Handle special syllables first
    if (SPECIAL_SYLLABLES.hasOwnProperty(char)) {
        return decomposeSpecialSyllable(char);
    }
    
    // Standard Unicode decomposition
    const syllableIndex = code - UNICODE_RANGES.HANGUL_SYLLABLES_START;
    
    const chosungIndex = Math.floor(syllableIndex / UNICODE_RANGES.JUNGSUNG_JONGSUNG_COUNT);
    const jungsungIndex = Math.floor((syllableIndex % UNICODE_RANGES.JUNGSUNG_JONGSUNG_COUNT) / UNICODE_RANGES.JONGSUNG_COUNT);
    const jongsungIndex = syllableIndex % UNICODE_RANGES.JONGSUNG_COUNT;
    
    // Validate indices
    if (chosungIndex >= JAMO_ARRAYS.CHOSUNG.length || 
        jungsungIndex >= JAMO_ARRAYS.JUNGSUNG.length ||
        jongsungIndex >= JAMO_ARRAYS.JONGSUNG.length) {
        console.error(`Invalid decomposition indices for character '${char}' (U+${code.toString(16).toUpperCase()})`);
        return null;
    }
    
    const result = [];
    
    // Add chosung (initial consonant)
    const chosung = JAMO_ARRAYS.CHOSUNG[chosungIndex];
    result.push(chosung);
    
    // Add jungsung (vowel) - handle compound vowels
    const jungsung = JAMO_ARRAYS.JUNGSUNG[jungsungIndex];
    if (COMPOUND_VOWELS.hasOwnProperty(jungsung)) {
        result.push(...COMPOUND_VOWELS[jungsung]);
    } else {
        result.push(jungsung);
    }
    
    // Add jongsung (final consonant) if present - handle compound consonants
    if (jongsungIndex > 0) {
        const jongsung = JAMO_ARRAYS.JONGSUNG[jongsungIndex];
        if (COMPOUND_JONGSUNG.hasOwnProperty(jongsung)) {
            result.push(...COMPOUND_JONGSUNG[jongsung]);
        } else {
            result.push(jongsung);
        }
    }
    
    return result;
}

// === Special Syllable Decomposition ===
function decomposeSpecialSyllable(char) {
    // Handle commonly problematic syllables
    switch (char) {
        case '돼':
            // 돼 = ㄷ + ㅗ + ㅏ + ㅔ (되 + ㅗ/ㅏ combination)
            return ['ㄷ', 'ㅗ', 'ㅐ'];
            
        case '뭐':
            // 뭐 = ㅁ + ㅜ + ㅓ
            return ['ㅁ', 'ㅜ', 'ㅓ'];
            
        case '뒤':
            // 뒤 = ㄷ + ㅜ + ㅣ
            return ['ㄷ', 'ㅜ', 'ㅣ'];
            
        case '왜':
            // 왜 = ㅇ + ㅗ + ㅐ
            return ['ㅇ', 'ㅗ', 'ㅐ'];
            
        case '웨':
            // 웨 = ㅇ + ㅜ + ㅔ
            return ['ㅇ', 'ㅜ', 'ㅔ'];
            
        case '위':
            // 위 = ㅇ + ㅜ + ㅣ
            return ['ㅇ', 'ㅜ', 'ㅣ'];
            
        case '외':
            // 외 = ㅇ + ㅗ + ㅣ
            return ['ㅇ', 'ㅗ', 'ㅣ'];
            
        case '의':
            // 의 = ㅇ + ㅡ + ㅣ
            return ['ㅇ', 'ㅡ', 'ㅣ'];
            
        default:
            // Fallback to standard decomposition
            return null;
    }
}

// === Enhanced Jamo to QWERTY Conversion ===
export function jamoToQwerty(jamo) {
    // Direct mapping lookup
    if (JAMO_TO_QWERTY.hasOwnProperty(jamo)) {
        return JAMO_TO_QWERTY[jamo];
    }
    
    // Handle edge cases
    const code = jamo.charCodeAt(0);
    
    // Safe ASCII characters (pass through)
    if (code >= 32 && code <= 126) {
        return jamo;
    }
    
    // Hangul syllables (shouldn't happen but handle gracefully)
    if (code >= UNICODE_RANGES.HANGUL_SYLLABLES_START && code <= UNICODE_RANGES.HANGUL_SYLLABLES_END) {
        console.warn(`Unexpected Hangul syllable '${jamo}' in jamo conversion`);
        return jamo; // Pass through
    }
    
    // Unknown jamo - log for debugging
    if (code >= UNICODE_RANGES.HANGUL_JAMO_START && code <= UNICODE_RANGES.HANGUL_JAMO_END) {
        console.warn(`Unmapped jamo character '${jamo}' (U+${code.toString(16).toUpperCase()})`);
    }
    
    // Filter out problematic characters
    return '';
}

// === Enhanced Text Conversion ===
export function convertHangulToJamoKeys(text) {
    if (!text || typeof text !== 'string') {
        return '';
    }
    
    let result = '';
    const errors = [];
    
    for (let i = 0; i < text.length; i++) {
        const char = text[i];
        
        try {
            const jamos = decomposeHangul(char);
            
            if (jamos) {
                // Convert each jamo to QWERTY
                for (const jamo of jamos) {
                    const qwertyKey = jamoToQwerty(jamo);
                    if (qwertyKey) {
                        result += qwertyKey;
                    } else {
                        errors.push(`Failed to map jamo '${jamo}' from character '${char}' at position ${i}`);
                    }
                }
            } else {
                // Check if it's a single jamo character
                const qwertyKey = jamoToQwerty(char);
                if (qwertyKey) {
                    result += qwertyKey;
                } else {
                    // Non-Hangul character - pass through
                    result += char;
                }
            }
        } catch (error) {
            console.error(`Error processing character '${char}' at position ${i}:`, error);
            errors.push(`Error processing '${char}': ${error.message}`);
            result += char; // Fallback: include original character
        }
    }
    
    // Log errors if any
    if (errors.length > 0) {
        console.warn('Conversion errors encountered:', errors);
    }
    
    return result;
}

// === Enhanced Text Analysis ===
export function analyzeText(text) {
    if (!text || typeof text !== 'string') {
        return 'unknown';
    }
    
    let hasKorean = false;
    let hasEnglish = false;
    let hasSpecial = false;
    let hasUnsupported = false;
    
    const stats = {
        koreanChars: 0,
        englishChars: 0,
        specialChars: 0,
        unsupportedChars: 0,
        totalChars: text.length
    };
    
    for (const char of text) {
        const code = char.charCodeAt(0);
        
        if (code >= UNICODE_RANGES.HANGUL_SYLLABLES_START && code <= UNICODE_RANGES.HANGUL_SYLLABLES_END) {
            hasKorean = true;
            stats.koreanChars++;
        } else if ((code >= 65 && code <= 90) || (code >= 97 && code <= 122)) {
            hasEnglish = true;
            stats.englishChars++;
        } else if (code >= 32 && code <= 126) {
            hasSpecial = true;
            stats.specialChars++;
        } else if (code === 10 || code === 13 || code === 9) {
            // 엔터키(\n), 캐리지 리턴(\r), 탭(\t)은 특수 문자로 분류
            hasSpecial = true;
            stats.specialChars++;
        } else {
            hasUnsupported = true;
            stats.unsupportedChars++;
        }
    }
    
    // Determine primary type
    let type;
    if (hasKorean && hasEnglish) {
        type = 'mixed';
    } else if (hasKorean) {
        type = 'korean';
    } else if (hasEnglish || hasSpecial) {
        type = 'english';
    } else {
        type = 'unknown';
    }
    
    return {
        type,
        stats,
        hasUnsupported
    };
}

// === Validation Functions ===
export function validateJamo(jamo) {
    return VALIDATION.MAPPABLE_JAMO.has(jamo);
}

export function validateSyllable(char) {
    const code = char.charCodeAt(0);
    return (code >= UNICODE_RANGES.HANGUL_SYLLABLES_START && 
            code <= UNICODE_RANGES.HANGUL_SYLLABLES_END) ||
           (code >= UNICODE_RANGES.HANGUL_JAMO_START && 
            code <= UNICODE_RANGES.HANGUL_JAMO_END);
}

export function canTypeOnKeyboard(char) {
    try {
        const jamos = decomposeHangul(char);
        if (!jamos) return false;
        
        return jamos.every(jamo => validateJamo(jamo));
    } catch (error) {
        console.error(`Error checking keyboard support for '${char}':`, error);
        return false;
    }
}

// === Testing and Debugging ===
export function runDiagnostics() {
    console.group('Korean Converter Diagnostics');
    
    // Test basic mappings
    console.log('Testing QWERTY to Jamo mappings...');
    Object.entries(QWERTY_TO_JAMO).forEach(([key, jamo]) => {
        const reversed = JAMO_TO_QWERTY[jamo];
        if (reversed !== key) {
            console.error(`Mapping mismatch: ${key} -> ${jamo} -> ${reversed}`);
        }
    });
    
    // Test decomposition
    console.log('Testing syllable decomposition...');
    DEBUG_UTILS.TEST_SYLLABLES.forEach(syllable => {
        try {
            const jamos = decomposeHangul(syllable);
            const converted = convertHangulToJamoKeys(syllable);
            console.log(`${syllable} -> [${jamos?.join(', ')}] -> "${converted}"`);
        } catch (error) {
            console.error(`Failed to process '${syllable}':`, error);
        }
    });
    
    // Test special cases
    console.log('Testing special syllables...');
    DEBUG_UTILS.PROBLEM_CHARS.forEach(char => {
        const jamos = decomposeHangul(char);
        const converted = convertHangulToJamoKeys(char);
        const canType = canTypeOnKeyboard(char);
        console.log(`${char} -> [${jamos?.join(', ')}] -> "${converted}" (typable: ${canType})`);
    });
    
    console.groupEnd();
    
    return {
        mappingCount: Object.keys(QWERTY_TO_JAMO).length,
        reverseMappingCount: Object.keys(JAMO_TO_QWERTY).length,
        specialSyllables: Object.keys(SPECIAL_SYLLABLES).length,
        compoundJongsung: Object.keys(COMPOUND_JONGSUNG).length,
        compoundVowels: Object.keys(COMPOUND_VOWELS).length
    };
}

// === Backward Compatibility ===
// Export with original function names for compatibility
export { decomposeHangul as decomposeHangul };
export { jamoToQwerty as jamoToQwerty };
export { convertHangulToJamoKeys as convertHangulToJamoKeys };
export { analyzeText as analyzeText };