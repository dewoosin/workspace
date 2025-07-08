# BLE Debug History - GHOSTYPE ESP32

## 목적
이 문서는 GHOSTYPE ESP32 프로젝트의 BLE 연결 문제 해결 과정을 추적합니다.
모든 BLE 관련 수정사항, 실험, 설정 변경을 기록하여 반복적인 실수를 방지하고
장기적인 문제 해결 지식 베이스를 구축합니다.

---

## 2024-12-28

### 15:30 - 초기 BLE 문제 확인
**문제**: ESP32 장치가 검색되지 않거나, 검색되어도 페어링/연결이 실패함
**증상**: 
- Chrome Web Bluetooth에서 장치 검색 안됨
- 일부 경우 검색되지만 GATT 연결 실패
- 시리얼 포트 충돌로 인한 연결 끊김

**현재 설정**:
```cpp
// BLE 기본 설정
Device Name: "GHOSTYPE"
Service UUID: "12345678-1234-5678-9012-123456789abc"
MTU: 기본값 (512 추정)
보안: 기본 NimBLE 설정
```

**상태**: 문제 지속 중

---

### 15:45 - MTU 크기 조정 시도
**시도한 변경사항**:
```cpp
// BLENimbleManager.cpp
NimBLEDevice::setMTU(247);  // 512 → 247로 변경
```

**이유**: BLE MTU 크기가 클수록 호환성 문제 발생 가능
**결과**: 검색 문제 지속됨
**상태**: 해결되지 않음

---

### 16:00 - 보안 설정 제거 실험
**시도한 변경사항**:
```cpp
// 기존
NimBLEDevice::setSecurityAuth(false, false, true);
NimBLEDevice::setSecurityPasskey(123456);
NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);

// 변경 후
// 모든 보안 설정 제거 (기본값 사용)
```

**이유**: 복잡한 보안 설정이 연결을 방해할 수 있음
**결과**: 여전히 검색되지 않음
**상태**: 해결되지 않음

---

### 16:15 - 광고 간격 조정
**시도한 변경사항**:
```cpp
// 기존
pAdvertising->setMinInterval(100);  // 62.5ms
pAdvertising->setMaxInterval(200);  // 125ms

// 변경 후
// 광고 간격 설정 제거 (기본값 사용)
```

**이유**: 너무 빠른 광고 간격이 문제를 일으킬 수 있음
**결과**: 검색 문제 지속
**상태**: 해결되지 않음

---

### 16:30 - 시리얼 충돌 문제 확인
**발견사항**: 
- ESP32에서 Serial 출력과 BLE 동시 사용 시 충돌 발생
- `ClearCommError failed (PermissionError(13))` 에러
- 시리얼 모니터 종료 후에도 연결 안됨

**시도한 해결책**:
1. 모든 Serial.print() 제거
2. 시리얼 모니터 완전 종료
3. ESP32 하드 리셋 (BOOT+RESET)

**결과**: 시리얼 충돌은 해결되었지만 BLE 검색 문제는 지속
**상태**: 부분적 해결

---

### 17:00 - 기본 BLE 광고 테스트
**시도한 변경사항**:
```cpp
// 최소한의 BLE 구현으로 단순화
Device Name: "ESP32" (GHOSTYPE → ESP32)
서비스: 없음 (기본 광고만)
특성: 없음
```

**이유**: 복잡한 서비스/특성 설정이 문제일 수 있음
**코드**:
```cpp
NimBLEDevice::init("ESP32");
NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
pAdvertising->setName("ESP32");
pAdvertising->start();
```

**결과**: 검색 문제 지속
**상태**: 해결되지 않음

---

### 17:30 - 프로덕션 코드 리팩토링
**변경사항**:
- 전체 BLE 코드를 `ble_manager.cpp/h`로 분리
- 모든 Serial 출력 제거
- 예외 처리 강화
- 메모리 관리 개선

**새로운 BLE 설정**:
```cpp
// ble_manager.cpp
Device Name: BLE_DEVICE_NAME (config.h에서 "GHOSTYPE")
Service UUID: BLE_SERVICE_UUID
RX Characteristic: BLE_CHAR_RX_UUID 
TX Characteristic: BLE_CHAR_TX_UUID
연결 매개변수: BLE_MIN_CONN_INTERVAL, BLE_MAX_CONN_INTERVAL
```

**결과**: 시리얼 포트 에러는 해결되었으나 BLE 검색 문제 지속
**상태**: 기본 문제 미해결

---

### 18:00 - 현재 상황 점검
**발견사항**:
- 프로덕션 코드 리팩토링 완료 (모듈화, Serial 제거)
- 시리얼 포트 충돌 문제는 해결됨
- 하지만 BLE 장치 검색 기본 문제는 여전히 존재

**현재 설정** (ble_manager.cpp):
```cpp
// 가장 기본적인 BLE 설정 사용
NimBLEDevice::init(BLE_DEVICE_NAME);  // "GHOSTYPE"
// MTU, 보안, 광고 간격 모두 기본값
// 복잡한 매개변수 설정 제거
```

**상태**: **근본 원인 미해결 - 하드웨어/환경 의심 단계**

---

### 18:15 - 최소 BLE 테스트 코드 생성
**접근법**: 문제를 격리하기 위해 최소한의 BLE 광고만 수행하는 테스트 생성

**테스트 코드** (`ble_test.cpp`):
```cpp
// 서비스/특성 없이 광고만 수행
NimBLEDevice::init("GHOSTYPE-TEST");
NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
pAdvertising->setName("GHOSTYPE-TEST");
pAdvertising->start();
```

**LED 신호**:
- 초기화: 2회 깜빡임
- 광고 성공: 계속 켜짐
- 광고 실패: 빠른 깜빡임

**목적**: 복잡한 GHOSTYPE 펌웨어와 분리하여 BLE 하드웨어 자체 테스트
**상태**: 테스트 준비 완료

---

### 18:30 - 치명적 부트 루프 발생
**문제**: 프로덕션 코드 업로드 후 ESP32가 무한 리셋 루프에 빠짐
**증상**:
```
rst:0x3 (RTC_SW_SYS_RST),boot:0x8 (SPI_FAST_FLASH_BOOT)
Saved PC:0x403cdb0a
```
- 위 메시지가 초당 수십 번 반복
- BLE 검색 불가 (부팅조차 못함)

**원인 분석**:
1. USB HID와 BLE 동시 사용 충돌 의심
2. USB 모드 설정 오류 가능성
3. 메모리 부족 또는 스택 오버플로우

**긴급 수정사항**:
1. platformio.ini USB 설정 변경:
   - ARDUINO_USB_MODE=1 → 0
   - ARDUINO_USB_CDC_ON_BOOT=0 → 1
2. HID 초기화 비활성화:
   - USB.begin() 주석 처리
   - keyboard.begin() 주석 처리
   - hid_ready = false로 설정

**결과**: 테스트 필요
**상태**: 부트 루프 해결 시도 중

---

## 현재 상태 요약

### 확인된 문제들
1. **장치 검색 실패** - 가장 심각한 문제
2. **시리얼 포트 충돌** - 해결됨
3. **GATT 연결 실패** - 검색된 경우에도 연결 안됨

### 시도한 해결책들
1. ✅ MTU 크기 감소 (512→247)
2. ✅ 보안 설정 제거
3. ✅ 광고 간격 기본값 사용
4. ✅ Serial 출력 제거
5. ✅ 기본 BLE 광고 테스트
6. ✅ 프로덕션 코드 리팩토링

### 다음 테스트 계획
1. **하드웨어 테스트**: 다른 ESP32-S3 보드로 테스트
2. **펌웨어 테스트**: 알려진 작동하는 BLE 예제 코드 테스트
3. **환경 테스트**: 다른 컴퓨터/브라우저에서 테스트
4. **스마트폰 테스트**: nRF Connect 앱으로 BLE 스캔
5. **안테나 테스트**: WiFi 비활성화 후 BLE만 사용

### 의심되는 원인들
1. **하드웨어 문제**: ESP32-S3 BLE 안테나 불량
2. **펌웨어 문제**: NimBLE 라이브러리 설정 오류
3. **환경 문제**: 2.4GHz 간섭, 호스트 블루투스 문제
4. **코드 문제**: 초기화 순서나 설정 오류

---

## 테스트 체크리스트

### 기본 확인사항
- [ ] ESP32 전원 상태 확인
- [ ] 펌웨어 업로드 성공 확인
- [ ] LED 상태 확인 (정상 초기화)
- [ ] 시리얼 모니터 완전 종료
- [ ] ESP32 하드 리셋 수행

### BLE 검색 테스트
- [ ] Chrome Web Bluetooth로 검색
- [ ] Edge 브라우저로 검색
- [ ] Android nRF Connect로 검색
- [ ] iPhone Bluetooth 설정에서 검색
- [ ] 다른 컴퓨터에서 검색

### 환경 격리 테스트
- [ ] WiFi 라우터 끄고 테스트
- [ ] 다른 BLE 장치들 끄고 테스트
- [ ] 1m 이내 근거리에서 테스트
- [ ] 금속 물체 없는 환경에서 테스트

---

## 코드 변경 이력

### v1.0 (초기 버전)
```cpp
// BLESimple.cpp - 기본 구현
NimBLEDevice::init("ESP32");
// 기본 서비스/특성 생성
// 기본 광고 설정
```

### v2.0 (프로덕션 리팩토링)
```cpp
// ble_manager.cpp - 모듈화된 구현
class BLEManager {
    static bool initialize();
    static BLEReceivedData getReceivedData();
    static bool sendResponse(const String& response);
    // ...
};
```

---

## 참고 자료

### 성공 사례 (다른 프로젝트)
- ESP32 BLE 기본 예제들이 정상 작동하는 경우가 많음
- NimBLE 라이브러리 공식 예제들 참조 필요

### 실패 사례 패턴
- 복잡한 보안 설정시 연결 실패
- 큰 MTU 설정시 호환성 문제
- Serial과 BLE 동시 사용시 충돌

### 추천 디버깅 도구
- **nRF Connect** (Android/iOS): BLE 스캔 및 연결 테스트
- **Chrome Bluetooth Internals**: `chrome://bluetooth-internals/`
- **ESP32 Bluetooth Classic**: BLE 대신 Classic Bluetooth 테스트

---

*이 문서는 BLE 문제 해결 시마다 지속적으로 업데이트됩니다.*