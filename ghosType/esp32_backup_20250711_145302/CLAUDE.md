# ESP32 펌웨어 아키텍처 - GHOSTYPE

## 개요

ESP32 펌웨어는 구조화된 프로토콜 명령을 호스트 컴퓨터의 키보드 동작으로 변환하는 정교한 **BLE-to-USB HID 브리지** 역할을 합니다. **T-Dongle-S3 하드웨어**에 특별히 최적화된 단일 파일, 모놀리식 아키텍처를 구현하며, 언어 모드 전환 기능을 갖춘 정밀한 USB HID 키보드 에뮬레이션을 통해 한국어와 영어 텍스트 입력을 모두 처리합니다.

## 아키텍처

### 현재 구현
- **단일 파일 설계**: 모든 기능이 `main.cpp`에 통합 (523줄)
- **Nordic UART 서비스**: 크로스 플랫폼 호환성을 위한 업계 표준 BLE 서비스
- **T-Dongle-S3 최적화**: ESP32-S3 칩셋을 위한 하드웨어별 최적화
- **듀얼 코어 운영**: BLE는 코어 0, 메인 처리는 코어 1에서 실행
- **제로 보안**: Web Bluetooth 호환성을 위해 완전히 개방

### BLE 서버 구성
```cpp
// 서비스 UUID
#define SERVICE_UUID        "6e400001-b5a3-f393-e0a9-e50e24dcca9e"  // Nordic UART
#define RX_CHAR_UUID        "6e400002-b5a3-f393-e0a9-e50e24dcca9e"  // 클라이언트 → ESP32
#define TX_CHAR_UUID        "6e400003-b5a3-f393-e0a9-e50e24dcca9e"  // ESP32 → 클라이언트

// GATT 구성
- MTU 크기: 185 바이트 (안정성을 위해 최적화)
- 보안: 없음 (Web Bluetooth를 위한 개방 접근)
- 연결 제한: 단일 클라이언트 연결
- 자동 재연결: 연결 해제 시 즉시 광고 재시작
```

### 하드웨어 통합
```cpp
// T-Dongle-S3 사양
- CPU: ESP32-S3 @ 240MHz 듀얼 코어
- 메모리: 320KB SRAM + PSRAM 지원
- 플래시: 4MB QSPI (80MHz 속도)
- USB: 단일 USB-C 포트 (전원 + HID)
- 디스플레이: 0.96" LCD (현재 구현에서는 미사용)
- 제한사항: 내장 LED 없음, USB 포트 공유
```

## 컴포넌트 플로우

### 1. BLE 메시지 수신
```cpp
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        String value = pCharacteristic->getValue();
        if (xSemaphoreTake(queueMutex, portMAX_DELAY) == pdTRUE) {
            typingQueue.push(value);
            xSemaphoreGive(queueMutex);
        }
    }
};
```

### 2. 프로토콜 파싱 엔진
**최신 프로토콜 명령:**
```cpp
#CMD:HANGUL     // 한국어 IME 모드로 전환
#CMD:ENGLISH    // 영어 IME 모드로 전환
#TEXT:content   // 지정된 텍스트 내용 타이핑
#CMD:ENTER      // Enter 키 누르기
```

**레거시 프로토콜 지원:**
```cpp
GHTYPE_KOR:text     // 한국어 텍스트 (하위 호환성)
GHTYPE_ENG:text     // 영어 텍스트 (하위 호환성)
GHTYPE_CFG:json     // 구성 JSON (속도 제어)
```

### 3. 언어 모드 관리
```cpp
bool isKoreanMode = false;

void toggleToKoreanMode() {
    if (!isKoreanMode) {
        keyboard.write(KEY_HANGUL);  // 0xF2 - 한국어 토글 키
        delay(800);                  // 보수적인 IME 전환 지연
        isKoreanMode = true;
    }
}
```

### 4. USB HID 키보드 출력
```cpp
#include <USB.h>
#include <USBHIDKeyboard.h>

USBHIDKeyboard keyboard;

// 구성 가능한 속도로 문자별 타이핑
void typeText(String text) {
    for (char c : text) {
        keyboard.write(c);
        delay(1000 / globalTypingSpeed);  // 1-50 CPS 구성 가능
    }
}
```

### 5. 전체 데이터 플로우
```
BLE 클라이언트 → RX 특성 → 프로토콜 파서 → 언어 전환기 → USB HID → 호스트 시스템
      ↓             ↓           ↓             ↓           ↓         ↓
 웹/iOS 앱 → Nordic UART → 명령 큐 → IME 제어 → 키 이벤트 → 텍스트 입력
```

## 기술적 이슈

### 현재 제한사항

#### 1. **아키텍처 제약**
- **모놀리식 설계**: 모든 기능이 단일 파일에 있어 유지보수 및 확장이 어려움
- **메모리 관리**: Arduino String 클래스의 과도한 사용으로 힙 단편화 발생
- **구성 영속성 없음**: 전원 주기 시 설정 재설정
- **제한된 오류 보고**: 클라이언트 애플리케이션에 대한 최소한의 피드백

#### 2. **하드웨어 종속성**
- **T-Dongle-S3 전용**: 수정 없이는 다른 ESP32 변형으로 이식 불가
- **단일 USB 포트**: HID와 디버깅(시리얼 모니터)을 동시에 사용할 수 없음
- **전원 제한**: 집중적인 BLE + HID 작업에는 USB 전원이 부족할 수 있음
- **상태 표시기 없음**: 시각적 상태 피드백을 위한 내장 LED 없음

#### 3. **성능 병목 현상**
```cpp
// 식별된 성능 문제
std::queue<String> typingQueue;           // 무한 큐 - 메모리 위험
String 연산 전반;                         // 힙 단편화
고정 800ms IME 지연;                      // 보수적이지만 비효율적
문자별 처리;                              // 배치 최적화 없음
```

#### 4. **보안 취약점**
- **인증 없음**: 누구나 연결하고 타이핑 명령을 보낼 수 있음
- **암호화 없음**: 모든 BLE 통신이 평문
- **명령 주입**: 악의적인 키보드 명령 시퀀스 가능성
- **입력 검증 없음**: 들어오는 명령에 대한 제한된 삭제

#### 5. **프로토콜 제한사항**
- **MTU 제약**: 185바이트 최대 메시지 크기로 큰 텍스트 블록 제한
- **압축 없음**: 반복적인 콘텐츠에 대한 비효율적인 대역폭 사용
- **레거시 지원**: 여러 프로토콜 버전으로 복잡성 증가
- **ACK/NACK 없음**: 성공적인 명령 실행에 대한 제한된 확인

## 개선 권장사항

### 1. **모듈형 아키텍처 리팩토링**
```cpp
// 제안된 모듈형 구조
class BLEManager {
    void initializeService();
    void handleConnection();
    void processIncomingData();
};

class ProtocolParser {
    Command parseCommand(String input);
    bool validateCommand(Command cmd);
};

class HIDController {
    void setTypingSpeed(int cps);
    void switchLanguageMode(LanguageMode mode);
    void typeText(String text);
};

class ConfigManager {
    void loadSettings();
    void saveSettings();
    void handleConfigUpdate();
};
```

### 2. **향상된 메모리 관리**
```cpp
// String을 고정 버퍼로 교체
#define MAX_COMMAND_LENGTH 512
char commandBuffer[MAX_COMMAND_LENGTH];

// 큐를 위한 원형 버퍼 구현
template<size_t SIZE>
class CircularBuffer {
    char buffer[SIZE];
    size_t head, tail;
    
public:
    bool enqueue(const char* data, size_t len);
    size_t dequeue(char* data, size_t maxLen);
};
```

### 3. **고급 프로토콜 기능**
```cpp
// 확장된 프로토콜 명령
#CMD:SPEED:25          // 타이핑 속도를 25 CPS로 설정
#CMD:BATCH:10          // 작업당 10개 문자 배치
#CMD:WAIT:1000         // 1000ms 지연 삽입
#CMD:STATUS            // 장치 상태 요청
#ACK:SUCCESS           // 명령 확인
#ERR:INVALID_CMD       // 오류 보고
```

### 4. **하드웨어 추상화 계층**
```cpp
class HardwareAbstraction {
    virtual void initializeDisplay() = 0;
    virtual void updateStatus(String message) = 0;
    virtual void setLED(LEDState state) = 0;
    virtual bool hasDisplay() = 0;
};

class TDongleS3Hardware : public HardwareAbstraction {
    // T-Dongle-S3 전용 구현
};
```

### 5. **향상된 보안 프레임워크**
```cpp
class SecurityManager {
    bool authenticateDevice(String deviceId);
    String encryptCommand(String plaintext);
    String decryptCommand(String ciphertext);
    bool validateCommandSignature(String command, String signature);
};

// 보안 페어링 프로세스
enum PairingState { UNPAIRED, PAIRING, PAIRED };
class PairingManager {
    void initiatePairing();
    bool exchangeKeys();
    void completePairing();
};
```

### 6. **성능 최적화**
```cpp
// 향상된 처리량을 위한 배치 처리
class BatchProcessor {
    void addCharacter(char c);
    void processBatch();
    void flushBuffer();
    
private:
    char batchBuffer[32];
    size_t batchSize;
    unsigned long lastBatchTime;
};

// 시스템 응답에 기반한 적응형 타이밍
class AdaptiveTimer {
    void adjustDelay(bool success);
    unsigned long getOptimalDelay();
    
private:
    unsigned long baseDelay;
    float adaptationFactor;
};
```

## 미래 서버 통합 계획

### 1. **서버 매개 아키텍처**
```
미래 상태: 클라이언트 앱 → API 게이트웨이 → BLE 프록시 → ESP32 → USB HID
현재 상태: 클라이언트 앱 → 직접 BLE → ESP32 → USB HID
```

### 2. **ESP32 역할 진화**
```cpp
// 서버 통합을 위한 미래 ESP32 개선사항
class ServerConnector {
    void connectToWiFi();
    void registerWithServer();
    void receiveCommands();
    void reportStatus();
};

// 하이브리드 연결성
enum ConnectionMode {
    DIRECT_BLE,      // 현재 모드 - 직접 클라이언트 연결
    SERVER_PROXY,    // 서버 매개 명령
    DUAL_MODE        // 직접 및 서버 연결 모두
};
```

### 3. **향상된 장치 관리**
```cpp
// 장치 등록 및 인증
struct DeviceInfo {
    String deviceId;
    String firmwareVersion;
    String hardwareRevision;
    uint32_t capabilities;
};

class DeviceManager {
    void registerDevice(DeviceInfo info);
    void updateFirmware(String version);
    void reportHealth();
    void syncConfiguration();
};
```

### 4. **실시간 상태 보고**
```cpp
// 텔레메트리 및 모니터링
struct DeviceMetrics {
    float cpuUsage;
    size_t freeMemory;
    int bleConnectionCount;
    uint32_t commandsProcessed;
    float batteryLevel;
};

class TelemetryReporter {
    void collectMetrics();
    void reportToServer();
    void handleAlerts();
};
```

### 5. **무선 업데이트**
```cpp
// OTA 업데이트 기능
class OTAManager {
    void checkForUpdates();
    bool downloadFirmware(String version);
    bool verifyFirmware();
    void installUpdate();
    void rollbackOnFailure();
};
```

### 6. **다중 클라이언트 지원**
```cpp
// 향상된 연결 관리
class MultiClientManager {
    void handleNewConnection(BLEClient client);
    void balanceLoad();
    void prioritizeCommands();
    void manageQueues();
    
private:
    std::vector<BLEClient> activeClients;
    CommandQueue priorityQueue;
    CommandQueue normalQueue;
};
```

## 개발 통합 가이드라인

### 1. **서버 통신 프로토콜**
```cpp
// 미래 서버 API 통합
enum APICommand {
    REGISTER_DEVICE,
    TYPE_TEXT,
    SET_CONFIG,
    GET_STATUS,
    UPDATE_FIRMWARE
};

struct APIMessage {
    APICommand command;
    String deviceId;
    String payload;
    uint32_t timestamp;
    String signature;
};
```

### 2. **구성 동기화**
```cpp
// 서버와 설정 동기화
class ConfigSync {
    void uploadCurrentConfig();
    void downloadServerConfig();
    void mergeConfigurations();
    void resolveConflicts();
};
```

### 3. **서비스 품질**
```cpp
// 우선순위 명령 처리
enum CommandPriority {
    CRITICAL,    // 시스템 명령
    HIGH,        // 실시간 타이핑
    NORMAL,      // 표준 텍스트
    LOW          // 백그라운드 작업
};

class QoSManager {
    void prioritizeCommand(Command cmd, CommandPriority priority);
    void processHighPriorityQueue();
    void throttleLowPriorityCommands();
};
```

---

**현재 상태**: ✅ **프로덕션 준비 완료** (직접 BLE 모드)  
**미래 준비**: 🔄 **서버 통합 계획됨**  
**최종 업데이트**: 2025년 7월  
**대상 하드웨어**: T-Dongle-S3 (ESP32-S3)  
**BLE 프로토콜**: Nordic UART 서비스  
**보안 수준**: 개방 (직접) / 암호화 (미래 서버 모드)