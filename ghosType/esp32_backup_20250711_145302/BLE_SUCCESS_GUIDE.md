# BLE 성공 가이드 - GHOSTYPE T-Dongle-S3

## 🎉 최종 성공 구성
이 문서는 T-Dongle-S3에서 BLE 연결이 성공적으로 작동한 설정을 상세히 기록합니다.

---

## 1. 하드웨어 정보
- **보드**: T-Dongle-S3 (LilyGO)
- **칩**: ESP32-S3
- **특징**: 0.96인치 LCD, USB 동글 형태
- **제약사항**: BLE와 USB HID 동시 사용 불가

---

## 2. 개발 환경
- **IDE**: VS Code + PlatformIO
- **플랫폼**: Windows
- **업로드 포트**: COM3/COM4 (변동 가능)

---

## 3. PlatformIO 설정 (platformio.ini)

```ini
[env:lilygo-t-dongle-s3]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino

; T-Dongle-S3 전용 설정
board_build.mcu = esp32s3
board_build.f_cpu = 240000000L
board_build.f_flash = 80000000L
board_build.flash_mode = qio
board_build.arduino.memory_type = qio_opi
board_build.partitions = default.csv

; USB 설정 (중요: USB_MODE는 제거해야 BLE 안정)
build_flags = 
    -DARDUINO_USB_CDC_ON_BOOT=1
    -DBOARD_HAS_PSRAM
    -DCORE_DEBUG_LEVEL=1

; 업로드 설정
upload_speed = 115200  ; 속도를 낮춰서 안정성 향상
monitor_speed = 115200
monitor_filters = direct
upload_resetmethod = nodemcu

; 라이브러리 (중요: NimBLE 대신 ESP32 네이티브 BLE 사용)
lib_deps = 
    ; NimBLE는 사용하지 않음!
    adafruit/Adafruit GFX Library@^1.11.5
    adafruit/Adafruit ST7735 and ST7789 Library@^1.10.0
```

---

## 4. 핵심 라이브러리 및 헤더

```cpp
#include <Arduino.h>
#include <BLEDevice.h>      // ESP32 네이티브 BLE
#include <BLEServer.h>      
#include <BLEUtils.h>
#include <BLE2902.h>        // Notify를 위한 Descriptor
#include <esp_gap_ble_api.h> // 보안 설정용
```

**중요**: NimBLE 라이브러리는 사용하지 않습니다! ESP32 네이티브 BLE만 사용.

---

## 5. BLE UUID 설정 (Nordic UART Service)

```cpp
#define SERVICE_UUID        "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define RX_CHAR_UUID        "6e400002-b5a3-f393-e0a9-e50e24dcca9e"  // 웹 → ESP32
#define TX_CHAR_UUID        "6e400003-b5a3-f393-e0a9-e50e24dcca9e"  // ESP32 → 웹
```

**중요**: 웹 브라우저와 정확히 일치해야 함!

---

## 6. BLE 보안 설정 (Web Bluetooth 호환)

```cpp
// BLE 초기화
BLEDevice::init("GHOSTYPE-S3");

// 보안 완전 비활성화 (Web Bluetooth 호환)
esp_ble_auth_req_t auth_req = ESP_LE_AUTH_NO_BOND;
esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;
uint8_t key_size = 16;
uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
uint8_t rsp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, sizeof(uint8_t));
esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, sizeof(uint8_t));
esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, sizeof(uint8_t));
```

**핵심**: 보안을 완전히 비활성화해야 Web Bluetooth와 호환됨

---

## 7. BLE 서버 및 콜백 설정

### 서버 콜백
```cpp
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        Serial.println("🔗 *** BLE DEVICE CONNECTED! ***");
    };

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        Serial.println("❌ *** BLE DEVICE DISCONNECTED ***");
        
        // 광고 재시작 (중요!)
        delay(500);
        pServer->getAdvertising()->start();
    }
};
```

### 데이터 수신 콜백
```cpp
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string rxValue = pCharacteristic->getValue();
        
        if (rxValue.length() > 0) {
            // 데이터 처리
            processTypingCommand(rxValue);
            
            // 응답 전송
            if (pTxCharacteristic && deviceConnected) {
                String response = "OK:Received " + String(rxValue.length()) + " chars";
                pTxCharacteristic->setValue(response.c_str());
                pTxCharacteristic->notify();
            }
        }
    }
};
```

---

## 8. BLE 초기화 순서 (중요!)

```cpp
void setup() {
    // 1. BLE 장치 초기화
    BLEDevice::init("GHOSTYPE-S3");
    
    // 2. 보안 설정
    // (위의 보안 설정 코드)
    
    // 3. 서버 생성
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    
    // 4. 서비스 생성
    BLEService *pService = pServer->createService(SERVICE_UUID);
    
    // 5. RX 특성 생성 (웹 → ESP32)
    pRxCharacteristic = pService->createCharacteristic(
                        RX_CHAR_UUID,
                        BLECharacteristic::PROPERTY_WRITE
                      );
    pRxCharacteristic->setCallbacks(new MyCallbacks());
    
    // 6. TX 특성 생성 (ESP32 → 웹)
    pTxCharacteristic = pService->createCharacteristic(
                        TX_CHAR_UUID,
                        BLECharacteristic::PROPERTY_READ |
                        BLECharacteristic::PROPERTY_NOTIFY
                      );
    pTxCharacteristic->addDescriptor(new BLE2902()); // 중요!
    
    // 7. 서비스 시작
    pService->start();
    
    // 8. 광고 설정 및 시작
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  
    pAdvertising->setMaxPreferred(0x12);
    BLEDevice::startAdvertising();
}
```

---

## 9. 프로토콜 형식

### 수신 데이터 형식
- **설정**: `GHTYPE_CFG:{"mode":"typing","speed_cps":6}`
- **한글**: `GHTYPE_KOR:텍스트내용`
- **영문**: `GHTYPE_ENG:text content`

### 응답 형식
- **성공**: `OK:Received X chars`
- **오류**: `ERR:메시지`

---

## 10. 문제 해결 과정에서 배운 것

### 작동하지 않은 것들
1. **NimBLE 라이브러리**: T-Dongle-S3와 호환성 문제
2. **복잡한 보안 설정**: Web Bluetooth 연결 실패
3. **USB HID 동시 사용**: BLE와 충돌 발생
4. **높은 MTU 설정**: 연결 불안정

### 필수 설정
1. **ESP32 네이티브 BLE 사용**: NimBLE 대신
2. **보안 비활성화**: Web Bluetooth 호환성
3. **BLE2902 Descriptor 추가**: Notify 기능 필수
4. **광고 재시작**: 연결 해제 시 필수

---

## 11. 테스트 절차

1. **업로드**
   - BOOT 버튼 누른 상태에서 업로드
   - COM 포트 확인 필수 (COM3/COM4 변동)

2. **BLE 검색**
   - Chrome Web Bluetooth
   - 장치명: "GHOSTYPE-S3"

3. **연결 확인**
   - 시리얼 모니터: "🔗 *** BLE DEVICE CONNECTED! ***"
   - 5초마다 상태 체크 메시지

4. **데이터 전송**
   - 웹에서 텍스트 입력
   - ESP32 수신 확인
   - 응답 메시지 확인

---

## 12. 주의사항

1. **USB 모드 설정**
   - `ARDUINO_USB_MODE=1` 사용하면 BLE 불안정
   - CDC 모드만 활성화

2. **시리얼 모니터**
   - 업로드 중에는 종료 필수
   - UTF-8 인코딩 지원 확인

3. **전원 공급**
   - USB 동글 형태로 전원 불안정 가능
   - 안정적인 USB 포트 사용

---

## 13. 현재 제약사항

1. **HID 키보드 기능**
   - T-Dongle-S3에서는 BLE와 동시 사용 불가
   - 시뮬레이션만 가능

2. **LCD 디스플레이**
   - 초기화 복잡, 현재 미사용

3. **메모리 사용**
   - PSRAM 활성화 필수
   - 큰 데이터 처리 시 주의

---

## 14. 다음 단계 권장사항

1. **다른 하드웨어 사용**
   - ESP32-S3 DevKitC: BLE + HID 동시 지원
   - 일반 ESP32: 더 안정적인 BLE

2. **아키텍처 분리**
   - BLE 전용 ESP32
   - HID 전용 ESP32
   - 시리얼 통신으로 연결

---

*이 문서는 2024년 12월 기준 T-Dongle-S3에서 검증된 설정입니다.*