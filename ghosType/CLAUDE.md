# CLAUDE.md - GHOSTYPE 프로젝트 개발 가이드

## 프로젝트 개요
GHOSTYPE는 웹 브라우저에서 BLE를 통해 ESP32로 타이핑 명령을 전송하고, ESP32가 USB HID 키보드로 텍스트를 입력하는 시스템입니다.

## 아키텍처
```
웹 브라우저 (JavaScript) → BLE → ESP32-S3 → USB HID → 호스트 컴퓨터
```

### 데이터 흐름
1. 사용자가 웹 페이지에서 텍스트 입력
2. JavaScript가 한글을 QWERTY 키로 변환
3. BLE를 통해 ESP32로 전송
4. ESP32가 USB HID 키보드로 타이핑 실행

## 핵심 원칙

### 🚨 중요: BLE 수정 시 필수 확인사항
**모든 BLE 로직 변경은 `esp32/BLE_Debug_History.md` 파일을 먼저 확인해야 합니다.**

- BLE 연결 문제는 반복적으로 발생하는 복잡한 이슈입니다
- 과거의 실패한 시도를 반복하지 않기 위해 모든 변경사항이 기록됩니다
- 새로운 BLE 수정을 시도하기 전에 반드시 히스토리를 검토하세요

### 역할 분담
- **JavaScript**: 한글→QWERTY 변환, BLE 통신
- **ESP32**: BLE 수신, USB HID 키보드 출력만
- **중복 없음**: 한글 처리는 JavaScript에서만

## 파일 구조

### JavaScript (클라이언트)
```
js/
├── hangulPreprocessor.js     # 한글→QWERTY 변환
├── webBLEInterface.js        # BLE 통신 관리
├── bleConnectionDiagnostics.js # BLE 진단 도구
└── index.html               # 사용자 인터페이스
```

### ESP32 (서버)
```
esp32/src/
├── main.cpp                 # 메인 엔트리 포인트
├── config.h                 # 시스템 설정 및 상수
├── ble_manager.*           # BLE 통신 관리
├── parser.*                # 데이터 파싱 및 명령 해석
├── typing_handler.*        # 타이핑 실행 및 제어
├── hid_utils.*             # USB HID 키보드 제어
└── BLE_Debug_History.md    # BLE 문제 해결 이력
```

## 개발 가이드라인

### 코드 수정 시 주의사항

#### 1. BLE 관련 수정
```bash
# BLE 코드 수정 전 필수 단계
1. esp32/BLE_Debug_History.md 검토
2. 과거 시도된 방법인지 확인
3. 새로운 접근 방식인 경우에만 진행
4. 수정 후 결과를 BLE_Debug_History.md에 기록
```

#### 2. JavaScript 수정
- 한글 변환 로직은 `hangulPreprocessor.js`에만 위치
- BLE 통신 로직은 `webBLEInterface.js`에만 위치
- ESP32 연결 문제 시 진단 도구 먼저 실행

#### 3. ESP32 수정
- 프로덕션 코드는 Serial 출력 없음
- 모든 설정은 `config.h`에 중앙 관리
- 모듈별로 명확히 분리된 구조 유지

### 테스트 절차

#### BLE 연결 테스트
1. ESP32 펌웨어 업로드
2. 시리얼 모니터 완전 종료
3. ESP32 하드 리셋 (BOOT+RESET)
4. 웹에서 "🔧 Run Diagnostics" 실행
5. 결과를 BLE_Debug_History.md에 기록

#### 타이핑 테스트
```javascript
// 테스트 순서
1. 영어 텍스트: "Hello World"
2. 한글 텍스트: "안녕하세요"  
3. 혼합 텍스트: "Hello 안녕하세요"
4. 토글 테스트: "Test 되 vs 돼 example"
```

## 문제 해결

### BLE 연결 실패
1. **첫 번째**: `BLE_Debug_History.md` 확인
2. **두 번째**: 하드웨어 리셋 및 환경 점검
3. **세 번째**: 진단 도구로 상세 분석
4. **마지막**: 새로운 해결책 시도 후 이력 기록

### 타이핑 문제
- JavaScript 콘솔에서 변환 결과 확인
- ESP32 LED 상태로 시스템 상태 파악
- 토글 마커 처리 여부 확인

### 성능 문제
- 메모리 사용량 모니터링
- 타이핑 속도 조절
- 청크 크기 최적화

## 버전 관리

### 브랜치 전략
- `main`: 안정 버전
- `feature/ble-fix`: BLE 문제 해결용
- `feature/typing-improve`: 타이핑 성능 개선용

### 커밋 메시지
```
feat: Add BLE connection diagnostics
fix: Resolve MTU size compatibility issue
docs: Update BLE debug history with test results
```

## 배포 가이드

### ESP32 펌웨어
```bash
cd esp32
pio run -t upload
```

### 웹 애플리케이션
```bash
# 로컬 서버 실행
cd js
python -m http.server 8000
# 브라우저에서 http://localhost:8000 접속
```

## 연락처 및 지원

### 문제 보고
- BLE 관련 문제: `BLE_Debug_History.md`에 먼저 기록
- 새로운 기능 요청: GitHub Issues
- 버그 리포트: 재현 단계와 함께 상세히 기록

### 개발 히스토리
- 모든 BLE 수정사항은 날짜/시간과 함께 기록
- 실패한 시도도 중요한 정보이므로 반드시 문서화
- 성공한 해결책은 재사용 가능하도록 상세히 기록

---

**⚠️ 중요 알림: BLE 코드 수정 시 반드시 BLE_Debug_History.md를 먼저 확인하세요!**