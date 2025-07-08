# GHOSTYPE 시스템 아키텍처

## 올바른 데이터 흐름

```
사용자 입력: "Hello 안녕하세요"
     ↓
JavaScript 전처리기 (hangulPreprocessor.js)
     ↓ 
변환된 키 시퀀스: "Hello ⌨HANGUL_TOGGLE⌨dkssudgktpdy"
     ↓
BLE 전송 (webBLEInterface.js)
     ↓
ESP32 수신 (BLESimple)
     ↓
HID 키보드 출력 (USB HID)
```

## 역할 분담

### 🌐 JavaScript (클라이언트)
- **hangulPreprocessor.js**: 한글→QWERTY 변환
- **webBLEInterface.js**: BLE 통신 관리
- **index.html**: 사용자 인터페이스

### 🔌 ESP32 (서버)  
- **BLESimple**: BLE 서버 (데이터 수신만)
- **USB HID**: 키보드 출력 (변환된 키 그대로 타이핑)
- **NO 한글 처리**: JavaScript에서 이미 변환됨

## 데이터 형식

### JavaScript → ESP32
```json
{
  "text": "Hello ⌨HANGUL_TOGGLE⌨dkssudgktpdy",
  "speed_cps": 6,
  "interval_ms": 100
}
```

### ESP32 → JavaScript (응답)
```
"OK:45"  // 처리된 문자 수
```

## 제거된 불필요한 코드

- ❌ `HangulQWERTY.cpp/h` (ESP32에서 삭제)
- ❌ ESP32의 한글 처리 로직
- ❌ 중복 변환 코드

## 핵심 원칙

1. **단일 책임**: JavaScript만 한글 변환
2. **단순성**: ESP32는 키 출력만
3. **효율성**: 중복 변환 제거
4. **유지보수성**: 명확한 역할 분담