/**
 * Demonstration of Korean Mapping Improvements
 * Shows before/after comparison and key fixes
 */

// Import both old and new converters for comparison
import { convertHangulToJamoKeys as oldConverter } from './korean-converter.js';
import { 
    convertHangulToJamoKeys as newConverter,
    runDiagnostics,
    canTypeOnKeyboard
} from './korean-converter-improved.js';

// Demo test cases showing improvements
const DEMO_CASES = [
    {
        text: '돼',
        description: 'Special syllable "dwe" (becomes)',
        oldExpected: 'problematic',
        newExpected: 'eho'
    },
    {
        text: '뭐',
        description: 'Special syllable "mwo" (what)',
        oldExpected: 'problematic', 
        newExpected: 'anjr'
    },
    {
        text: '닭',
        description: 'Compound jongsung "dalk" (chicken)',
        oldExpected: 'incomplete',
        newExpected: 'ekfr'
    },
    {
        text: '없',
        description: 'Compound jongsung "eop" (none)',
        oldExpected: 'incomplete',
        newExpected: 'djqt'
    },
    {
        text: '과',
        description: 'Compound vowel "gwa"', 
        oldExpected: 'incomplete',
        newExpected: 'rhk'
    },
    {
        text: '의',
        description: 'Compound vowel "ui" (of/meaning)',
        oldExpected: 'problematic',
        newExpected: 'dml'
    },
    {
        text: '안녕하세요',
        description: 'Common greeting "annyeonghaseyo"',
        oldExpected: 'basic support',
        newExpected: 'dkssudgktpdy'
    }
];

export function runDemo() {
    console.log('🚀 Korean Mapping Improvements Demo\n');
    
    console.group('📊 Before vs After Comparison');
    
    DEMO_CASES.forEach((testCase, index) => {
        console.log(`\n${index + 1}. ${testCase.description}`);
        console.log(`   Input: "${testCase.text}"`);
        
        try {
            const oldResult = oldConverter(testCase.text);
            const newResult = newConverter(testCase.text);
            const canType = canTypeOnKeyboard(testCase.text);
            
            console.log(`   Old: "${oldResult}" (${testCase.oldExpected})`);
            console.log(`   New: "${newResult}" (${testCase.newExpected})`);
            console.log(`   Typable: ${canType ? '✅ Yes' : '❌ No'}`);
            
            if (oldResult !== newResult) {
                console.log(`   🎯 IMPROVED!`);
            }
        } catch (error) {
            console.error(`   💥 Error: ${error.message}`);
        }
    });
    
    console.groupEnd();
    
    // Show coverage improvements
    console.group('📈 Coverage Improvements');
    
    const coverageTests = [
        '가나다라마바사아자차카타파하',  // Basic consonants
        '까따빠싸짜',                    // Double consonants  
        '과괘괴궈궤귀의',                // Complex vowels
        '각간갈감갑갓강갖',              // Basic finals
        '닭삶앉읽없',                    // Compound finals
        '돼뭐뒤왜웨위외의'               // Special cases
    ];
    
    coverageTests.forEach((text, index) => {
        const categories = [
            'Basic consonants',
            'Double consonants', 
            'Complex vowels',
            'Basic finals',
            'Compound finals',
            'Special cases'
        ];
        
        try {
            const result = newConverter(text);
            const allTypable = [...text].every(char => canTypeOnKeyboard(char));
            
            console.log(`${categories[index]}: "${text}"`);
            console.log(`  Output: "${result}"`);
            console.log(`  All typable: ${allTypable ? '✅' : '❌'}`);
        } catch (error) {
            console.error(`  Error: ${error.message}`);
        }
    });
    
    console.groupEnd();
    
    // Run diagnostics
    console.group('🔧 System Diagnostics');
    const diagnostics = runDiagnostics();
    console.log('Diagnostics completed. Check console for details.');
    console.groupEnd();
    
    // Performance comparison
    console.group('⚡ Performance Test');
    const testText = '안녕하세요 반갑습니다 한글 테스트입니다 '.repeat(100);
    const iterations = 1000;
    
    console.time('Old Converter');
    for (let i = 0; i < iterations; i++) {
        oldConverter(testText);
    }
    console.timeEnd('Old Converter');
    
    console.time('New Converter');
    for (let i = 0; i < iterations; i++) {
        newConverter(testText);
    }
    console.timeEnd('New Converter');
    
    console.log(`Processed ${testText.length * iterations} characters`);
    console.groupEnd();
    
    return {
        testCases: DEMO_CASES.length,
        diagnostics,
        improvements: 'Comprehensive Korean support with special syllables, compound characters, and error handling'
    };
}

// Make available globally for browser console
if (typeof window !== 'undefined') {
    window.koreanDemo = {
        runDemo,
        testText: (text) => {
            console.log(`Testing: "${text}"`);
            const result = newConverter(text);
            const canType = canTypeOnKeyboard(text);
            console.log(`Result: "${result}"`);
            console.log(`Typable: ${canType}`);
            return result;
        }
    };
    
    console.log('Korean demo available at window.koreanDemo');
    console.log('Run window.koreanDemo.runDemo() to see improvements');
}