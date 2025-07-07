// src/main.cpp
// GHOSTYPE 상품화 버전 - 스마트 키보드 모드 전환

#include <Arduino.h>
#include "BLEConfig.h"
#include "BLENimbleManager.h"

// ESP32 system headers (Arduino framework compatible)
#ifdef ESP32
  // Use Arduino ESP32 equivalents instead of ESP-IDF headers
  // esp_system.h functions are available through Arduino.h
  // esp_chip_info.h functions are available through ESP.h
#endif

// USB HID 키보드
#include <USB.h>
#include <USBHIDKeyboard.h>

// 전역 객체
BLENimbleManager* bleManager = nullptr;
USBHIDKeyboard Keyboard;

// 상태 변수
unsigned long lastStatusUpdate = 0;
unsigned long lastHeartbeat = 0;
bool systemReady = false;
bool usbHidReady = false;
uint8_t errorCount = 0;

// ===== 키보드 모드 관리 =====
enum KeyboardMode {
    MODE_UNKNOWN = 0,    // 알 수 없음 (초기 상태)
    MODE_ENGLISH = 1,    // 영문 모드
    MODE_KOREAN = 2      // 한글 모드
};

KeyboardMode currentKeyboardMode = MODE_UNKNOWN;  // 현재 키보드 모드
unsigned long lastModeChange = 0;                 // 마지막 모드 변경 시간

// ===== 유니크한 프로토콜 정의 =====
#define PROTOCOL_PREFIX "GHTYPE_"
#define PROTOCOL_ENGLISH "GHTYPE_ENG:"
#define PROTOCOL_KOREAN "GHTYPE_KOR:"
#define PROTOCOL_SPECIAL "GHTYPE_SPE:"

// 함수 선언
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
void typeWithSmartTiming(const String& text);
void handleSerialCommands();
void printHelp();
void resetSystem();

// ===== 안전한 지연 함수 =====
void safeDelay(uint32_t ms) {
    uint32_t start = millis();
    while (millis() - start < ms) {
        yield();
        delay(1);
    }
}

// ===== 키보드 모드 확인 및 전환 =====
void ensureKeyboardMode(KeyboardMode targetMode) {
    if (currentKeyboardMode == targetMode) {
        // 이미 원하는 모드면 스킵
        return;
    }
    
    // 모드 변경이 너무 자주 일어나지 않도록 제한
    if (millis() - lastModeChange < 200) {
        delay(200 - (millis() - lastModeChange));
    }
    
    forceKeyboardMode(targetMode);
}

// ===== 강제 키보드 모드 전환 =====
void forceKeyboardMode(KeyboardMode mode) {
    if (!usbHidReady) return;
    
    String modeStr = getKeyboardModeString(mode);
    Serial.printf("🔄 키보드 모드 전환: %s → %s\n", 
                  getKeyboardModeString(currentKeyboardMode).c_str(),
                  modeStr.c_str());
    
    // Alt + Shift (한영키)
    Keyboard.press(KEY_LEFT_ALT);
    delay(50);
    Keyboard.press(KEY_LEFT_SHIFT);
    delay(50);
    Keyboard.releaseAll();
    delay(300);  // 모드 전환 완료 대기
    
    currentKeyboardMode = mode;
    lastModeChange = millis();
    
    Serial.printf("✅ %s 모드 활성화\n", modeStr.c_str());
}

// ===== 키보드 모드 문자열 반환 =====
String getKeyboardModeString(KeyboardMode mode) {
    switch (mode) {
        case MODE_ENGLISH: return "영문";
        case MODE_KOREAN:  return "한글";
        case MODE_UNKNOWN: return "알수없음";
        default:           return "오류";
    }
}

// ===== 수신 데이터 처리 (프로토콜 기반) =====
void processReceivedData(const std::string& data) {
    Serial.printf("\n🎯 데이터 수신: \"%s\"\n", data.c_str());
    
    String dataStr = String(data.c_str());
    
    // 프로토콜 분석
    if (dataStr.startsWith(PROTOCOL_ENGLISH)) {
        // 영문 텍스트 처리
        String englishText = dataStr.substring(strlen(PROTOCOL_ENGLISH));
        Serial.printf("🔤 영문 텍스트: \"%s\"\n", englishText.c_str());
        processEnglishText(englishText);
        
    } else if (dataStr.startsWith(PROTOCOL_KOREAN)) {
        // 한글 자모 키 처리
        String jamoKeys = dataStr.substring(strlen(PROTOCOL_KOREAN));
        Serial.printf("🇰🇷 한글 자모: \"%s\"\n", jamoKeys.c_str());
        processKoreanJamo(jamoKeys);
        
    } else if (dataStr.startsWith(PROTOCOL_SPECIAL)) {
        // 특수 명령 처리
        String specialCmd = dataStr.substring(strlen(PROTOCOL_SPECIAL));
        Serial.printf("🎹 특수 명령: \"%s\"\n", specialCmd.c_str());
        processSpecialCommand(specialCmd);
        
    } else {
        // 프로토콜 없으면 기본 영문으로 처리
        Serial.printf("📝 기본 영문: \"%s\"\n", dataStr.c_str());
        processEnglishText(dataStr);
    }
    
    // 통계 업데이트
    static uint32_t totalMessages = 0;
    totalMessages++;
    Serial.printf("📊 총 처리 메시지: %d개\n", totalMessages);
}

// ===== 영문 텍스트 처리 =====
void processEnglishText(const String& text) {
    if (!usbHidReady || text.length() == 0) {
        Serial.printf("→ [시리얼 모드] %s\n", text.c_str());
        return;
    }
    
    try {
        // 영문 모드 확인 및 전환
        ensureKeyboardMode(MODE_ENGLISH);
        
        // 텍스트 입력
        Serial.println("⌨️ 영문 텍스트 입력 중...");
        typeWithSmartTiming(text);
        
        Serial.printf("✅ 영문 입력 완료: \"%s\"\n", text.c_str());
        
    } catch (...) {
        Serial.println("❌ 영문 텍스트 입력 실패");
    }
}

// ===== 한글 자모 키 처리 =====
void processKoreanJamo(const String& jamoKeys) {
    if (!usbHidReady || jamoKeys.length() == 0) {
        Serial.printf("→ [시리얼 모드] 한글: %s\n", jamoKeys.c_str());
        return;
    }
    
    try {
        // 한글 모드 확인 및 전환
        ensureKeyboardMode(MODE_KOREAN);
        
        // 자모 키 입력
        Serial.println("⌨️ 한글 자모 키 입력 중...");
        typeWithSmartTiming(jamoKeys);
        
        Serial.printf("✅ 한글 입력 완료: 자모 \"%s\"\n", jamoKeys.c_str());
        
    } catch (...) {
        Serial.println("❌ 한글 자모 키 입력 실패");
    }
}

// ===== 특수 명령 처리 =====
void processSpecialCommand(const String& command) {
    if (!usbHidReady) {
        Serial.println("❌ USB HID 비활성화 - 특수 명령 사용 불가");
        return;
    }
    
    String cmd = command;
    cmd.toLowerCase();
    
    Serial.printf("🎹 특수 명령 실행: %s\n", cmd.c_str());
    
    if (cmd == "enter") {
        Keyboard.press(KEY_RETURN);
        Keyboard.releaseAll();
        Serial.println("⌨️ Enter 키 전송");
    }
    else if (cmd == "tab") {
        Keyboard.press(KEY_TAB);
        Keyboard.releaseAll();
        Serial.println("⌨️ Tab 키 전송");
    }
    else if (cmd == "backspace") {
        Keyboard.press(KEY_BACKSPACE);
        Keyboard.releaseAll();
        Serial.println("⌨️ Backspace 키 전송");
    }
    else if (cmd == "space") {
        Keyboard.write(' ');
        Serial.println("⌨️ Space 키 전송");
    }
    else if (cmd == "ctrl+c") {
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press('c');
        Keyboard.releaseAll();
        Serial.println("⌨️ Ctrl+C 전송");
    }
    else if (cmd == "ctrl+v") {
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press('v');
        Keyboard.releaseAll();
        Serial.println("⌨️ Ctrl+V 전송");
    }
    else if (cmd == "alt+tab") {
        Keyboard.press(KEY_LEFT_ALT);
        Keyboard.press(KEY_TAB);
        Keyboard.releaseAll();
        Serial.println("⌨️ Alt+Tab 전송");
    }
    else if (cmd == "haneng") {
        forceKeyboardMode(MODE_KOREAN);
    }
    else if (cmd == "eng") {
        forceKeyboardMode(MODE_ENGLISH);
    }
    else if (cmd == "reset_mode") {
        currentKeyboardMode = MODE_UNKNOWN;
        Serial.println("🔄 키보드 모드 초기화");
    }
    else {
        Serial.printf("❓ 알 수 없는 특수 명령: %s\n", cmd.c_str());
    }
    
    delay(50);
}

// ===== 스마트 타이핑 (자연스러운 속도) =====
void typeWithSmartTiming(const String& text) {
    Serial.printf("⌨️ 스마트 타이핑: \"%s\" (%d 문자)\n", text.c_str(), text.length());
    
    for (int i = 0; i < text.length(); i++) {
        char c = text.charAt(i);
        
        if (c == '\n') {
            Keyboard.press(KEY_RETURN);
            Keyboard.releaseAll();
            delay(100);
        } else if (c == '\t') {
            Keyboard.press(KEY_TAB);
            Keyboard.releaseAll();
            delay(100);
        } else if (c >= 'A' && c <= 'Z') {
            // Shift + 문자
            Keyboard.press(KEY_LEFT_SHIFT);
            delay(20);
            Keyboard.press(c);
            delay(30);
            Keyboard.releaseAll();
            delay(60 + random(40));  // 60-100ms 랜덤
        } else {
            Keyboard.write(c);
            delay(70 + random(50));  // 70-120ms 랜덤
        }
        
        // 긴 텍스트 진행률 표시
        if (text.length() > 30 && i > 0 && i % 15 == 0) {
            Serial.printf("📝 진행: %d/%d (%.1f%%)\n", 
                          i, text.length(), 
                          (float)i * 100.0 / text.length());
        }
    }
    
    Serial.println("✅ 타이핑 완료");
}

// ===== 시스템 초기화 =====
void initializeSystem() {
    Serial.begin(115200);
    safeDelay(1000);
    
    Serial.println("\n\n");
    Serial.println("╔══════════════════════════════════════════════╗");
    Serial.println("║        GHOSTYPE Professional v2.1             ║");
    Serial.println("║       스마트 키보드 모드 전환                 ║");
    Serial.println("║            T-Dongle-S3 Edition               ║");
    Serial.println("╚══════════════════════════════════════════════╝");
    
    Serial.println("\n📊 시스템 정보:");
    Serial.printf("   펌웨어: %s\n", PRODUCT_VERSION);
    Serial.printf("   칩: ESP32-S3\n");
    Serial.printf("   CPU: %d MHz\n", getCpuFrequencyMhz());
    Serial.printf("   메모리: %d KB 사용 가능\n", ESP.getFreeHeap() / 1024);
    
    Serial.println("\n🔧 프로토콜 정보:");
    Serial.println("   영문: " PROTOCOL_ENGLISH "[텍스트]");
    Serial.println("   한글: " PROTOCOL_KOREAN "[자모키]");
    Serial.println("   특수: " PROTOCOL_SPECIAL "[명령]");
    Serial.println("   🎯 스마트 키보드 모드 자동 전환");
    Serial.println("   ⌨️ 자연스러운 타이핑 속도");
}

// ===== 하드웨어 초기화 =====
bool initializeHardware() {
    Serial.println("\n🔧 하드웨어 초기화 중...");
    
    try {
        pinMode(RGB_LED_PIN, OUTPUT);
        digitalWrite(RGB_LED_PIN, LOW);
        Serial.println("✅ RGB LED 핀 초기화 완료");
        
        Serial.println("⌨️ USB HID 키보드 초기화 중...");
        
        USB.begin();
        safeDelay(1000);
        
        Keyboard.begin();
        safeDelay(500);
        
        Serial.println("💡 USB HID 키보드 테스트 중...");
        safeDelay(1000);
        
        Keyboard.write(' ');
        safeDelay(100);
        
        usbHidReady = true;
        Serial.println("✅ USB HID 키보드 초기화 완료!");
        Serial.println("🎯 스마트 키보드 모드 전환 준비");
        Serial.println("⚠️ 메모장이나 텍스트 에디터를 열어두세요!");
        
        // 초기 모드를 영문으로 설정
        Serial.println("🔄 초기 키보드 모드를 영문으로 설정...");
        forceKeyboardMode(MODE_ENGLISH);
        
        return true;
        
    } catch (...) {
        Serial.println("❌ USB HID 초기화 실패 - 시리얼 모드로 동작");
        usbHidReady = false;
        return true;
    }
}

// ===== 시리얼 명령 처리 =====
void handleSerialCommands() {
    if (!Serial.available()) return;
    
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toLowerCase();
    
    if (command.length() == 0) return;
    
    Serial.printf("\n⌨️ 명령: %s\n", command.c_str());
    
    if (command == "status" || command == "s") {
        if (bleManager) {
            bleManager->printStatus();
        }
        Serial.printf("⌨️ USB HID: %s\n", usbHidReady ? "활성화" : "비활성화");
        Serial.printf("🎯 현재 키보드 모드: %s\n", getKeyboardModeString(currentKeyboardMode).c_str());
    }
    else if (command == "help" || command == "h" || command == "?") {
        printHelp();
    }
    else if (command == "test") {
        if (usbHidReady) {
            Serial.println("⌨️ 영문 테스트 중...");
            processEnglishText("GHOSTYPE Test!");
        }
    }
    else if (command == "testko") {
        if (usbHidReady) {
            Serial.println("🇰🇷 한글 테스트 중...");
            // "안녕" = ㅇㅏㄴㄴㅕㅇ = dkssud
            processKoreanJamo("dkssud");
        }
    }
    else if (command == "eng") {
        forceKeyboardMode(MODE_ENGLISH);
    }
    else if (command == "kor") {
        forceKeyboardMode(MODE_KOREAN);
    }
    else if (command == "mode") {
        Serial.printf("🎯 현재 키보드 모드: %s\n", getKeyboardModeString(currentKeyboardMode).c_str());
    }
    else if (command == "reset" || command == "r") {
        resetSystem();
    }
    else if (command.startsWith("eng:")) {
        String text = command.substring(4);
        processEnglishText(text);
    }
    else if (command.startsWith("kor:")) {
        String jamo = command.substring(4);
        processKoreanJamo(jamo);
    }
    else if (command.startsWith("spe:")) {
        String special = command.substring(4);
        processSpecialCommand(special);
    }
    else {
        Serial.println("❓ 알 수 없는 명령. 'help' 입력하여 도움말 확인");
    }
}

// ===== 도움말 출력 =====
void printHelp() {
    Serial.println("\n📚 사용 가능한 명령:");
    Serial.println("┌─────────────┬──────────────────────────────┐");
    Serial.println("│ 명령        │ 설명                         │");
    Serial.println("├─────────────┼──────────────────────────────┤");
    Serial.println("│ status (s)  │ 상태 정보                    │");
    Serial.println("│ test        │ 영문 키보드 테스트           │");
    Serial.println("│ testko      │ 한글 키보드 테스트           │");
    Serial.println("│ eng         │ 영문 모드로 강제 전환        │");
    Serial.println("│ kor         │ 한글 모드로 강제 전환        │");
    Serial.println("│ mode        │ 현재 키보드 모드 확인        │");
    Serial.println("│ eng:[text]  │ 영문 텍스트 직접 입력        │");
    Serial.println("│ kor:[jamo]  │ 한글 자모 키 직접 입력       │");
    Serial.println("│ spe:[cmd]   │ 특수 명령 직접 실행          │");
    Serial.println("│ reset (r)   │ 시스템 재시작                │");
    Serial.println("│ help (h,?)  │ 이 도움말                    │");
    Serial.println("└─────────────┴──────────────────────────────┘");
    
    Serial.println("\n💡 프로토콜 사용법:");
    Serial.println("   🔤 영문: " PROTOCOL_ENGLISH "Hello World");
    Serial.println("   🇰🇷 한글: " PROTOCOL_KOREAN "dkssud");
    Serial.println("   🎹 특수: " PROTOCOL_SPECIAL "enter");
    
    Serial.println("\n🎯 특수 명령어:");
    Serial.println("   enter, tab, backspace, space");
    Serial.println("   ctrl+c, ctrl+v, alt+tab");
    Serial.println("   haneng, eng, reset_mode");
    
    Serial.printf("\n📊 현재 키보드 모드: %s\n", getKeyboardModeString(currentKeyboardMode).c_str());
}

// ===== 시스템 재시작 =====
void resetSystem() {
    Serial.println("🔄 시스템 재시작 중...");
    
    if (usbHidReady) {
        Keyboard.end();
    }
    
    if (bleManager) {
        bleManager->stop();
        delete bleManager;
        bleManager = nullptr;
    }
    
    safeDelay(1000);
    ESP.restart();
}

// ===== 메인 설정 =====
void setup() {
    initializeSystem();
    
    if (!initializeHardware()) {
        Serial.println("❌ 하드웨어 초기화 실패!");
        while (1) {
            safeDelay(1000);
        }
    }
    
    safeDelay(2000);
    
    Serial.println("🚀 BLE 시스템 초기화 중...");
    
    try {
        bleManager = new BLENimbleManager();
        
        if (bleManager && bleManager->begin()) {
            systemReady = true;
            Serial.println("✅ BLE 초기화 성공!");
        } else {
            Serial.println("❌ BLE 초기화 실패");
        }
    } catch (...) {
        Serial.println("❌ BLE 매니저 생성 중 예외 발생");
    }
    
    if (systemReady) {
        Serial.println("\n✅ 시스템 준비 완료!");
        Serial.println("📱 사용 방법:");
        if (bleManager) {
            Serial.printf("1. '%s' 검색 및 연결\n", bleManager->getDeviceName().c_str());
        }
        Serial.println("2. 메모장 열어두기");
        Serial.println("3. 웹에서 프로토콜 형식으로 전송:");
        Serial.println("   - 영문: " PROTOCOL_ENGLISH "Hello");
        Serial.println("   - 한글: " PROTOCOL_KOREAN "dkssud");
        Serial.println("   - 특수: " PROTOCOL_SPECIAL "enter");
        Serial.println("4. 자동 키보드 모드 전환으로 완벽 입력!");
        Serial.println("════════════════════════════════════════\n");
    }
    
    lastStatusUpdate = millis();
    lastHeartbeat = millis();
}

// ===== 메인 루프 =====
void loop() {
    handleSerialCommands();
    
    if (systemReady && bleManager) {
        try {
            if (bleManager->hasReceivedData()) {
                std::string receivedData = bleManager->getReceivedData();
                if (!receivedData.empty()) {
                    processReceivedData(receivedData);
                }
            }
        } catch (...) {
            Serial.println("❌ BLE 데이터 처리 중 예외 발생");
            errorCount++;
            if (errorCount > 5) {
                resetSystem();
            }
        }
    }
    
    if (millis() - lastStatusUpdate > 30000) {
        lastStatusUpdate = millis();
        
        if (systemReady && bleManager) {
            if (bleManager->isAnyDeviceConnected()) {
                Serial.printf("[연결됨] 💾 %dKB | 🎯 %s 모드\n", 
                              ESP.getFreeHeap() / 1024,
                              getKeyboardModeString(currentKeyboardMode).c_str());
            } else {
                Serial.printf("[대기중] 💾 %dKB | 📡 광고 중...\n", 
                              ESP.getFreeHeap() / 1024);
            }
        }
    }
    
    if (millis() - lastHeartbeat > 5000) {
        lastHeartbeat = millis();
        digitalWrite(RGB_LED_PIN, !digitalRead(RGB_LED_PIN));
    }
    
    safeDelay(10);
    yield();
}