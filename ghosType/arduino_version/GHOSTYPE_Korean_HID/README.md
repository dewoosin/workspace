# GHOSTYPE Korean HID - Arduino IDE 버전

## 📋 개요

ESP32-S3 기반 한국어 키보드 USB HID 구현입니다. Windows에서 Samsung Korean USB Keyboard로 인식되어 완벽한 한영 전환을 지원합니다.

## 🔧 설치 및 설정

### 1. Arduino IDE 설정

1. **Arduino IDE 2.x 설치**
2. **ESP32 보드 추가**:
   - 파일 → 환경설정 → 추가 보드 매니저 URLs
   - `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json` 추가
3. **ESP32 보드 설치**:
   - 도구 → 보드 → 보드 매니저 → "ESP32" 검색 → 설치

### 2. 필수 라이브러리 설치

스케치 → 라이브러리 포함하기 → 라이브러리 관리에서 다음 라이브러리 설치:

```
✅ Adafruit TinyUSB Library (v2.2.6 이상)
✅ NimBLE-Arduino (v1.4.0 이상)  
✅ ArduinoJson (v6.21.3 이상)
```

### 3. 보드 설정

도구 메뉴에서 다음과 같이 설정:

```
Board: "ESP32S3 Dev Module"
USB Mode: "USB-OTG (TinyUSB)"
USB CDC On Boot: "Enabled"
USB Firmware MSC On Boot: "Disabled"
USB DFU On Boot: "Disabled"
Flash Size: "4MB (32Mb)"
Partition Scheme: "Default 4MB with spiffs"
PSRAM: "OPI PSRAM"
Upload Speed: "921600"
```

## 🚀 사용법

### 1. 스케치 열기
- `GHOSTYPE_Korean_HID.ino` 파일을 Arduino IDE로 열기

### 2. 컴파일 및 업로드
- **확인** 버튼으로 컴파일 테스트
- **업로드** 버튼으로 ESP32에 업로드

### 3. 동작 확인
- **시리얼 모니터** 열기 (115200 baud)
- **Windows 장치 관리자**에서 키보드 인식 확인
- **메모장**에서 실제 타이핑 테스트

## 📊 기술 사양

### USB 장치 정보
- **VID**: 0x04E8 (Samsung Electronics)
- **PID**: 0x7021 (Korean USB Keyboard)
- **Country Code**: 16 (Korean)
- **Language**: Korean (0x0412) + English (0x0409)

### 지원 기능
- ✅ 표준 키보드 입력 (Report ID 1)
- ✅ Consumer Control 키 (Report ID 2)
- ✅ 한영 전환 (12가지 방식)
- ✅ 실시간 상태 모니터링
- ✅ 자동 테스트 기능

### 한영 전환 방식
1. Right Alt (기본)
2. Alt + Shift
3. Ctrl + Space
4. Shift + Space
5. Hangul Key (0xF2)
6. Left Alt
7. Win + Space
8. Language 1 (0x90)
9. Language 2 (0x91)
10. F9 Key
11. Menu Key
12. Application Key

## 🧪 테스트 기능

### 자동 테스트
- 시스템 부팅 후 자동으로 테스트 실행
- 10초마다 순환 테스트
- 시리얼 모니터에서 결과 확인

### 테스트 항목
1. **기본 키 입력**: 영문 텍스트 타이핑
2. **한영 전환**: 언어 모드 변경
3. **Consumer 키**: 특수 키 전송
4. **시스템 상태**: 현재 상태 출력

## 🔍 문제 해결

### 컴파일 오류
- **라이브러리 누락**: 필수 라이브러리 재설치
- **보드 설정**: ESP32S3 + TinyUSB 모드 확인
- **헤더 파일**: 모든 파일이 같은 폴더에 있는지 확인

### 업로드 오류
- **포트 인식 안됨**: BOOT 버튼 누르고 업로드
- **권한 오류**: Arduino IDE 관리자 권한 실행
- **시리얼 충돌**: 시리얼 모니터 종료 후 업로드

### 동작 오류
- **키보드 인식 안됨**: USB 케이블 및 포트 확인
- **한영 전환 안됨**: Windows 한국어 입력기 설치 확인
- **타이핑 안됨**: 메모장 등 텍스트 입력 가능한 앱에서 테스트

## 📈 성공 지표

### ✅ 정상 동작 확인
- 시리얼 모니터: "Korean USB HID initialized successfully"
- Windows 장치 관리자: "Korean USB Keyboard" 표시
- 메모장: 실제 텍스트 입력 가능
- 한영 전환: 언어 모드 변경 동작

### ❌ 오류 상황 대응
- 초기화 실패 → 라이브러리 및 보드 설정 재확인
- 인식 실패 → USB 케이블 및 드라이버 확인
- 타이핑 실패 → 시리얼 모니터에서 오류 메시지 확인

## 💡 참고사항

### 중요 파일
- `GHOSTYPE_Korean_HID.ino`: 메인 스케치
- `config.h`: 설정 파일
- `usb_descriptors.h`: USB 정의
- `usb_descriptors.cpp`: 구현 파일

### 확장 가능성
- BLE 기능 추가 가능
- 더 많은 한영 전환 방식 지원
- 타이핑 속도 제어 기능
- 진단 및 분석 도구

---

**버전**: 1.0.0  
**작성일**: 2025년 7월 11일  
**대상**: ESP32-S3 개발 보드  
**라이선스**: MIT License