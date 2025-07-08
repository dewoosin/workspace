// src/main.cpp
// GHOSTYPE Production Firmware - Smart Keyboard Mode Switching

#include <Arduino.h>
#include "BLEConfig.h"
#include "BLENimbleManager.h"
#include "HangulQWERTY.h"

// ESP32 system headers (Arduino framework compatible)
#ifdef ESP32
  // Use Arduino ESP32 equivalents instead of ESP-IDF headers
  // esp_system.h functions are available through Arduino.h
  // esp_chip_info.h functions are available through ESP.h
#endif

// USB HID keyboard for T-Dongle-S3 hardware
#include <USB.h>
#include <USBHIDKeyboard.h>

// Global hardware objects
BLENimbleManager* bleManager = nullptr;  // BLE communication handler
USBHIDKeyboard Keyboard;                 // USB HID keyboard interface

// System state variables
unsigned long lastStatusUpdate = 0;      // For periodic status checks
unsigned long lastHeartbeat = 0;         // For LED heartbeat indicator
bool systemReady = false;                // Overall system initialization status
bool usbHidReady = false;                // USB keyboard availability flag
uint8_t errorCount = 0;                  // Error counter for auto-recovery

// Typing state management - 타이핑 상태 관리
bool isTyping = false;                   // Current typing operation flag / 현재 타이핑 작업 플래그
unsigned long typingStartTime = 0;       // Start time of current typing session / 현재 타이핑 세션 시작 시간
static const uint32_t MAX_TYPING_DURATION = 300000;  // 300 second (5분) timeout / 300초 (5분) 타임아웃

// Command queuing system - 명령 큐잉 시스템
#define MAX_COMMAND_QUEUE 20  // 증가: 5 -> 20
std::string commandQueue[MAX_COMMAND_QUEUE];
int queueHead = 0;
int queueTail = 0;
int queueCount = 0;

// Long text handling - 긴 텍스트 처리
uint32_t totalCharsReceived = 0;   // 총 수신 문자수
uint32_t totalCharsTyped = 0;      // 총 타이핑 문자수
unsigned long sessionStartTime = 0; // 세션 시작 시간

// Chunk management - 청크 관리
struct ChunkInfo {
    String id;                     // 청크 ID
    uint16_t sequence;            // 시퀀스 번호
    String checksum;              // 체크섬
    unsigned long receivedAt;     // 수신 시각
    bool processed;               // 처리 완료 여부
};

#define MAX_CHUNK_HISTORY 10      // 최대 청크 히스토리
ChunkInfo chunkHistory[MAX_CHUNK_HISTORY];
int chunkHistoryIndex = 0;
uint16_t expectedSequence = 0;    // 예상 다음 시퀀스

// Typing speed control (adjustable via web interface)
// 타이핑 속도 제어 (웹 인터페이스를 통해 조정 가능)
uint8_t typingSpeedCPS = 6;              // Characters per second (default: 6) / 초당 문자 수
uint32_t baseTypingDelay = 167;          // Base delay between keystrokes (1000ms / 6 cps) / 키 입력 간 기본 지연시간
uint32_t intervalMS = 100;               // Interval pause after N characters / N개 문자 후 추가 지연시간
uint8_t intervalCharCount = 5;           // Characters before interval pause / 간격 지연 전 문자 수

// Keyboard mode management for Korean/English switching
enum KeyboardMode {
    MODE_UNKNOWN = 0,    // Initial state - mode not determined
    MODE_ENGLISH = 1,    // English input mode
    MODE_KOREAN = 2      // Korean input mode (Hangul)
};

KeyboardMode currentKeyboardMode = MODE_UNKNOWN;  // Current active keyboard mode
unsigned long lastModeChange = 0;                 // Timestamp of last mode switch

// BLE data protocol constants
const char PROTOCOL_PREFIX[] = "GHTYPE_";
const char PROTOCOL_ENGLISH[] = "GHTYPE_ENG:";
const char PROTOCOL_KOREAN[] = "GHTYPE_KOR:";
const char PROTOCOL_SPECIAL[] = "GHTYPE_SPE:";
const char PROTOCOL_CONFIG[] = "GHTYPE_CFG:";

// Function declarations
void initializeSystem();
bool initializeHardware();
void safeDelay(uint32_t ms);
void ensureKeyboardMode(KeyboardMode targetMode);
void forceKeyboardMode(KeyboardMode mode);
String getKeyboardModeString(KeyboardMode mode);
void processReceivedData(const std::string& data);
void processEnglishText(const String& text);
void processKoreanJamo(const String& jamoKeys);
void processSpecialCommand(const String& command);
void processConfiguration(const String& config);
void typeWithSmartTiming(const String& text);
void updateTypingSpeed(uint8_t newSpeedCPS);
void handleSerialCommands();
void resetSystem();
void processWebClientJSON(const String& jsonStr);
String calculateChecksum(const String& text);
void sendChunkAck(const String& chunkId, bool success);
void addToChunkHistory(const String& id, uint16_t seq, const String& checksum);
bool isChunkDuplicate(const String& id);
bool enqueueCommand(const std::string& command);
bool dequeueCommand(std::string& command);
void processCommandQueue();

// Safe delay function that yields to prevent watchdog timeouts
void safeDelay(uint32_t ms) {
    uint32_t start = millis();
    while (millis() - start < ms) {
        yield();  // Allow other tasks to run
        delay(1);
    }
}

// Ensure keyboard is in correct mode before typing (with rate limiting)
void ensureKeyboardMode(KeyboardMode targetMode) {
    if (currentKeyboardMode == targetMode) {
        return;  // Already in target mode, no switch needed
    }
    
    // Prevent rapid mode changes (minimum 200ms between switches)
    if (millis() - lastModeChange < 200) {
        delay(200 - (millis() - lastModeChange));
    }
    
    forceKeyboardMode(targetMode);
}

// 안전한 키보드 모드 전환 - Alt+Shift 조합 사용 (한국어 IME 표준)
// Safe keyboard mode switch using Alt+Shift combination (Korean IME standard)
void forceKeyboardMode(KeyboardMode mode) {
    if (!usbHidReady) return;  // USB 키보드 사용 불가 시 건너뛰기
    
    // 잘못된 모드 값 검증
    // Validate mode value
    if (mode != MODE_ENGLISH && mode != MODE_KOREAN) {
        return;  // 유효하지 않은 모드
    }
    
    // 모드 전환 빈도 제한 (최소 500ms 간격)
    // Rate limiting for mode changes (minimum 500ms interval)
    unsigned long currentTime = millis();
    if (currentTime - lastModeChange < 500) {
        return;  // 너무 빠른 모드 전환 방지
    }
    
    // 알 수 없는 모드에서 영어로 초기화
    // Initialize to English from unknown mode
    if (currentKeyboardMode == MODE_UNKNOWN) {
        // Alt+Shift를 2회 전송하여 영어 모드 확실히 설정
        // Send Alt+Shift twice to ensure English mode
        for (int i = 0; i < 2; i++) {
            if (!usbHidReady) return;  // 중간에 USB 연결 끊어지면 중단
            
            Keyboard.press(KEY_LEFT_ALT);
            delay(50);
            Keyboard.press(KEY_LEFT_SHIFT);
            delay(50);
            Keyboard.releaseAll();
            delay(200);
        }
        currentKeyboardMode = MODE_ENGLISH;
        lastModeChange = currentTime;
    }
    
    // 목표 모드와 다르면 전환
    // Switch if different from target mode
    if (currentKeyboardMode != mode) {
        if (!usbHidReady) return;  // 최종 안전성 확인
        
        Keyboard.press(KEY_LEFT_ALT);
        delay(50);                      // 안정적인 감지를 위한 짧은 홀드
        Keyboard.press(KEY_LEFT_SHIFT);
        delay(50);                      // 안정적인 감지를 위한 짧은 홀드
        Keyboard.releaseAll();
        delay(500);                     // 모드 전환을 위한 긴 지연
        
        currentKeyboardMode = mode;
        lastModeChange = millis();
    }
}

// Convert keyboard mode enum to string for debugging/status
String getKeyboardModeString(KeyboardMode mode) {
    switch (mode) {
        case MODE_ENGLISH: return "English";
        case MODE_KOREAN:  return "Korean";
        case MODE_UNKNOWN: return "Unknown";
        default:           return "Error";
    }
}

// Main data processing function - parses protocol and routes to appropriate handler
// 메인 데이터 처리 함수 - 프로토콜 파싱 및 적절한 핸들러로 라우팅
void processReceivedData(const std::string& data) {
    String dataStr = String(data.c_str());
    
    // Check if currently typing - queue command if busy
    // 현재 타이핑 중인지 확인 - 바쁠 때 명령 큐에 추가
    if (isTyping) {
        enqueueCommand(data);
        return;
    }
    
    // Check if this is a JSON payload from web client
    // 웹 클라이언트의 JSON 페이로드인지 확인
    if (dataStr.startsWith("{") && dataStr.endsWith("}")) {
        processWebClientJSON(dataStr);
        return;
    }
    
    // Parse protocol prefix and route to appropriate processor
    // 프로토콜 접두사 파싱 및 적절한 프로세서로 라우팅
    if (dataStr.startsWith(PROTOCOL_ENGLISH)) {
        // Extract English text (everything after protocol prefix)
        String englishText = dataStr.substring(strlen(PROTOCOL_ENGLISH));
        processEnglishText(englishText);
        
    } else if (dataStr.startsWith(PROTOCOL_KOREAN)) {
        // Extract Korean jamo key sequence
        String jamoKeys = dataStr.substring(strlen(PROTOCOL_KOREAN));
        processKoreanJamo(jamoKeys);
        
    } else if (dataStr.startsWith(PROTOCOL_SPECIAL)) {
        // Extract special command
        String specialCmd = dataStr.substring(strlen(PROTOCOL_SPECIAL));
        processSpecialCommand(specialCmd);
        
    } else if (dataStr.startsWith(PROTOCOL_CONFIG)) {
        // Extract configuration JSON
        String configData = dataStr.substring(strlen(PROTOCOL_CONFIG));
        processConfiguration(configData);
        
    } else {
        // No protocol prefix - treat as plain English text
        // 프로토콜 접두사 없음 - 일반 영어 텍스트로 처리
        processEnglishText(dataStr);
    }
    
    // Update message counter for statistics
    // 통계를 위한 메시지 카운터 업데이트
    static uint32_t totalMessages = 0;
    totalMessages++;
}

// Process English text input with automatic mode switching
void processEnglishText(const String& text) {
    if (!usbHidReady || text.length() == 0) {
        return;  // Skip if USB keyboard unavailable or empty text
    }
    
    try {
        // Ensure keyboard is in English mode before typing
        ensureKeyboardMode(MODE_ENGLISH);
        
        // Type the text with natural timing
        typeWithSmartTiming(text);
        
    } catch (...) {
        // Silent error handling - increment error counter for monitoring
        errorCount++;
    }
}

// Process Korean text with automatic Hangul-to-QWERTY conversion
void processKoreanJamo(const String& koreanText) {
    if (!usbHidReady || koreanText.length() == 0) {
        return;  // Skip if USB keyboard unavailable or empty text
    }
    
    try {
        // Ensure keyboard is in Korean mode before typing
        ensureKeyboardMode(MODE_KOREAN);
        
        // Convert Korean text to QWERTY key sequence using proper jamo decomposition
        // 올바른 자모 분해를 사용하여 한국어 텍스트를 QWERTY 키 시퀀스로 변환
        String qwertyKeys = HangulQWERTY::hangulToQWERTY(koreanText);
        typeWithSmartTiming(qwertyKeys);
        
    } catch (...) {
        // Silent error handling - increment error counter for monitoring
        errorCount++;
    }
}

// Process special keyboard commands (enter, tab, ctrl+c, etc.)
void processSpecialCommand(const String& command) {
    if (!usbHidReady) {
        return;  // Skip if USB keyboard not available
    }
    
    String cmd = command;
    cmd.toLowerCase();  // Normalize to lowercase for comparison
    
    // Execute specific special commands
    if (cmd == "enter") {
        Keyboard.press(KEY_RETURN);
        Keyboard.releaseAll();
    }
    else if (cmd == "tab") {
        Keyboard.press(KEY_TAB);
        Keyboard.releaseAll();
    }
    else if (cmd == "backspace") {
        Keyboard.press(KEY_BACKSPACE);
        Keyboard.releaseAll();
    }
    else if (cmd == "space") {
        Keyboard.write(' ');
    }
    else if (cmd == "ctrl+c") {
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press('c');
        Keyboard.releaseAll();
    }
    else if (cmd == "ctrl+v") {
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press('v');
        Keyboard.releaseAll();
    }
    else if (cmd == "alt+tab") {
        Keyboard.press(KEY_LEFT_ALT);
        Keyboard.press(KEY_TAB);
        Keyboard.releaseAll();
    }
    else if (cmd == "haneng") {
        // Toggle between Korean and English modes
        // 한국어와 영어 모드 간 전환
        if (currentKeyboardMode == MODE_ENGLISH) {
            forceKeyboardMode(MODE_KOREAN);
        } else {
            forceKeyboardMode(MODE_ENGLISH);
        }
    }
    else if (cmd == "eng") {
        forceKeyboardMode(MODE_ENGLISH); // Force switch to English mode
    }
    else if (cmd == "kor") {
        forceKeyboardMode(MODE_KOREAN);  // Force switch to Korean mode
    }
    else if (cmd == "reset_mode") {
        currentKeyboardMode = MODE_UNKNOWN;  // Reset mode detection
    }
    else if (cmd == "sync_mode") {
        // Synchronize keyboard mode by forcing English first
        currentKeyboardMode = MODE_UNKNOWN;
        forceKeyboardMode(MODE_ENGLISH);
    }
    // Unknown commands are silently ignored
    
    delay(50);  // Brief delay for command processing
}

// Process configuration changes (JSON format from web interface)
void processConfiguration(const String& config) {
    // Simple JSON parsing for speed_cps field
    // Expected format: {"mode":"typing","speed_cps":6}
    
    int speedIndex = config.indexOf("\"speed_cps\":");
    if (speedIndex != -1) {
        // Extract speed_cps value from JSON
        int valueStart = speedIndex + 12; // Length of "speed_cps":
        int valueEnd = config.indexOf(',', valueStart);
        if (valueEnd == -1) valueEnd = config.indexOf('}', valueStart);
        
        if (valueEnd != -1) {
            String speedStr = config.substring(valueStart, valueEnd);
            speedStr.trim();
            int newSpeed = speedStr.toInt();
            
            // Validate speed range (1-20 characters per second)
            if (newSpeed >= 1 && newSpeed <= 20) {
                updateTypingSpeed(newSpeed);
            }
        }
    }
}

// Update typing speed and recalculate delays
void updateTypingSpeed(uint8_t newSpeedCPS) {
    typingSpeedCPS = newSpeedCPS;
    // Note: baseTypingDelay is now calculated in typeWithSmartTiming()
    // 참고: baseTypingDelay는 이제 typeWithSmartTiming()에서 계산됨
    
    // Send confirmation back to web interface via BLE
    if (bleManager) {
        String response = "SPD:" + String(typingSpeedCPS);
        bleManager->sendNotification(response.c_str());
    }
}

// 안전한 타이핑 함수 - 자연스러운 인간 타이핑 패턴
// Safe typing function with natural human-like timing patterns
void typeWithSmartTiming(const String& text) {
    // 입력 검증 및 길이 제한
    // Input validation and length limit
    if (text.length() == 0 || text.length() > 5000) {
        return;  // 빈 텍스트이거나 너무 긴 텍스트
    }
    
    // 안전한 CPS 기반 타이밍 계산
    // Safe CPS-based timing calculation
    if (typingSpeedCPS == 0) typingSpeedCPS = 1;  // 0으로 나누기 방지
    uint32_t charDelay = 1000 / typingSpeedCPS;
    
    // 지연 시간 범위 제한 (최소 20ms, 최대 2000ms)
    // Delay time range limit (min 20ms, max 2000ms)
    if (charDelay < 20) charDelay = 20;
    if (charDelay > 2000) charDelay = 2000;
    
    // 안전한 랜덤 범위 계산
    // Safe random range calculation
    uint32_t randomRange = charDelay / 5;
    if (randomRange > 200) randomRange = 200;  // 최대 200ms 변화
    
    // 무한 루프 방지용 카운터
    // Counter to prevent infinite loops
    int charCount = 0;
    const int MAX_CHARS = 5000;
    
    for (int i = 0; i < text.length() && charCount < MAX_CHARS; i++, charCount++) {
        char c = text.charAt(i);
        
        // 안전한 특수 문자 처리
        // Safe special character handling
        if (c == '\n') {
            if (usbHidReady) {
                Keyboard.press(KEY_RETURN);
                delay(30);
                Keyboard.releaseAll();
                delay(100);  // 고정 지연시간
            }
        } else if (c == '\t') {
            if (usbHidReady) {
                Keyboard.press(KEY_TAB);
                delay(30);
                Keyboard.releaseAll();
                delay(100);  // 고정 지연시간
            }
        } else if (c >= 'A' && c <= 'Z') {
            // 대문자 처리 (Shift 키 사용)
            // Handle uppercase letters with Shift modifier
            if (usbHidReady) {
                Keyboard.press(KEY_LEFT_SHIFT);
                delay(20);   // Shift 감지를 위한 짧은 홀드
                Keyboard.press(c);
                delay(30);   // 키 감지를 위한 짧은 홀드
                Keyboard.releaseAll();
                
                // 안전한 지연 시간 적용
                // Apply safe delay time
                uint32_t actualDelay = charDelay + (random(1000) % (randomRange + 1));
                if (actualDelay > 2000) actualDelay = 2000;
                delay(actualDelay);
            }
        } else if (c >= 0x20 && c <= 0x7E) {  // 출력 가능한 ASCII 문자만
            // 일반 문자 입력 (안전한 ASCII 범위)
            // Regular character typing (safe ASCII range)
            if (usbHidReady) {
                Keyboard.write(c);
                
                // 안전한 지연 시간 적용
                // Apply safe delay time
                uint32_t actualDelay = charDelay + (random(1000) % (randomRange + 1));
                if (actualDelay > 2000) actualDelay = 2000;
                delay(actualDelay);
            }
        }
        // 안전하지 않은 문자는 건너뛰기 (제어 문자 등)
        // Skip unsafe characters (control characters, etc.)
        
        // 간격 지연 적용 (안전한 범위 확인)
        // Apply interval pause (with safe range check)
        if (intervalCharCount > 0 && intervalCharCount <= 100 && 
            (i + 1) % intervalCharCount == 0) {
            
            uint32_t safeIntervalMS = intervalMS;
            if (safeIntervalMS > 5000) safeIntervalMS = 5000;  // 최대 5초 제한
            delay(safeIntervalMS);
        }
        
        // 주기적으로 yield() 호출 (와치독 타임아웃 방지)
        // Periodically call yield() (prevent watchdog timeout)
        if (charCount % 50 == 0) {
            yield();
        }
    }
}

// Initialize serial communication and display system info
void initializeSystem() {
    // Initialize serial for debugging/status (will be removed in production)
    // For now, keeping minimal initialization only
}

// Initialize USB HID keyboard and GPIO hardware
bool initializeHardware() {
    try {
        // Initialize RGB LED pin for status indication
        pinMode(RGB_LED_PIN, OUTPUT);
        digitalWrite(RGB_LED_PIN, LOW);
        
        // Initialize USB subsystem first
        USB.begin();
        safeDelay(1000);  // Wait for USB enumeration
        
        // Initialize HID keyboard interface
        Keyboard.begin();
        safeDelay(500);   // Wait for HID registration
        
        // 키보드 기능 테스트 제거 (자동 스페이스 방지)
        // Remove keyboard test (prevent automatic space)
        // Keyboard.write(' '); - 제거됨
        
        usbHidReady = true;
        
        // Set initial keyboard mode to English
        forceKeyboardMode(MODE_ENGLISH);
        
        return true;
        
    } catch (...) {
        // USB HID initialization failed - device will operate in serial mode only
        usbHidReady = false;
        return true;  // Continue anyway for BLE functionality
    }
}

// Handle serial commands for debugging and manual control
void handleSerialCommands() {
    if (!Serial.available()) return;
    
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toLowerCase();
    
    if (command.length() == 0) return;
    
    // Process various debug commands
    if (command == "status" || command == "s") {
        // Print system status information
        if (bleManager) {
            bleManager->printStatus();
        }
    }
    else if (command == "test") {
        // Test English typing functionality
        if (usbHidReady) {
            processEnglishText("GHOSTYPE Test!");
        }
    }
    else if (command == "testko") {
        // Test Korean typing functionality
        if (usbHidReady) {
            // "안녕" = ㅇㅏㄴㄴㅕㅇ = dkssud (jamo to QWERTY mapping)
            processKoreanJamo("dkssud");
        }
    }
    else if (command == "testko2") {
        // Test Korean with sync - more reliable
        if (usbHidReady) {
            currentKeyboardMode = MODE_UNKNOWN;  // Force resync
            processKoreanJamo("dkssud");  // "안녕"
        }
    }
    else if (command == "eng") {
        forceKeyboardMode(MODE_ENGLISH);
    }
    else if (command == "kor") {
        forceKeyboardMode(MODE_KOREAN);
    }
    else if (command == "reset" || command == "r") {
        resetSystem();
    }
    else if (command.startsWith("eng:")) {
        // Direct English text input: "eng:Hello World"
        String text = command.substring(4);
        processEnglishText(text);
    }
    else if (command.startsWith("kor:")) {
        // Direct Korean jamo input: "kor:dkssud"
        String jamo = command.substring(4);
        processKoreanJamo(jamo);
    }
    else if (command.startsWith("spe:")) {
        // Direct special command: "spe:enter"
        String special = command.substring(4);
        processSpecialCommand(special);
    }
    else if (command.startsWith("speed:")) {
        // Change typing speed: "speed:10"
        String speedStr = command.substring(6);
        int newSpeed = speedStr.toInt();
        if (newSpeed >= 1 && newSpeed <= 20) {
            updateTypingSpeed(newSpeed);
        }
    }
    else if (command == "testjson") {
        // Test JSON processing
        // JSON 처리 테스트
        String testJson = "{\"text\":\"Hello World!\",\"speed_cps\":10,\"interval_ms\":200}";
        processWebClientJSON(testJson);
    }
    else if (command == "testjsonko") {
        // Test JSON with Korean (will show detection)
        // 한국어 JSON 테스트 (감지 표시)
        String testJson = "{\"text\":\"안녕하세요\",\"speed_cps\":5,\"interval_ms\":300}";
        processWebClientJSON(testJson);
    }
    else if (command == "testhangul") {
        // Test Hangul-to-QWERTY conversion system
        // 한글-QWERTY 변환 시스템 테스트
        if (HangulQWERTY::runTests()) {
            // All tests passed - 모든 테스트 통과
        } else {
            // Some tests failed - 일부 테스트 실패
        }
    }
    else if (command == "testko1") {
        // Test basic Korean words - 기본 한국어 단어 테스트
        processKoreanJamo("가윤");  // Should type "rkdbs"
    }
    else if (command == "testko2") {
        // Test complex Korean cases - 복합 한국어 케이스 테스트
        processKoreanJamo("되돼맑");  // Should type "enlehoakfr"
    }
    else if (command == "testko3") {
        // Test edge cases - 예외 케이스 테스트
        processKoreanJamo("띄넓");  // Should type "Emlspfq"
    }
    // Unknown commands are silently ignored
}

// Perform system reset and restart ESP32
void resetSystem() {
    // Clean shutdown of USB HID interface
    if (usbHidReady) {
        Keyboard.end();
    }
    
    // Clean shutdown of BLE manager
    if (bleManager) {
        bleManager->stop();
        delete bleManager;
        bleManager = nullptr;
    }
    
    safeDelay(1000);  // Allow clean shutdown
    ESP.restart();    // Hardware restart
}

// Main setup function - called once at startup
void setup() {
    Serial.begin(115200);  // Initialize serial for debugging
    initializeSystem();
    
    // Initialize hardware components
    if (!initializeHardware()) {
        // Hardware initialization failed - halt system
        while (1) {
            safeDelay(1000);
        }
    }
    
    safeDelay(2000);  // Allow hardware to stabilize
    
    // Initialize BLE communication system
    try {
        bleManager = new BLENimbleManager();
        
        if (bleManager && bleManager->begin()) {
            systemReady = true;  // System fully operational
        }
    } catch (...) {
        // BLE initialization failed - continue without BLE functionality
        systemReady = false;
    }
    
    // Record startup timestamps
    lastStatusUpdate = millis();
    lastHeartbeat = millis();
}

// Main loop function - called continuously
void loop() {
    // Check for typing timeout - reset if stuck
    // 타이핑 타임아웃 확인 - 멈춘 경우 리셋
    if (isTyping && (millis() - typingStartTime > MAX_TYPING_DURATION)) {
        isTyping = false;  // Reset stuck typing state
    }
    
    // Process BLE data if system is ready and connected
    if (systemReady && bleManager) {
        try {
            // Check for fragment timeout
            // 분할 타임아웃 확인
            bleManager->checkFragmentTimeout();
            
            // Check for incoming BLE data
            if (bleManager->hasReceivedData()) {
                std::string receivedData = bleManager->getReceivedData();
                if (!receivedData.empty()) {
                    processReceivedData(receivedData);  // Parse and execute command
                }
            }
            
            // Process queued commands when not busy
            // 바쁘지 않을 때 대기열 명령 처리
            processCommandQueue();
        } catch (...) {
            // BLE error handling - increment error counter
            errorCount++;
            if (errorCount > 5) {
                resetSystem();  // Auto-recovery after too many errors
            }
        }
    }
    
    // Periodic status updates (every 30 seconds) - minimal overhead
    if (millis() - lastStatusUpdate > 30000) {
        lastStatusUpdate = millis();
        // Status update logic here if needed for monitoring
    }
    
    // LED heartbeat indicator (every 5 seconds)
    if (millis() - lastHeartbeat > 5000) {
        lastHeartbeat = millis();
        digitalWrite(RGB_LED_PIN, !digitalRead(RGB_LED_PIN));  // Toggle LED
    }
    
    // Minimal delay and yield for task scheduling
    safeDelay(10);
    yield();
}

// Process JSON payload from web client
// 웹 클라이언트의 JSON 페이로드 처리
void processWebClientJSON(const String& jsonStr) {
    if (!usbHidReady) {
        return;
    }
    
    // 강화된 JSON 검증 - 악성 입력 방지
    // Enhanced JSON validation - prevent malicious input
    if (jsonStr.length() < 2 || jsonStr.length() > 8192) {  // 최대 8KB 제한
        return;  // Invalid size
    }
    
    if (!jsonStr.startsWith("{") || !jsonStr.endsWith("}")) {
        return;  // Invalid JSON structure
    }
    
    // 중괄호 균형 검사 - JSON 구조 검증
    // Brace balance check - JSON structure validation
    int braceCount = 0;
    for (int i = 0; i < jsonStr.length(); i++) {
        if (jsonStr.charAt(i) == '{') braceCount++;
        if (jsonStr.charAt(i) == '}') braceCount--;
        if (braceCount < 0) return;  // 잘못된 구조
    }
    if (braceCount != 0) return;  // 불균형한 중괄호
    
    // 필수 텍스트 필드 확인
    // Check for required text field
    if (jsonStr.indexOf("\"text\":") == -1) {
        return;  // Missing required text field
    }
    
    // Parse JSON fields manually (lightweight parsing)
    // JSON 필드 수동 파싱 (경량 파싱)
    String text = "";
    String chunkId = "";
    uint16_t sequence = 0;
    String receivedChecksum = "";
    uint8_t speed_cps = typingSpeedCPS;  // Default to current speed
    uint32_t interval_ms = intervalMS;   // Default to current interval
    
    // 안전한 "text" 필드 추출
    // Safe "text" field extraction
    int textStart = jsonStr.indexOf("\"text\":");
    if (textStart != -1) {
        int valueStart = jsonStr.indexOf("\"", textStart + 7);
        if (valueStart != -1 && valueStart < jsonStr.length() - 1) {
            int valueEnd = jsonStr.indexOf("\"", valueStart + 1);
            if (valueEnd != -1 && valueEnd > valueStart) {
                // 텍스트 길이 제한 (최대 2KB)
                // Text length limit (max 2KB)
                int textLength = valueEnd - valueStart - 1;
                if (textLength > 0 && textLength <= 2048) {
                    text = jsonStr.substring(valueStart + 1, valueEnd);
                    
                    // 안전한 이스케이프 문자 처리
                    // Safe escape character handling
                    text.replace("\\n", "\n");
                    text.replace("\\t", "\t");
                    text.replace("\\\"", "\"");
                    text.replace("\\\\", "\\");
                    
                    // 제어 문자 제거 (0x00-0x1F 범위, 단 \n, \t 제외)
                    // Remove control characters (0x00-0x1F range, except \n, \t)
                    String filteredText = "";
                    for (int i = 0; i < text.length(); i++) {
                        char c = text.charAt(i);
                        if (c >= 0x20 || c == '\n' || c == '\t') {
                            filteredText += c;
                        }
                    }
                    text = filteredText;
                }
            }
        }
    }
    
    // 안전한 "speed_cps" 필드 추출
    // Safe "speed_cps" field extraction
    int speedStart = jsonStr.indexOf("\"speed_cps\":");
    if (speedStart != -1) {
        int valueStart = speedStart + 12;
        int valueEnd = jsonStr.indexOf(',', valueStart);
        if (valueEnd == -1) valueEnd = jsonStr.indexOf('}', valueStart);
        
        // 범위 검증
        // Range validation
        if (valueEnd != -1 && valueEnd > valueStart && (valueEnd - valueStart) <= 10) {
            String speedStr = jsonStr.substring(valueStart, valueEnd);
            speedStr.trim();
            
            // 숫자 문자만 허용 (음수 방지)
            // Allow only numeric characters (prevent negative)
            bool isValidNumber = true;
            for (int i = 0; i < speedStr.length(); i++) {
                if (!isDigit(speedStr.charAt(i))) {
                    isValidNumber = false;
                    break;
                }
            }
            
            if (isValidNumber && speedStr.length() > 0) {
                int newSpeed = speedStr.toInt();
                if (newSpeed >= 1 && newSpeed <= 50) {  // 안전한 범위
                    speed_cps = newSpeed;
                }
            }
        }
    }
    
    // Extract "interval_ms" field
    // "interval_ms" 필드 추출
    int intervalStart = jsonStr.indexOf("\"interval_ms\":");
    if (intervalStart != -1) {
        int valueStart = intervalStart + 14;
        int valueEnd = jsonStr.indexOf(',', valueStart);
        if (valueEnd == -1) valueEnd = jsonStr.indexOf('}', valueStart);
        if (valueEnd != -1) {
            String intervalStr = jsonStr.substring(valueStart, valueEnd);
            intervalStr.trim();
            int newInterval = intervalStr.toInt();
            if (newInterval >= 0 && newInterval <= 5000) {  // 0-5 seconds max
                interval_ms = newInterval;
            }
        }
    }
    
    // Extract chunk information - 청크 정보 추출
    // Extract "chunk_id" field
    int chunkIdStart = jsonStr.indexOf("\"chunk_id\":\"");
    if (chunkIdStart != -1) {
        int valueStart = chunkIdStart + 12;
        int valueEnd = jsonStr.indexOf("\"", valueStart);
        if (valueEnd != -1) {
            chunkId = jsonStr.substring(valueStart, valueEnd);
        }
    }
    
    // Extract "sequence" field
    int seqStart = jsonStr.indexOf("\"sequence\":");
    if (seqStart != -1) {
        int valueStart = seqStart + 11;
        int valueEnd = jsonStr.indexOf(',', valueStart);
        if (valueEnd == -1) valueEnd = jsonStr.indexOf('}', valueStart);
        if (valueEnd != -1) {
            String seqStr = jsonStr.substring(valueStart, valueEnd);
            seqStr.trim();
            sequence = seqStr.toInt();
        }
    }
    
    // Extract "checksum" field
    int checksumStart = jsonStr.indexOf("\"checksum\":\"");
    if (checksumStart != -1) {
        int valueStart = checksumStart + 12;
        int valueEnd = jsonStr.indexOf("\"", valueStart);
        if (valueEnd != -1) {
            receivedChecksum = jsonStr.substring(valueStart, valueEnd);
        }
    }
    
    // Chunk validation if chunk_id is present
    // 청크 ID가 있으면 검증 수행
    bool isChunkedData = (chunkId.length() > 0);
    bool validChunk = true;
    
    if (isChunkedData) {
        // Check for duplicate chunk
        // 중복 청크 확인
        if (isChunkDuplicate(chunkId)) {
            sendChunkAck(chunkId, true);  // Already processed, send ACK
            return;
        }
        
        // Verify checksum if provided
        // 체크섬 검증
        if (receivedChecksum.length() > 0 && text.length() > 0) {
            String calculatedChecksum = calculateChecksum(text);
            if (calculatedChecksum != receivedChecksum) {
                sendChunkAck(chunkId, false);  // Send NACK
                return;
            }
        }
        
        // Add to chunk history
        // 청크 히스토리에 추가
        addToChunkHistory(chunkId, sequence, receivedChecksum);
    }
    
    // Apply settings temporarily for this typing session
    // 이 타이핑 세션에 대해 설정 임시 적용
    uint8_t originalSpeedCPS = typingSpeedCPS;
    uint32_t originalDelay = baseTypingDelay;
    uint32_t originalIntervalMS = intervalMS;
    
    // Update typing parameters
    // 타이핑 매개변수 업데이트
    typingSpeedCPS = speed_cps;
    intervalMS = interval_ms;
    
    // Process the text with updated settings
    // 업데이트된 설정으로 텍스트 처리
    if (text.length() > 0) {
        Serial.print("Typing with speed: ");
        Serial.print(speed_cps);
        Serial.print(" cps, interval: ");
        Serial.print(interval_ms);
        Serial.println(" ms");
        
        // Auto-detect language and type accordingly
        // 언어 자동 감지 및 그에 따른 타이핑
        bool hasKorean = false;
        for (int i = 0; i < text.length(); i++) {
            if (text.charAt(i) > 127) {  // Non-ASCII characters (likely Korean)
                hasKorean = true;
                break;
            }
        }
        
        if (hasKorean) {
            // Korean text - convert to QWERTY keystrokes using proper jamo decomposition
            // 한국어 텍스트 - 올바른 자모 분해를 사용하여 QWERTY 키 입력으로 변환
            ensureKeyboardMode(MODE_KOREAN);
            String qwertyKeys = HangulQWERTY::hangulToQWERTY(text);
            typeWithSmartTiming(qwertyKeys);
        } else {
            // English text processing
            // 영어 텍스트 처리
            ensureKeyboardMode(MODE_ENGLISH);
            typeWithSmartTiming(text);
        }
        
        // Mark chunk as processed and send ACK
        // 청크를 처리 완료로 마킹하고 ACK 전송
        if (isChunkedData) {
            // Mark chunk as processed
            // 청크를 처리 완료로 마킹
            for (int i = 0; i < MAX_CHUNK_HISTORY; i++) {
                if (chunkHistory[i].id == chunkId) {
                    chunkHistory[i].processed = true;
                    break;
                }
            }
            
            // Send ACK for successful typing
            // 타이핑 성공에 대한 ACK 전송
            sendChunkAck(chunkId, true);
        } else {
            // Send legacy completion notification
            // 레거시 완료 알림 전송
            if (bleManager) {
                String response = "OK:" + String(text.length());
                bleManager->sendNotification(response.c_str());
            }
        }
    }
    
    // Restore original settings
    // 원래 설정 복원
    typingSpeedCPS = originalSpeedCPS;
    intervalMS = originalIntervalMS;
}

// Command queue management functions - 명령 큐 관리 함수
bool enqueueCommand(const std::string& command) {
    if (queueCount >= MAX_COMMAND_QUEUE) {
        return false;  // Queue full
    }
    
    commandQueue[queueTail] = command;
    queueTail = (queueTail + 1) % MAX_COMMAND_QUEUE;
    queueCount++;
    return true;
}

bool dequeueCommand(std::string& command) {
    if (queueCount == 0) {
        return false;  // Queue empty
    }
    
    command = commandQueue[queueHead];
    queueHead = (queueHead + 1) % MAX_COMMAND_QUEUE;
    queueCount--;
    return true;
}

void processCommandQueue() {
    // Process queued commands when not typing
    // 타이핑 중이 아닐 때 대기열의 명령 처리
    if (!isTyping && queueCount > 0) {
        std::string command;
        if (dequeueCommand(command)) {
            processReceivedData(command);
        }
    }
}

// Calculate simple checksum for text verification
// 텍스트 검증을 위한 간단한 체크섬 계산
String calculateChecksum(const String& text) {
    uint32_t hash = 0;
    for (int i = 0; i < text.length(); i++) {
        hash = ((hash << 5) - hash) + text.charAt(i);
    }
    return String(abs((int)hash), 16);
}

// Send ACK/NACK for received chunk
// 수신된 청크에 대한 ACK/NACK 전송
void sendChunkAck(const String& chunkId, bool success) {
    if (bleManager && bleManager->isConnected()) {
        String response = success ? "ACK:" : "NACK:";
        response += chunkId;
        bleManager->sendNotification(response.c_str());
    }
}

// Add chunk to processing history
// 청크를 처리 히스토리에 추가
void addToChunkHistory(const String& id, uint16_t seq, const String& checksum) {
    ChunkInfo& chunk = chunkHistory[chunkHistoryIndex];
    chunk.id = id;
    chunk.sequence = seq;
    chunk.checksum = checksum;
    chunk.receivedAt = millis();
    chunk.processed = false;
    
    chunkHistoryIndex = (chunkHistoryIndex + 1) % MAX_CHUNK_HISTORY;
}

// Check if chunk is duplicate
// 청크 중복 여부 확인
bool isChunkDuplicate(const String& id) {
    for (int i = 0; i < MAX_CHUNK_HISTORY; i++) {
        if (chunkHistory[i].id == id && chunkHistory[i].processed) {
            return true;
        }
    }
    return false;
}