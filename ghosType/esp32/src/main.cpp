// src/main.cpp
// GHOSTYPE Production Firmware - Smart Keyboard Mode Switching

#include <Arduino.h>
#include "BLEConfig.h"
#include "BLENimbleManager.h"

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

// Typing speed control (adjustable via web interface)
uint8_t typingSpeedCPS = 6;              // Characters per second (default: 6)
uint32_t baseTypingDelay = 167;          // Base delay between keystrokes (1000ms / 6 cps)

// Keyboard mode management for Korean/English switching
enum KeyboardMode {
    MODE_UNKNOWN = 0,    // Initial state - mode not determined
    MODE_ENGLISH = 1,    // English input mode
    MODE_KOREAN = 2      // Korean input mode (Hangul)
};

KeyboardMode currentKeyboardMode = MODE_UNKNOWN;  // Current active keyboard mode
unsigned long lastModeChange = 0;                 // Timestamp of last mode switch

// Protocol definitions for parsing incoming BLE data
#define PROTOCOL_PREFIX "GHTYPE_"
#define PROTOCOL_ENGLISH "GHTYPE_ENG:"     // English text input
#define PROTOCOL_KOREAN "GHTYPE_KOR:"      // Korean jamo key sequence
#define PROTOCOL_SPECIAL "GHTYPE_SPE:"     // Special commands (enter, ctrl+c, etc.)
#define PROTOCOL_CONFIG "GHTYPE_CFG:"      // Configuration changes (typing speed)

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

// Force keyboard mode switch using Alt+Shift combination (Korean IME standard)
void forceKeyboardMode(KeyboardMode mode) {
    if (!usbHidReady) return;  // Skip if USB keyboard not available
    
    // Send Alt+Shift to toggle Korean/English input mode
    Keyboard.press(KEY_LEFT_ALT);
    delay(50);                      // Brief hold for reliable detection
    Keyboard.press(KEY_LEFT_SHIFT);
    delay(50);                      // Brief hold for reliable detection
    Keyboard.releaseAll();
    delay(300);                     // Wait for OS to process mode change
    
    currentKeyboardMode = mode;
    lastModeChange = millis();
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
void processReceivedData(const std::string& data) {
    String dataStr = String(data.c_str());
    
    // Parse protocol prefix and route to appropriate processor
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
        processEnglishText(dataStr);
    }
    
    // Update message counter for statistics
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

// Process Korean jamo key sequence with automatic mode switching
void processKoreanJamo(const String& jamoKeys) {
    if (!usbHidReady || jamoKeys.length() == 0) {
        return;  // Skip if USB keyboard unavailable or empty sequence
    }
    
    try {
        // Ensure keyboard is in Korean mode before typing jamo keys
        ensureKeyboardMode(MODE_KOREAN);
        
        // Type the jamo key sequence (Korean characters broken down to QWERTY keys)
        typeWithSmartTiming(jamoKeys);
        
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
        forceKeyboardMode(MODE_KOREAN);  // Force switch to Korean mode
    }
    else if (cmd == "eng") {
        forceKeyboardMode(MODE_ENGLISH); // Force switch to English mode
    }
    else if (cmd == "reset_mode") {
        currentKeyboardMode = MODE_UNKNOWN;  // Reset mode detection
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
    baseTypingDelay = 1000 / typingSpeedCPS;  // Convert CPS to milliseconds delay
    
    // Send confirmation back to web interface via BLE
    if (bleManager) {
        String response = String("Speed updated: ") + String(typingSpeedCPS) + " cps";
        bleManager->sendNotification(response.c_str());
    }
}

// Type text with natural human-like timing variations
void typeWithSmartTiming(const String& text) {
    // Calculate random timing variation (30% of base delay)
    uint32_t randomRange = baseTypingDelay / 3;
    
    for (int i = 0; i < text.length(); i++) {
        char c = text.charAt(i);
        
        // Handle special characters with fixed delays
        if (c == '\n') {
            Keyboard.press(KEY_RETURN);
            Keyboard.releaseAll();
            delay(100);  // Fixed delay for special keys
        } else if (c == '\t') {
            Keyboard.press(KEY_TAB);
            Keyboard.releaseAll();
            delay(100);  // Fixed delay for special keys
        } else if (c >= 'A' && c <= 'Z') {
            // Handle uppercase letters with Shift modifier
            Keyboard.press(KEY_LEFT_SHIFT);
            delay(20);   // Brief hold for shift detection
            Keyboard.press(c);
            delay(30);   // Brief hold for key detection
            Keyboard.releaseAll();
            // Variable delay based on typing speed + randomization
            delay(baseTypingDelay + random(randomRange));
        } else {
            // Regular character typing
            Keyboard.write(c);
            // Variable delay based on typing speed + randomization for natural feel
            delay(baseTypingDelay + random(randomRange));
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
        
        // Test keyboard functionality with space character
        Keyboard.write(' ');
        safeDelay(100);
        
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
    // Process any incoming serial commands for debugging
    handleSerialCommands();
    
    // Process BLE data if system is ready and connected
    if (systemReady && bleManager) {
        try {
            // Check for incoming BLE data
            if (bleManager->hasReceivedData()) {
                std::string receivedData = bleManager->getReceivedData();
                if (!receivedData.empty()) {
                    processReceivedData(receivedData);  // Parse and execute command
                }
            }
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