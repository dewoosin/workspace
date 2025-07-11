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

**결과**: 여전히 BLE 검색 안됨, 연결 시 무한 루프
**상태**: 문제 지속

---

### 19:00 - 최소 코드로 롤백 테스트
**접근법**: 모든 복잡한 코드 제거하고 가장 기본적인 BLE만 테스트
**변경사항**:
1. 모든 모듈 파일 비활성화 (.disabled 확장자 추가)
2. 가장 단순한 BLE 광고 코드만 사용:
```cpp
void setup() {
    delay(1000);
    NimBLEDevice::init("GHOSTYPE");
    NimBLEServer* pServer = NimBLEDevice::createServer();
    NimBLEService* pService = pServer->createService("12345678-1234-5678-9012-123456789abc");
    pService->start();
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID("12345678-1234-5678-9012-123456789abc");
    pAdvertising->start();
}
```

**목적**: 
- 복잡한 코드가 문제인지 확인
- 이전에 작동했던 최소 설정으로 테스트
- HID, Parser, BLE Manager 등 모든 복잡성 제거

**상태**: 테스트 준비 완료

---

### 19:15 - 극단적 부트 루프 지속
**문제**: 가장 단순한 BLE 코드도 부트 루프 발생
**접근법**: 모든 기능 제거하고 빈 스케치로 테스트

**1단계 - 빈 코드 테스트**:
```cpp
void setup() {
    pinMode(2, OUTPUT);  // LED만
}
void loop() {
    digitalWrite(2, HIGH);
    delay(1000);
    digitalWrite(2, LOW);
    delay(1000);
}
```

**2단계 - platformio.ini 최소화**:
- 모든 build_flags 제거
- 모든 특수 설정 제거
- 기본 설정만 유지

**의심 원인**:
1. 펌웨어 파티션 손상
2. 플래시 메모리 문제
3. 부트로더 손상
4. 전원 공급 불안정

**해결 시도**:
1. 완전한 플래시 지우기 필요
2. 부트로더 재설치 고려
3. 다른 ESP32-S3 보드로 테스트

**상태**: 심각한 하드웨어/펌웨어 수준 문제 의심

---

### 19:30 - 플래시 지우기 시도
**문제**: esptool 명령에 반응 없음
**원인**: 부트 루프로 인해 플래시 모드 진입 불가

**해결 방법**:
1. **수동 플래시 모드 진입** (중요!):
   - BOOT 버튼 누른 상태에서
   - RESET 버튼 짧게 누르고 떼기
   - BOOT 버튼 1-2초 더 유지 후 떼기

2. **긴급 복구 절차**:
   ```bash
   # 플래시 완전 지우기
   python -m esptool --chip esp32s3 --port COM3 erase_flash
   
   # PlatformIO로 지우기
   pio run --target erase
   ```

3. **포트 확인**:
   - 장치 관리자에서 COM 포트 확인
   - 부트 루프 중에는 포트가 계속 변경될 수 있음

**상태**: 수동 개입 필요

---

### 19:45 - 부트 루프 해결!
**결과**: 플래시 지우기 후 부트 루프 사라짐
**현상**:
```
Disconnected (ClearCommError failed...)
Reconnecting to COM3     Connected!
```

**해결 과정**:
1. BOOT+RESET 버튼으로 플래시 모드 진입
2. 플래시 메모리 완전 지우기
3. 기본 LED 깜빡임 코드 업로드

**다음 단계**:
1. ✅ 기본 코드로 정상 작동 확인
2. 단계적으로 BLE 기능 추가
3. 문제 발생 지점 정확히 파악

**교훈**: 
- 복잡한 설정(USB HID + BLE)이 플래시 손상 유발
- 플래시 지우기가 필수적인 복구 방법

**상태**: 기본 동작 정상, BLE 테스트 준비

---

### 20:00 - T-Dongle-S3 보드 확인
**중요 발견**: 사용 중인 보드가 일반 ESP32-S3가 아닌 **T-Dongle-S3**

**T-Dongle-S3 특징**:
- 0.96인치 ST7735 LCD 디스플레이 내장
- USB 동글 형태 (컴팩트 디자인)
- TF 카드 슬롯
- 내장 LED가 없거나 다른 핀에 연결
- 일반 DevKit과 다른 핀맵

**변경사항**:
1. platformio.ini 수정:
   - T-Dongle-S3 전용 설정 추가
   - USB CDC 모드 활성화
   - PSRAM 지원 활성화

2. 테스트 코드 수정:
   - BOOT 버튼 (GPIO0) 입력 테스트
   - 시리얼 출력으로 동작 확인

**주의사항**:
- T-Dongle-S3는 내장 LED가 없을 수 있음
- LCD 디스플레이를 활용한 상태 표시 가능
- USB 동글 형태로 전원/데이터 동시 공급

**상태**: 보드별 맞춤 설정 적용

---

### 20:30 - T-Dongle-S3 BLE 검색 실패
**현상**:
- 시리얼 통신 정상 (버튼 카운트 출력됨)
- BLE 장치가 검색되지 않음
- COM 포트가 COM3/COM4로 변경됨

**가능한 원인**:
1. T-Dongle-S3의 BLE 안테나 문제
2. NimBLE 라이브러리와 T-Dongle-S3 호환성
3. 플래시 파티션 설정 문제
4. BLE 스택 초기화 실패

**디버그 코드 추가**:
- ESP32 칩 ID 출력
- BLE 초기화 각 단계별 확인
- 광고 시작 성공/실패 확인

**다음 시도**:
1. 파티션 테이블 확인
2. 다른 BLE 라이브러리 시도 (ArduinoBLE)
3. ESP32 기본 BLE 예제 테스트

**상태**: 문제 진단 중

---

### 21:00 - 🎉 BLE 검색 성공!
**중요한 돌파구**: ESP32 네이티브 BLE 라이브러리로 변경 후 검색 성공!

**성공 요인**:
1. NimBLE → ESP32 네이티브 BLE 라이브러리 변경
2. T-Dongle-S3 전용 파티션 설정
3. 단계별 초기화 과정 추가

**현재 상태**:
- ✅ BLE 장치 검색됨 ("GHOSTYPE-S3")
- ❌ BLE 연결 실패

**다음 단계**: 연결 문제 해결
- 연결 시 오류 메시지 분석
- 페어링 과정 최적화
- 특성(Characteristic) 설정 확인

**교훈**: 
- T-Dongle-S3는 ESP32 네이티브 BLE와 호환성이 더 좋음
- NimBLE 라이브러리는 일부 ESP32-S3 변종에서 문제 발생 가능

**상태**: BLE 검색 성공, 연결 문제 해결 중

---

### 21:30 - 🎉🎉 BLE 완전 성공!
**최종 해결책**: ESP32 네이티브 BLE + 보안 비활성화 + Web Bluetooth 표준 UUID

**성공한 구성**:
1. **라이브러리**: ESP32 네이티브 BLE (NimBLE 사용 안 함)
2. **UUID**: Nordic UART Service 표준
   - Service: `6e400001-b5a3-f393-e0a9-e50e24dcca9e`
   - RX: `6e400002-b5a3-f393-e0a9-e50e24dcca9e`
   - TX: `6e400003-b5a3-f393-e0a9-e50e24dcca9e`
3. **보안**: 완전 비활성화 (ESP_LE_AUTH_NO_BOND)
4. **USB 모드**: CDC만 활성화 (HID 비활성화)

**작동 확인**:
- ✅ BLE 장치 검색
- ✅ 안정적인 연결 유지
- ✅ 양방향 데이터 통신
- ✅ 프로토콜 파싱 (GHTYPE_KOR/ENG/CFG)
- ⚠️ HID 타이핑은 T-Dongle-S3 한계로 시뮬레이션만

**핵심 교훈**:
1. T-Dongle-S3는 ESP32 네이티브 BLE가 더 안정적
2. Web Bluetooth는 보안 설정에 민감함
3. UUID가 정확히 일치해야 연결 가능
4. BLE와 USB HID 동시 사용 시 충돌 발생

**상세 가이드**: BLE_SUCCESS_GUIDE.md 참조

**상태**: ✅ BLE 통신 완전 성공!

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