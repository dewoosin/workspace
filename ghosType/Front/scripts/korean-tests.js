/**
 * Comprehensive Korean Converter Test Suite
 * Tests all aspects of Hangul to QWERTY conversion
 */

import { 
    decomposeHangul, 
    jamoToQwerty, 
    convertHangulToJamoKeys, 
    analyzeText,
    validateJamo,
    validateSyllable,
    canTypeOnKeyboard,
    runDiagnostics
} from './korean-converter-improved.js';

import { 
    QWERTY_TO_JAMO, 
    JAMO_TO_QWERTY,
    DEBUG_UTILS 
} from './korean-mappings.js';

// === Test Data ===
const TEST_CASES = {
    // Basic syllables
    basic: [
        { char: '가', expected: 'rk', description: '가 (ga)' },
        { char: '나', expected: 'sk', description: '나 (na)' },
        { char: '다', expected: 'ek', description: '다 (da)' },
        { char: '라', expected: 'fk', description: '라 (ra)' },
        { char: '마', expected: 'ak', description: '마 (ma)' },
        { char: '바', expected: 'qk', description: '바 (ba)' },
        { char: '사', expected: 'tk', description: '사 (sa)' },
        { char: '아', expected: 'dk', description: '아 (a)' },
        { char: '자', expected: 'wk', description: '자 (ja)' },
        { char: '차', expected: 'ck', description: '차 (cha)' },
        { char: '카', expected: 'zk', description: '카 (ka)' },
        { char: '타', expected: 'xk', description: '타 (ta)' },
        { char: '파', expected: 'vk', description: '파 (pa)' },
        { char: '하', expected: 'gk', description: '하 (ha)' }
    ],
    
    // Complex vowels
    complexVowels: [
        { char: '과', expected: 'rhk', description: '과 (gwa)' },
        { char: '괘', expected: 'rho', description: '괘 (gwae)' },
        { char: '괴', expected: 'rhl', description: '괴 (goe)' },
        { char: '궈', expected: 'rnjr', description: '궈 (gweo)' },
        { char: '궤', expected: 'rnjp', description: '궤 (gwe)' },
        { char: '귀', expected: 'rnn', description: '귀 (gwi)' },
        { char: '의', expected: 'dml', description: '의 (ui)' }
    ],
    
    // Final consonants
    finalConsonants: [
        { char: '각', expected: 'rkr', description: '각 (gak)' },
        { char: '간', expected: 'rks', description: '간 (gan)' },
        { char: '갈', expected: 'rkf', description: '갈 (gal)' },
        { char: '감', expected: 'rka', description: '감 (gam)' },
        { char: '갑', expected: 'rkq', description: '갑 (gap)' },
        { char: '갓', expected: 'rkt', description: '갓 (gat)' },
        { char: '강', expected: 'rkd', description: '강 (gang)' },
        { char: '갖', expected: 'rkw', description: '갖 (gaj)' },
        { char: '갚', expected: 'rkQ', description: '갚 (gap)' },
        { char: '같', expected: 'rkT', description: '같 (gat)' }
    ],
    
    // Compound final consonants
    compoundFinals: [
        { char: '닭', expected: 'ekfr', description: '닭 (dak) - ㄹㄱ' },
        { char: '삶', expected: 'tka', description: '삶 (sam) - ㄹㅁ' },
        { char: '앉', expected: 'dksw', description: '앉 (anj) - ㄴㅈ' },
        { char: '읽', expected: 'dfr', description: '읽 (ilk) - ㄹㄱ' },
        { char: '없', expected: 'djqt', description: '없 (eop) - ㅂㅅ' }
    ],
    
    // Special problematic syllables
    special: [
        { char: '돼', expected: 'eho', description: '돼 (dwe)' },
        { char: '뭐', expected: 'anjr', description: '뭐 (mwo)' },
        { char: '뒤', expected: 'ehn', description: '뒤 (dwi)' },
        { char: '왜', expected: 'hko', description: '왜 (wae)' },
        { char: '웨', expected: 'hpj', description: '웨 (we)' },
        { char: '위', expected: 'hn', description: '위 (wi)' },
        { char: '외', expected: 'hl', description: '외 (oe)' }
    ],
    
    // Double consonants
    doubleConsonants: [
        { char: '까', expected: 'Rk', description: '까 (kka)' },
        { char: '따', expected: 'Ek', description: '따 (tta)' },
        { char: '빠', expected: 'Qk', description: '빠 (ppa)' },
        { char: '싸', expected: 'Tk', description: '싸 (ssa)' },
        { char: '짜', expected: 'Wk', description: '짜 (jja)' }
    ],
    
    // Mixed text
    mixed: [
        { text: 'Hello안녕', description: 'Mixed English-Korean' },
        { text: '안녕Hello', description: 'Mixed Korean-English' },
        { text: '한글ABC123', description: 'Korean-English-Numbers' },
        { text: '!@#한글$%^', description: 'Special chars with Korean' }
    ]
};

// === Test Functions ===
export function testBasicMapping() {
    console.group('🔤 Testing Basic QWERTY ↔ Jamo Mapping');
    
    let passed = 0;
    let failed = 0;
    
    // Test forward mapping
    Object.entries(QWERTY_TO_JAMO).forEach(([qwerty, jamo]) => {
        const reverse = JAMO_TO_QWERTY[jamo];
        if (reverse === qwerty) {
            passed++;
        } else {
            failed++;
            console.error(`❌ Mapping error: ${qwerty} -> ${jamo} -> ${reverse}`);
        }
    });
    
    console.log(`✅ Passed: ${passed}, ❌ Failed: ${failed}`);
    console.groupEnd();
    return { passed, failed };
}

export function testSyllableDecomposition() {
    console.group('🔍 Testing Syllable Decomposition');
    
    let passed = 0;
    let failed = 0;
    
    ['basic', 'complexVowels', 'finalConsonants', 'compoundFinals', 'doubleConsonants'].forEach(category => {
        console.log(`\n--- ${category.toUpperCase()} ---`);
        
        TEST_CASES[category].forEach(testCase => {
            try {
                const jamos = decomposeHangul(testCase.char);
                const result = convertHangulToJamoKeys(testCase.char);
                
                if (result === testCase.expected) {
                    console.log(`✅ ${testCase.description}: "${result}"`);
                    passed++;
                } else {
                    console.error(`❌ ${testCase.description}: expected "${testCase.expected}", got "${result}"`);
                    console.log(`   Jamos: [${jamos?.join(', ')}]`);
                    failed++;
                }
            } catch (error) {
                console.error(`💥 ${testCase.description}: Error - ${error.message}`);
                failed++;
            }
        });
    });
    
    console.log(`\n✅ Passed: ${passed}, ❌ Failed: ${failed}`);
    console.groupEnd();
    return { passed, failed };
}

export function testSpecialCases() {
    console.group('⭐ Testing Special Cases');
    
    let passed = 0;
    let failed = 0;
    
    TEST_CASES.special.forEach(testCase => {
        try {
            const result = convertHangulToJamoKeys(testCase.char);
            const canType = canTypeOnKeyboard(testCase.char);
            
            console.log(`${testCase.description}:`);
            console.log(`  Input: ${testCase.char}`);
            console.log(`  Output: "${result}"`);
            console.log(`  Expected: "${testCase.expected}"`);
            console.log(`  Typable: ${canType}`);
            
            if (result === testCase.expected) {
                console.log(`  ✅ PASS`);
                passed++;
            } else {
                console.log(`  ❌ FAIL`);
                failed++;
            }
        } catch (error) {
            console.error(`💥 ${testCase.description}: Error - ${error.message}`);
            failed++;
        }
        console.log('');
    });
    
    console.log(`✅ Passed: ${passed}, ❌ Failed: ${failed}`);
    console.groupEnd();
    return { passed, failed };
}

export function testMixedText() {
    console.group('🌐 Testing Mixed Text');
    
    TEST_CASES.mixed.forEach(testCase => {
        try {
            const analysis = analyzeText(testCase.text);
            const result = convertHangulToJamoKeys(testCase.text);
            
            console.log(`${testCase.description}:`);
            console.log(`  Input: "${testCase.text}"`);
            console.log(`  Output: "${result}"`);
            console.log(`  Analysis:`, analysis);
            console.log('');
        } catch (error) {
            console.error(`💥 ${testCase.description}: Error - ${error.message}`);
        }
    });
    
    console.groupEnd();
}

export function testEdgeCases() {
    console.group('🚨 Testing Edge Cases');
    
    const edgeCases = [
        { input: '', description: 'Empty string' },
        { input: null, description: 'Null input' },
        { input: undefined, description: 'Undefined input' },
        { input: '123', description: 'Numbers only' },
        { input: '!@#$%', description: 'Special characters' },
        { input: 'ㄱㄴㄷ', description: 'Individual jamo' },
        { input: '𝕳𝖊𝖑𝖑𝖔', description: 'Unicode mathematical characters' },
        { input: '😀😃😄', description: 'Emoji characters' }
    ];
    
    edgeCases.forEach(testCase => {
        try {
            const analysis = analyzeText(testCase.input);
            const result = convertHangulToJamoKeys(testCase.input);
            
            console.log(`${testCase.description}:`);
            console.log(`  Input: ${testCase.input}`);
            console.log(`  Output: "${result}"`);
            console.log(`  Type: ${analysis.type || analysis}`);
            console.log('');
        } catch (error) {
            console.error(`💥 ${testCase.description}: Error - ${error.message}`);
        }
    });
    
    console.groupEnd();
}

export function testPerformance() {
    console.group('⚡ Performance Testing');
    
    const longText = '안녕하세요 반갑습니다 한글 테스트입니다 '.repeat(100);
    const iterations = 1000;
    
    console.time('Conversion Performance');
    for (let i = 0; i < iterations; i++) {
        convertHangulToJamoKeys(longText);
    }
    console.timeEnd('Conversion Performance');
    
    console.log(`Processed ${longText.length * iterations} characters in ${iterations} iterations`);
    console.groupEnd();
}

export function runAllTests() {
    console.log('🧪 Starting Comprehensive Korean Converter Tests\n');
    
    const results = {
        basicMapping: testBasicMapping(),
        syllableDecomposition: testSyllableDecomposition(),
        specialCases: testSpecialCases()
    };
    
    testMixedText();
    testEdgeCases();
    testPerformance();
    
    const diagnostics = runDiagnostics();
    
    // Summary
    console.group('📊 Test Summary');
    const totalPassed = Object.values(results).reduce((sum, result) => sum + (result.passed || 0), 0);
    const totalFailed = Object.values(results).reduce((sum, result) => sum + (result.failed || 0), 0);
    
    console.log(`Total Tests: ${totalPassed + totalFailed}`);
    console.log(`✅ Passed: ${totalPassed}`);
    console.log(`❌ Failed: ${totalFailed}`);
    console.log(`Success Rate: ${((totalPassed / (totalPassed + totalFailed)) * 100).toFixed(1)}%`);
    console.log('\nDiagnostics:', diagnostics);
    console.groupEnd();
    
    return {
        results,
        diagnostics,
        summary: {
            totalPassed,
            totalFailed,
            successRate: (totalPassed / (totalPassed + totalFailed)) * 100
        }
    };
}

// === Interactive Testing in Browser Console ===
if (typeof window !== 'undefined') {
    window.koreanTests = {
        runAllTests,
        testBasicMapping,
        testSyllableDecomposition,
        testSpecialCases,
        testMixedText,
        testEdgeCases,
        testPerformance,
        
        // Quick test function
        quickTest: (text) => {
            console.log(`Input: "${text}"`);
            const result = convertHangulToJamoKeys(text);
            console.log(`Output: "${result}"`);
            const analysis = analyzeText(text);
            console.log('Analysis:', analysis);
            return result;
        }
    };
    
    console.log('Korean tests available at window.koreanTests');
    console.log('Run window.koreanTests.runAllTests() for comprehensive testing');
}