/**
 * Safety Test Suite for GHOSTYPE System
 * GHOSTYPE 시스템 안전성 테스트 스위트
 */

class SafetyTestSuite {
    constructor() {
        this.preprocessor = new HangulPreprocessor();
        this.passCount = 0;
        this.failCount = 0;
    }
    
    /**
     * Run all safety tests
     * 모든 안전성 테스트 실행
     */
    runAllTests() {
        console.log('🛡️ GHOSTYPE 안전성 테스트 시작\n');
        console.log('='*60);
        
        this.testMalformedInput();
        this.testExtremeValues();
        this.testInfiniteLoops();
        this.testMemoryLimits();
        this.testInvalidCharacters();
        this.testEdgeCases();
        
        console.log('\n' + '='*60);
        console.log(`📊 테스트 결과: ${this.passCount}개 통과, ${this.failCount}개 실패`);
        console.log(`✨ 안전성 점수: ${(this.passCount / (this.passCount + this.failCount) * 100).toFixed(1)}%\n`);
    }
    
    /**
     * Test malformed input handling
     * 잘못된 입력 처리 테스트
     */
    testMalformedInput() {
        console.log('\n1️⃣ 잘못된 입력 처리 테스트');
        
        const malformedInputs = [
            null,
            undefined,
            '',
            {},
            [],
            123,
            true,
            '\x00\x01\x02',  // 제어 문자
            'a'.repeat(200000)  // 극도로 긴 문자열
        ];
        
        malformedInputs.forEach((input, index) => {
            try {
                const result = this.preprocessor.processText(input);
                this.assertTest(
                    typeof result === 'string',
                    `잘못된 입력 ${index}: 문자열 반환`,
                    typeof result
                );
            } catch (error) {
                this.assertTest(
                    false,
                    `잘못된 입력 ${index}: 예외 발생`,
                    error.message
                );
            }
        });
    }
    
    /**
     * Test extreme values
     * 극한값 테스트
     */
    testExtremeValues() {
        console.log('\n2️⃣ 극한값 테스트');
        
        const extremeTests = [
            {
                name: '매우 긴 한글 텍스트',
                input: '가'.repeat(50000),
                expectNoError: true
            },
            {
                name: '매우 긴 영어 텍스트',
                input: 'a'.repeat(50000),
                expectNoError: true
            },
            {
                name: '빈 문자들',
                input: '\n\t\r   ',
                expectNoError: true
            },
            {
                name: '특수 유니코드',
                input: '🎌🗾🇰🇷🇺🇸',
                expectNoError: true
            }
        ];
        
        extremeTests.forEach(test => {
            try {
                const startTime = Date.now();
                const result = this.preprocessor.processText(test.input);
                const endTime = Date.now();
                
                this.assertTest(
                    test.expectNoError && (endTime - startTime) < 5000,
                    `${test.name}: 5초 내 처리`,
                    `${endTime - startTime}ms`
                );
            } catch (error) {
                this.assertTest(
                    !test.expectNoError,
                    `${test.name}: 예외 처리`,
                    error.message
                );
            }
        });
    }
    
    /**
     * Test for infinite loops
     * 무한 루프 테스트
     */
    testInfiniteLoops() {
        console.log('\n3️⃣ 무한 루프 방지 테스트');
        
        const loopTests = [
            '가'.repeat(200000),  // 20만자
            'Hello '.repeat(50000),  // 30만자
            '안녕하세요 '.repeat(30000)  // 30만자
        ];
        
        loopTests.forEach((input, index) => {
            const startTime = Date.now();
            try {
                const result = this.preprocessor.processText(input);
                const endTime = Date.now();
                
                this.assertTest(
                    (endTime - startTime) < 10000,  // 10초 제한
                    `무한 루프 테스트 ${index + 1}: 시간 제한 준수`,
                    `${endTime - startTime}ms`
                );
            } catch (error) {
                this.assertTest(
                    false,
                    `무한 루프 테스트 ${index + 1}: 예외 발생`,
                    error.message
                );
            }
        });
    }
    
    /**
     * Test memory limits
     * 메모리 제한 테스트
     */
    testMemoryLimits() {
        console.log('\n4️⃣ 메모리 제한 테스트');
        
        // 메모리 사용량 모니터링
        if (typeof process !== 'undefined' && process.memoryUsage) {
            const initialMemory = process.memoryUsage().heapUsed;
            
            // 대용량 텍스트 처리
            const largeText = '가나다라마바사아자차카타파하'.repeat(10000);
            
            for (let i = 0; i < 10; i++) {
                this.preprocessor.processText(largeText);
            }
            
            const finalMemory = process.memoryUsage().heapUsed;
            const memoryIncrease = finalMemory - initialMemory;
            
            this.assertTest(
                memoryIncrease < 100 * 1024 * 1024,  // 100MB 제한
                '메모리 사용량 제한',
                `${Math.round(memoryIncrease / 1024 / 1024)}MB 증가`
            );
        } else {
            console.log('  ⚠️ Node.js 환경이 아니어서 메모리 테스트 건너뜀');
        }
    }
    
    /**
     * Test invalid characters
     * 유효하지 않은 문자 테스트
     */
    testInvalidCharacters() {
        console.log('\n5️⃣ 유효하지 않은 문자 테스트');
        
        const invalidChars = [
            '\x00',  // null
            '\x01',  // SOH
            '\x02',  // STX
            '\x7F',  // DEL
            '\uFFFE',  // 비문자
            '\uFFFF'   // 비문자
        ];
        
        invalidChars.forEach((char, index) => {
            try {
                const result = this.preprocessor.isHangul(char);
                this.assertTest(
                    result === false,
                    `유효하지 않은 문자 ${index}: 한글 아님`,
                    result
                );
            } catch (error) {
                this.assertTest(
                    false,
                    `유효하지 않은 문자 ${index}: 예외 발생`,
                    error.message
                );
            }
        });
    }
    
    /**
     * Test edge cases
     * 경계 케이스 테스트
     */
    testEdgeCases() {
        console.log('\n6️⃣ 경계 케이스 테스트');
        
        const edgeCases = [
            {
                name: '한글 범위 경계',
                input: '\uAC00\uD7A3',  // 가힣
                expectValid: true
            },
            {
                name: '한글 범위 밖',
                input: '\uABFF\uD7A4',  // 한글 범위 밖
                expectValid: true  // 처리는 되어야 함
            },
            {
                name: '빈 문자열',
                input: '',
                expectValid: true
            },
            {
                name: '공백만',
                input: '   ',
                expectValid: true
            },
            {
                name: '혼합 공백',
                input: ' \t\n\r ',
                expectValid: true
            }
        ];
        
        edgeCases.forEach(testCase => {
            try {
                const result = this.preprocessor.processText(testCase.input);
                this.assertTest(
                    testCase.expectValid,
                    testCase.name,
                    `처리됨: "${result}"`
                );
            } catch (error) {
                this.assertTest(
                    !testCase.expectValid,
                    testCase.name,
                    error.message
                );
            }
        });
    }
    
    /**
     * Assert test result
     * 테스트 결과 확인
     */
    assertTest(condition, description, actual) {
        if (condition) {
            console.log(`  ✅ ${description}`);
            this.passCount++;
        } else {
            console.log(`  ❌ ${description}`);
            if (actual) {
                console.log(`     실제: ${actual}`);
            }
            this.failCount++;
        }
    }
}

// JSON 페이로드 안전성 테스트
function testJSONPayloadSafety() {
    console.log('\n🧪 JSON 페이로드 안전성 테스트');
    
    const maliciousPayloads = [
        '{"text":"' + 'A'.repeat(10000) + '"}',  // 매우 긴 텍스트
        '{"text":"Hello","speed_cps":-1}',       // 음수 속도
        '{"text":"Hello","speed_cps":999999}',   // 매우 큰 속도
        '{"text":"Hello","interval_ms":-1}',     // 음수 간격
        '{{{"text":"malformed"}',                // 잘못된 JSON
        '{"text":null}',                         // null 텍스트
        '{"text":"","speed_cps":"invalid"}',     // 잘못된 타입
        JSON.stringify({text: '\x00\x01\x02'})  // 제어 문자
    ];
    
    console.log('다음 페이로드들이 ESP32에서 안전하게 처리되어야 합니다:');
    maliciousPayloads.forEach((payload, index) => {
        console.log(`  ${index + 1}. ${payload.substring(0, 50)}...`);
    });
}

// 테스트 실행
if (typeof require !== 'undefined' && require.main === module) {
    // Node.js 환경에서 실행
    try {
        const HangulPreprocessor = require('./hangulPreprocessor.js');
        const tester = new SafetyTestSuite();
        tester.runAllTests();
        testJSONPayloadSafety();
    } catch (error) {
        console.error('테스트 실행 오류:', error);
    }
} else if (typeof window !== 'undefined') {
    // 브라우저 환경에서 실행
    window.SafetyTestSuite = SafetyTestSuite;
    window.testJSONPayloadSafety = testJSONPayloadSafety;
}

// Export
if (typeof module !== 'undefined' && module.exports) {
    module.exports = { SafetyTestSuite, testJSONPayloadSafety };
}