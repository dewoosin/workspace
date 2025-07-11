# 컴파일 가이드 - Arduino IDE 방식

## 🔧 준비사항

### 1. Arduino IDE 설정
1. **Arduino IDE 2.x 설치**
2. **ESP32 보드 매니저 추가**:
   - 파일 → 환경설정 → 추가 보드 매니저 URLs
   - 추가: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`

3. **ESP32 보드 설치**:
   - 도구 → 보드 → 보드 매니저 → "ESP32" 검색 → 설치

### 2. 필수 라이브러리 설치
```
스케치 → 라이브러리 포함하기 → 라이브러리 관리...

설치할 라이브러리:
1. Adafruit TinyUSB Library (v2.2.6+)
2. NimBLE-Arduino (v1.4.0+)
3. ArduinoJson (v6.21.3+)
```

### 3. 보드 설정
```
도구 메뉴 설정:
- Board: "ESP32S3 Dev Module"
- USB Mode: "USB-OTG (TinyUSB)"
- USB CDC On Boot: "Enabled"
- USB Firmware MSC On Boot: "Disabled"
- USB DFU On Boot: "Disabled"
- Flash Size: "4MB (32Mb)"
- Partition Scheme: "Default 4MB with spiffs"
- PSRAM: "OPI PSRAM"
- Upload Speed: "921600"
```

## 📁 파일 구조

### Arduino IDE용 파일 구조
```
GHOSTYPE_Korean_HID/
├── GHOSTYPE_Korean_HID.ino         # 메인 스케치
├── korean_usb_hid.cpp              # 한국어 HID 클래스
├── korean_usb_hid.h                # 헤더 파일
├── usb_descriptors.cpp             # USB Descriptor
├── usb_descriptors.h               # USB Descriptor 헤더
└── config.h                        # 설정 파일
```

## ⚠️ 주의사항

### 컴파일 오류 대응
1. **라이브러리 누락**: 위 라이브러리 재설치
2. **헤더 파일 오류**: 경로 확인
3. **USB 모드 오류**: TinyUSB 모드 확인

### 업로드 오류 대응
1. **포트 인식 안됨**: BOOT 버튼 누르고 업로드
2. **권한 오류**: Arduino IDE 관리자 권한 실행
3. **시리얼 포트 충돌**: 다른 프로그램에서 포트 사용 중지

## 🚀 컴파일 절차

### 1. 파일 준비
현재 구조를 Arduino IDE 형식으로 변환 필요

### 2. 컴파일 테스트
Arduino IDE에서 "확인" 버튼 클릭

### 3. 업로드 테스트
ESP32 연결 후 "업로드" 버튼 클릭

### 4. 시리얼 모니터 확인
도구 → 시리얼 모니터에서 출력 확인

## 🎯 성공 지표

### 컴파일 성공 시
- "컴파일 완료" 메시지 출력
- 바이너리 크기 정보 표시
- 오류 메시지 없음

### 업로드 성공 시
- "업로드 완료" 메시지 출력
- ESP32 재부팅 후 시리얼 출력 시작

### 동작 확인
- 시리얼 모니터에서 초기화 메시지 확인
- Windows 장치 관리자에서 키보드 인식 확인