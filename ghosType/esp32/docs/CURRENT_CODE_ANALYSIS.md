# 현재 코드 분석 보고서

## 📊 기본 정보
- **프로젝트 크기**: 160KB
- **주요 소스 파일**: `src/main.cpp` (903줄)
- **백업 위치**: `esp32_backup_20250711_145302/`
- **플랫폼**: ESP32-S3, PlatformIO

## 🔍 현재 아키텍처 분석

### 1. 사용 중인 라이브러리
```cpp
#include <USB.h>
#include <USBHIDKeyboard.h>        // ⚠️ 교체 대상
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>
```

### 2. 핵심 컴포넌트
- **USB HID**: `USBHIDKeyboard keyboard;` - 단일 인스턴스
- **BLE 서버**: Nordic UART 서비스 사용
- **한영 전환**: 12가지 방식 지원 (`executeHangulToggle()`)
- **프로토콜**: 구조화된 명령어 시스템

### 3. 현재 한영 전환 메커니즘
```cpp
void executeHangulToggle() {
    switch (hangulToggleMethod) {
        case HANGUL_TOGGLE_RIGHT_ALT:
            keyboard.press(KEY_RIGHT_ALT);
            delay(50);
            keyboard.release(KEY_RIGHT_ALT);
            break;
        // ... 12가지 방식
    }
}
```

### 4. BLE 통신 구조
- **서비스 UUID**: `6e400001-b5a3-f393-e0a9-e50e24dcca9e`
- **RX/TX 특성**: 양방향 통신
- **MTU**: 185바이트 제한
- **프로토콜**: `#CMD:HANGUL`, `#TEXT:content` 등

## 🎯 문제점 식별

### 1. **USB HID 한계**
- `USBHIDKeyboard` 라이브러리는 기본 USB Descriptor 사용
- Country Code, Language ID 설정 불가
- VID/PID 커스터마이징 제한적

### 2. **한영 전환 실패 원인**
- Windows가 ESP32를 "일반 키보드"로 인식
- 한국어 키보드 속성 없음
- IME와의 연동 부족

### 3. **아키텍처 한계**
- 모놀리식 구조 (903줄 단일 파일)
- 하드코딩된 설정값
- 제한적인 확장성

## 🔄 교체 전략

### 1. **단계적 교체**
```
Phase 1: USB Descriptor 교체
├── USBHIDKeyboard → TinyUSB 직접 제어
├── Custom HID Report Descriptor
└── 한국어 키보드 Identity 설정

Phase 2: 기능 통합
├── 기존 BLE 기능 유지
├── 프로토콜 호환성 보장
└── 12가지 한영 전환 방식 이식
```

### 2. **보존해야 할 기능**
- ✅ BLE 서버 및 통신 프로토콜
- ✅ 구조화된 명령어 시스템
- ✅ 12가지 한영 전환 방식
- ✅ 자동 진단 및 테스트 기능
- ✅ 타이핑 속도 제어

### 3. **교체할 컴포넌트**
- ❌ `USBHIDKeyboard` → `TinyUSB` 직접 제어
- ❌ 기본 USB Descriptor → 한국어 키보드 Descriptor
- ❌ 단일 파일 구조 → 모듈화된 구조

## 📈 메모리 사용량 분석

### 현재 추정 사용량
- **Flash**: ~100KB (라이브러리 포함)
- **RAM**: ~50KB (BLE + HID 스택)
- **여유 공간**: 충분 (4MB Flash, 320KB RAM)

### 새로운 구조 예상 사용량
- **Flash**: ~120KB (+20KB, TinyUSB 직접 제어)
- **RAM**: ~60KB (+10KB, 추가 구조체)
- **안전 마진**: 여전히 충분

## 🚨 위험 요소

### 1. **호환성 위험**
- 기존 BLE 클라이언트 (웹, iOS)와의 호환성
- 프로토콜 변경으로 인한 통신 오류

### 2. **기능 위험**
- 12가지 한영 전환 방식의 정확한 이식 필요
- 타이밍 및 딜레이 값 재조정 필요

### 3. **개발 위험**
- TinyUSB 직접 제어의 복잡성
- 디버깅 난이도 증가

## 💡 권장 사항

### 1. **점진적 접근**
```
Step 1: 최소 기능 TinyUSB 구현
Step 2: 한국어 키보드 Descriptor 적용
Step 3: 기존 기능 하나씩 이식
Step 4: 전체 통합 테스트
```

### 2. **안전 장치**
- 각 단계별 백업 생성
- 롤백 계획 준비
- 기능별 단위 테스트

### 3. **성공 지표**
- Windows 장치 관리자에서 "Samsung Korean USB Keyboard" 인식
- 한영 전환 정상 동작
- 기존 BLE 기능 유지

## 📋 다음 단계 (STEP 2)

### 즉시 실행 가능한 작업
1. **PlatformIO 설정 업데이트**
   - TinyUSB 라이브러리 추가
   - 한국어 키보드 플래그 설정

2. **기본 헤더 파일 생성**
   - `hid_descriptor_korean.h`
   - `usb_device_config.h`

3. **컴파일 테스트**
   - 새로운 설정으로 빌드 성공 확인

### 준비 완료 상태
- ✅ 백업 완료
- ✅ 분석 완료
- ✅ 위험 요소 식별
- ✅ 교체 전략 수립

**STEP 1 완료 준비됨 - STEP 2 진행 가능**