import { QWERTY_TO_JAMO } from './constants.js';

// 역방향 매핑 (자모 → QWERTY)
const JAMO_TO_QWERTY = {};
Object.keys(QWERTY_TO_JAMO).forEach(key => {
    JAMO_TO_QWERTY[QWERTY_TO_JAMO[key]] = key;
});

// 한글 유니코드 분해 함수
export function decomposeHangul(char) {
    const code = char.charCodeAt(0);
    
    // 한글 완성형 범위 체크 (가-힣)
    if (code < 0xAC00 || code > 0xD7A3) {
        return null;
    }
    
    const base = code - 0xAC00;
    const cho = Math.floor(base / 588);           // 초성
    const jung = Math.floor((base % 588) / 28);   // 중성
    const jong = base % 28;                       // 종성
    
    // 자모 배열
    const chosung = ['ㄱ','ㄲ','ㄴ','ㄷ','ㄸ','ㄹ','ㅁ','ㅂ','ㅃ','ㅅ','ㅆ','ㅇ','ㅈ','ㅉ','ㅊ','ㅋ','ㅌ','ㅍ','ㅎ'];
    const jungsung = ['ㅏ','ㅐ','ㅑ','ㅒ','ㅓ','ㅔ','ㅕ','ㅖ','ㅗ','ㅘ','ㅙ','ㅚ','ㅛ','ㅜ','ㅝ','ㅞ','ㅟ','ㅠ','ㅡ','ㅢ','ㅣ'];
    const jongsung = ['','ㄱ','ㄲ','ㄱㅅ','ㄴ','ㄴㅈ','ㄴㅎ','ㄷ','ㄹ','ㄹㄱ','ㄹㅁ','ㄹㅂ','ㄹㅅ','ㄹㅌ','ㄹㅍ','ㄹㅎ','ㅁ','ㅂ','ㅂㅅ','ㅅ','ㅆ','ㅇ','ㅈ','ㅊ','ㅋ','ㅌ','ㅍ','ㅎ'];
    
    const result = [];
    result.push(chosung[cho]);
    result.push(jungsung[jung]);
    if (jong > 0) {
        const jongChar = jongsung[jong];
        if (jongChar.length === 2) {
            // 복합 종성 (예: ㄱㅅ)
            result.push(jongChar[0]);
            result.push(jongChar[1]);
        } else {
            result.push(jongChar);
        }
    }
    
    return result;
}

// 자모를 QWERTY 키로 변환
export function jamoToQwerty(jamo) {
    // Check if the character exists in the mapping
    if (JAMO_TO_QWERTY.hasOwnProperty(jamo)) {
        return JAMO_TO_QWERTY[jamo];
    }
    
    // For unmapped characters, only return if it's a safe printable character
    const code = jamo.charCodeAt(0);
    if ((code >= 32 && code <= 126) || (code >= 0xAC00 && code <= 0xD7A3)) {
        return jamo;
    }
    
    // Filter out problematic characters that might cause issues
    return '';
}

// 한글 텍스트를 자모 키 조합으로 변환
export function convertHangulToJamoKeys(text) {
    let result = '';
    
    for (let char of text) {
        const jamos = decomposeHangul(char);
        if (jamos) {
            // 한글인 경우 자모로 분해 후 QWERTY 키로 변환
            for (let jamo of jamos) {
                result += jamoToQwerty(jamo);
            }
        } else {
            // 한글이 아닌 경우 그대로
            result += char;
        }
    }
    
    return result;
}

// 텍스트 타입 분석
export function analyzeText(text) {
    let hasKorean = false;
    let hasEnglish = false;
    let hasSpecial = false;
    
    for (let char of text) {
        const code = char.charCodeAt(0);
        if (code >= 0xAC00 && code <= 0xD7A3) {
            hasKorean = true;
        } else if ((code >= 65 && code <= 90) || (code >= 97 && code <= 122)) {
            hasEnglish = true;
        } else if (code >= 32 && code <= 126) {
            hasSpecial = true;
        }
    }
    
    if (hasKorean && hasEnglish) return 'mixed';
    if (hasKorean) return 'korean';
    if (hasEnglish || hasSpecial) return 'english';
    return 'unknown';
}