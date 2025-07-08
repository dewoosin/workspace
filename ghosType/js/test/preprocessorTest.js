/**
 * Comprehensive test suite for Hangul Preprocessor
 * 한글 전처리기 종합 테스트 스위트
 */

const HangulPreprocessor = require('../hangulPreprocessor.js');

class PreprocessorTest {
    constructor() {
        this.preprocessor = new HangulPreprocessor();
        this.passCount = 0;
        this.failCount = 0;
    }
    
    /**
     * Run all tests
     * 모든 테스트 실행
     */
    runAllTests() {
        console.log('🧪 Running Hangul Preprocessor Test Suite\n');
        console.log('='*50 + '\n');
        
        this.testBasicConversion();
        this.testLanguageSwitching();
        this.testComplexCases();
        this.testEdgeCases();
        this.testPerformance();
        
        console.log('\n' + '='*50);
        console.log(`\n📊 Test Results: ${this.passCount} passed, ${this.failCount} failed`);
        console.log(`✨ Success Rate: ${(this.passCount / (this.passCount + this.failCount) * 100).toFixed(1)}%\n`);
    }
    
    /**
     * Test basic Hangul to QWERTY conversion
     * 기본 한글-QWERTY 변환 테스트
     */
    testBasicConversion() {
        console.log('1️⃣ Basic Conversion Tests\n');
        
        const tests = [
            { hangul: '가', qwerty: 'rk', desc: 'ㄱ + ㅏ' },
            { hangul: '윤', qwerty: 'dbs', desc: 'ㅇ + ㅠ + ㄴ' },
            { hangul: '하늘', qwerty: 'gksrmf', desc: '하 + 늘' },
            { hangul: '되', qwerty: 'enl', desc: 'ㄷ + ㅚ' },
            { hangul: '돼', qwerty: 'eho', desc: 'ㄷ + ㅙ' },
            { hangul: '맑', qwerty: 'akfr', desc: 'ㅁ + ㅏ + ㄺ' },
            { hangul: '띄', qwerty: 'Eml', desc: 'ㄸ + ㅢ' },
            { hangul: '넓', qwerty: 'spfq', desc: 'ㄴ + ㅓ + ㄼ' }
        ];
        
        tests.forEach(test => {
            const result = this.preprocessor.hangulToQwerty(test.hangul);
            this.assertTest(
                result === test.qwerty,
                `"${test.hangul}" → "${test.qwerty}" (${test.desc})`,
                result
            );
        });
    }
    
    /**
     * Test automatic language switching
     * 자동 언어 전환 테스트
     */
    testLanguageSwitching() {
        console.log('\n2️⃣ Language Switching Tests\n');
        
        const tests = [
            {
                input: 'Hello 안녕하세요 World!',
                expected: 'Hello ⌨HANGUL_TOGGLE⌨dkssudgktpdy ⌨HANGUL_TOGGLE⌨World!',
                desc: 'English → Korean → English'
            },
            {
                input: '대한민국 Korea 화이팅!',
                expected: 'eogksalsrnr ⌨HANGUL_TOGGLE⌨Korea ⌨HANGUL_TOGGLE⌨ghkdlxld!',
                desc: 'Korean → English → Korean'
            },
            {
                input: 'Test 되 vs 돼 example',
                expected: 'Test ⌨HANGUL_TOGGLE⌨enl vs eho ⌨HANGUL_TOGGLE⌨example',
                desc: '되/돼 distinction with switching'
            },
            {
                input: '123 가나다 ABC',
                expected: '123 ⌨HANGUL_TOGGLE⌨rkskek ⌨HANGUL_TOGGLE⌨ABC',
                desc: 'Numbers and switching'
            }
        ];
        
        tests.forEach(test => {
            const result = this.preprocessor.processText(test.input);
            this.assertTest(
                result === test.expected,
                test.desc,
                result
            );
        });
    }
    
    /**
     * Test complex mixed text cases
     * 복잡한 혼합 텍스트 케이스 테스트
     */
    testComplexCases() {
        console.log('\n3️⃣ Complex Mixed Text Tests\n');
        
        const tests = [
            {
                input: '맑은 sky 넓은 sea',
                expected: 'akfrms ⌨HANGUL_TOGGLE⌨sky ⌨HANGUL_TOGGLE⌨sjfdms ⌨HANGUL_TOGGLE⌨sea',
                desc: 'Complex jamo with English words'
            },
            {
                input: 'email@한글.com 입니다',
                expected: 'email@⌨HANGUL_TOGGLE⌨gksrmf⌨HANGUL_TOGGLE⌨.com ⌨HANGUL_TOGGLE⌨dlqslek',
                desc: 'Email with Korean domain'
            },
            {
                input: '(안녕) [Hello] {하세요}',
                expected: '(⌨HANGUL_TOGGLE⌨dkssud⌨HANGUL_TOGGLE⌨) [Hello] {⌨HANGUL_TOGGLE⌨gktpdy⌨HANGUL_TOGGLE⌨}',
                desc: 'Brackets and parentheses'
            }
        ];
        
        tests.forEach(test => {
            const result = this.preprocessor.processText(test.input);
            this.assertTest(
                result === test.expected,
                test.desc,
                result
            );
        });
    }
    
    /**
     * Test edge cases and special scenarios
     * 예외 케이스 및 특수 시나리오 테스트
     */
    testEdgeCases() {
        console.log('\n4️⃣ Edge Case Tests\n');
        
        const tests = [
            {
                input: '',
                expected: '',
                desc: 'Empty string'
            },
            {
                input: 'English only text',
                expected: 'English only text',
                desc: 'English only (no toggle)'
            },
            {
                input: '한글만있는텍스트',
                expected: 'gksrmfaksdlTsmsxprtmxm',
                desc: 'Korean only (no toggle)'
            },
            {
                input: '!!!@@@###$$$%%%',
                expected: '!!!@@@###$$$%%%',
                desc: 'Special characters only'
            },
            {
                input: '한a글b을c섞d어e서f',
                expected: 'gks⌨HANGUL_TOGGLE⌨a⌨HANGUL_TOGGLE⌨rmf⌨HANGUL_TOGGLE⌨b⌨HANGUL_TOGGLE⌨dmf⌨HANGUL_TOGGLE⌨c⌨HANGUL_TOGGLE⌨tjr⌨HANGUL_TOGGLE⌨d⌨HANGUL_TOGGLE⌨dj⌨HANGUL_TOGGLE⌨e⌨HANGUL_TOGGLE⌨tj⌨HANGUL_TOGGLE⌨f',
                desc: 'Alternating single characters'
            }
        ];
        
        tests.forEach(test => {
            const result = this.preprocessor.processText(test.input);
            this.assertTest(
                result === test.expected,
                test.desc,
                result
            );
        });
    }
    
    /**
     * Test performance with large inputs
     * 대용량 입력에 대한 성능 테스트
     */
    testPerformance() {
        console.log('\n5️⃣ Performance Tests\n');
        
        // Generate large mixed text
        let largeText = '';
        for (let i = 0; i < 100; i++) {
            largeText += 'Hello 안녕하세요 World 세계 ';
        }
        
        const startTime = Date.now();
        const result = this.preprocessor.processText(largeText);
        const endTime = Date.now();
        
        const processingTime = endTime - startTime;
        const charPerMs = largeText.length / processingTime;
        
        this.assertTest(
            processingTime < 100, // Should process in under 100ms
            `Large text (${largeText.length} chars) processed in ${processingTime}ms (${charPerMs.toFixed(1)} chars/ms)`,
            ''
        );
    }
    
    /**
     * Assert test result and update counters
     * 테스트 결과 확인 및 카운터 업데이트
     */
    assertTest(condition, description, actual) {
        if (condition) {
            console.log(`  ✅ ${description}`);
            this.passCount++;
        } else {
            console.log(`  ❌ ${description}`);
            if (actual) {
                console.log(`     Got: "${actual}"`);
            }
            this.failCount++;
        }
    }
}

// Run tests if executed directly
if (require.main === module) {
    const tester = new PreprocessorTest();
    tester.runAllTests();
}

module.exports = PreprocessorTest;