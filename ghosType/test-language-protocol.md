# Language-Switching Protocol Test Cases

## Implementation Overview

This document outlines test cases for the new structured language-switching protocol that enhances the existing Korean-to-QWERTY conversion system with explicit language switching commands.

## Protocol Format

### Commands
- `#CMD:HANGUL` - Switch to Korean input mode
- `#CMD:ENGLISH` - Switch to English input mode
- `#TEXT:{content}` - Type the specified content
- `#CMD:ENTER` - Send Enter key
- `#CMD:TAB` - Send Tab key
- `#CMD:SHIFT` - Send Shift key (future use)
- `#CMD:CTRL` - Send Ctrl key (future use)
- `#CMD:ALT` - Send Alt key (future use)

### Example Transformation
**Input**: "안녕Hellow 난 jason이야!"

**Expected Output**:
```
#CMD:HANGUL
#TEXT:dkssud
#CMD:ENGLISH
#TEXT:Hellow 
#CMD:HANGUL
#TEXT:sk 
#CMD:ENGLISH
#TEXT:jason
#CMD:HANGUL
#TEXT:dlwnd!
```

## Test Cases

### Test Case 1: Pure Korean Text
**Input**: "안녕하세요"
**Expected Protocol**:
```
#CMD:HANGUL
#TEXT:dkssudgktpdy
```
**ESP32 Behavior**:
1. Receives `#CMD:HANGUL` → Sends Alt+Shift if not in Korean mode
2. Receives `#TEXT:dkssudgktpdy` → Types each character with typing delay

### Test Case 2: Pure English Text
**Input**: "Hello World"
**Expected Protocol**:
```
#CMD:ENGLISH
#TEXT:Hello World
```
**ESP32 Behavior**:
1. Receives `#CMD:ENGLISH` → Sends Alt+Shift if in Korean mode
2. Receives `#TEXT:Hello World` → Types each character

### Test Case 3: Mixed Korean/English Text
**Input**: "안녕Hellow 난 jason이야!"
**Expected Protocol**:
```
#CMD:HANGUL
#TEXT:dkssud
#CMD:ENGLISH
#TEXT:Hellow 
#CMD:HANGUL
#TEXT:sk 
#CMD:ENGLISH
#TEXT:jason
#CMD:HANGUL
#TEXT:dlwnd!
```
**ESP32 Behavior**:
1. Switch to Korean → Type "dkssud"
2. Switch to English → Type "Hellow "
3. Switch to Korean → Type "sk "
4. Switch to English → Type "jason"
5. Switch to Korean → Type "dlwnd!"

### Test Case 4: Text with Control Characters
**Input**: "첫줄\n둘째줄\t탭테스트"
**Expected Protocol**:
```
#CMD:HANGUL
#TEXT:cjtwnf
#CMD:ENTER
#CMD:HANGUL
#TEXT:emdcornf
#CMD:TAB
#CMD:HANGUL
#TEXT:xkqxptrm
```

### Test Case 5: Numbers and Symbols
**Input**: "전화번호: 010-1234-5678"
**Expected Protocol**:
```
#CMD:HANGUL
#TEXT:wjsgkqjsgh: 
#CMD:ENGLISH
#TEXT:010-1234-5678
```

### Test Case 6: Alternating Single Characters
**Input**: "a한b글c"
**Expected Protocol**:
```
#CMD:ENGLISH
#TEXT:a
#CMD:HANGUL
#TEXT:gks
#CMD:ENGLISH
#TEXT:b
#CMD:HANGUL
#TEXT:rmf
#CMD:ENGLISH
#TEXT:c
```

### Test Case 7: Empty and Whitespace
**Input**: "한글 English"
**Expected Protocol**:
```
#CMD:HANGUL
#TEXT:gksrmf 
#CMD:ENGLISH
#TEXT:English
```

### Test Case 8: Special Korean Characters
**Input**: "돼지와 뭐하니?"
**Expected Protocol**:
```
#CMD:HANGUL
#TEXT:ehldldk anxgksl?
```

## Frontend JavaScript Testing

### Test Language Detection
```javascript
import { detectCharacterLanguage, segmentTextByLanguage } from './language-protocol.js';

// Test character detection
console.log(detectCharacterLanguage('안')); // Should return 'korean'
console.log(detectCharacterLanguage('a')); // Should return 'english'
console.log(detectCharacterLanguage('1')); // Should return 'english'
console.log(detectCharacterLanguage('\n')); // Should return 'control'

// Test text segmentation
const text = "안녕Hello 세계";
const segments = segmentTextByLanguage(text);
console.log(segments);
// Expected: [
//   {language: 'korean', text: '안녕', startIndex: 0, endIndex: 1},
//   {language: 'english', text: 'Hello ', startIndex: 2, endIndex: 7},
//   {language: 'korean', text: '세계', startIndex: 8, endIndex: 9}
// ]
```

### Test Protocol Generation
```javascript
import { convertToProtocol, validateProtocol } from './language-protocol.js';

const testCases = [
    '안녕',
    'Hello',
    '안녕Hellow 난 jason이야!',
    'Test\nNewline\tTab',
    '한글English한글'
];

testCases.forEach(testCase => {
    const result = convertToProtocol(testCase);
    console.log(`Input: "${testCase}"`);
    console.log('Protocol:', result.protocol);
    console.log('Analysis:', result.analysis.stats);
    
    const validation = validateProtocol(result.protocol);
    console.log('Valid:', validation.valid);
    if (!validation.valid) {
        console.log('Errors:', validation.errors);
    }
    console.log('---');
});
```

## ESP32 Testing

### Serial Monitor Output
When processing the protocol for "안녕Hellow", expected debug output:

```
구조화된 프로토콜 감지됨
구조화된 프로토콜 모드로 처리 시작
라인 1: #CMD:HANGUL
프로토콜 명령 처리: #CMD:HANGUL
한영 전환: 영문 → 한글
라인 2: #TEXT:dkssud
프로토콜 명령 처리: #TEXT:dkssud
텍스트 입력: "dkssud"
라인 3: #CMD:ENGLISH
프로토콜 명령 처리: #CMD:ENGLISH
한영 전환: 한글 → 영문
라인 4: #TEXT:Hellow
프로토콜 명령 처리: #TEXT:Hellow
텍스트 입력: "Hellow"
구조화된 프로토콜 처리 완료
구조화된 프로토콜 타이핑 완료!
```

### Manual Testing Procedure
1. **Upload ESP32 firmware** with structured protocol support
2. **Open BLE connection** from web frontend
3. **Enable structured protocol** mode in frontend (should be default)
4. **Test each test case** systematically:
   - Enter test input in frontend
   - Observe frontend protocol preview
   - Confirm ESP32 serial debug output
   - Verify actual typing behavior
   - Check language switching occurs correctly

### Expected Behavior Verification
- [ ] Korean characters are converted to QWERTY jamo sequences
- [ ] Language switches happen automatically based on character type
- [ ] No redundant language switches (English→English, Korean→Korean)
- [ ] Control characters (Enter, Tab) are handled correctly
- [ ] Mixed text produces correct segmentation
- [ ] ESP32 maintains language state correctly
- [ ] Unknown commands are ignored safely
- [ ] Malformed protocols don't crash the system

## Error Handling Tests

### Malformed Protocol
**Input Protocol**:
```
#CMD:UNKNOWN
#TEXT:test
#INVALID:line
```
**Expected Behavior**: ESP32 logs unknown commands but continues processing

### Missing Language Commands
**Input Protocol**:
```
#TEXT:test
```
**Expected Behavior**: Text is typed without language switch (current mode maintained)

### Empty Text Commands
**Input Protocol**:
```
#CMD:HANGUL
#TEXT:
```
**Expected Behavior**: Language switch occurs, no text typed

## Performance Testing

### Large Text Input
**Input**: Long mixed text (1000+ characters)
**Test Goals**:
- Verify protocol generation doesn't timeout
- Confirm ESP32 can process long protocols
- Check memory usage remains stable

### Rapid Switching
**Input**: "a한b글c영d어"
**Test Goals**:
- Verify rapid language switches work correctly
- Check for timing issues or race conditions
- Confirm no keystrokes are lost

## Backward Compatibility Tests

### Legacy JSON Protocol
**Input**: JSON format like `{"text": "dkssud", "speed_cps": 10, "type": "korean"}`
**Expected Behavior**: ESP32 processes using legacy mode (no language switching)

### Mixed Protocol Modes
**Test Scenario**: Switch between structured and legacy mode during session
**Expected Behavior**: Each message processed according to its format

## Success Criteria

✅ **Protocol Generation**: Frontend correctly segments mixed text and generates valid protocol commands

✅ **ESP32 Parsing**: ESP32 correctly parses all protocol commands and ignores unknown ones safely

✅ **Language Switching**: Automatic language toggling works reliably without redundant switches

✅ **Character Conversion**: Korean characters are correctly converted to QWERTY jamo sequences

✅ **Control Characters**: Enter, Tab, and other control characters work correctly

✅ **Error Handling**: Malformed or unknown commands don't break the system

✅ **Performance**: Large texts and rapid switching work without issues

✅ **Backward Compatibility**: Legacy protocols continue to work alongside new structured protocols

✅ **User Experience**: Mixed Korean/English text types naturally without manual language switching

## Future Enhancements

### Additional Commands
- `#CMD:CAPS` - Toggle Caps Lock
- `#CMD:NUM` - Toggle Num Lock
- `#CMD:F1` through `#CMD:F12` - Function keys
- `#CMD:WIN` - Windows key
- `#CMD:MENU` - Context menu key

### Advanced Features
- `#CMD:COMBO:CTRL+C` - Key combinations
- `#CMD:DELAY:100` - Introduce delays
- `#CMD:REPEAT:3` - Repeat next command/text
- `#CMD:MODE:GAME` - Switch to gaming mode (different key mappings)

### Protocol Compression
- Optimize protocol size for long texts
- Batch consecutive same-language text segments
- Compress common command sequences

### Analytics
- Track language switch frequency
- Monitor typing accuracy and speed
- Log protocol processing performance