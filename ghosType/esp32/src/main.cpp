// main.cpp - GHOSTYPE Production Firmware
// Clean implementation without any caching issues

#include <Arduino.h>
#include "BLEConfig.h"
#include "BLENimbleManager.h"
#include <USB.h>
#include <USBHIDKeyboard.h>

// Global objects
BLENimbleManager* bleManager = nullptr;
USBHIDKeyboard Keyboard;

// System state
unsigned long lastStatusUpdate = 0;
unsigned long lastHeartbeat = 0;
bool systemReady = false;
bool usbHidReady = false;
uint8_t errorCount = 0;

// Typing speed control
uint8_t typingSpeedCPS = 6;
uint32_t baseTypingDelay = 167;

// Keyboard mode management
enum KeyboardMode {
    MODE_UNKNOWN = 0,
    MODE_ENGLISH = 1,
    MODE_KOREAN = 2
};

KeyboardMode currentKeyboardMode = MODE_UNKNOWN;
unsigned long lastModeChange = 0;

// Protocol constants - using simple string literals to avoid any macro issues
static const char* PROTOCOL_ENG = "GHTYPE_ENG:";
static const char* PROTOCOL_KOR = "GHTYPE_KOR:";
static const char* PROTOCOL_SPE = "GHTYPE_SPE:";
static const char* PROTOCOL_CFG = "GHTYPE_CFG:";

// Function declarations
void safeDelay(uint32_t ms);
void ensureKeyboardMode(KeyboardMode targetMode);
void forceKeyboardMode(KeyboardMode mode);
void processReceivedData(const std::string& data);
void processEnglishText(const String& text);
void processKoreanJamo(const String& jamoKeys);
void processSpecialCommand(const String& command);
void processConfiguration(const String& config);
void typeWithSmartTiming(const String& text);
void updateTypingSpeed(uint8_t newSpeedCPS);
bool initializeHardware();
void resetSystem();

// Safe delay with yield
void safeDelay(uint32_t ms) {
    uint32_t start = millis();
    while (millis() - start < ms) {
        yield();
        delay(1);
    }
}

// Ensure correct keyboard mode before typing
void ensureKeyboardMode(KeyboardMode targetMode) {
    if (currentKeyboardMode == targetMode) {
        return; // Already in target mode
    }
    
    // Rate limiting - minimum 200ms between mode changes
    if (millis() - lastModeChange < 200) {
        delay(200 - (millis() - lastModeChange));
    }
    
    forceKeyboardMode(targetMode);
}

// Force keyboard mode switch using Alt+Shift
void forceKeyboardMode(KeyboardMode mode) {
    if (!usbHidReady) return;
    
    // Send Alt+Shift to toggle Korean/English mode
    Keyboard.press(KEY_LEFT_ALT);
    delay(50);
    Keyboard.press(KEY_LEFT_SHIFT);
    delay(50);
    Keyboard.releaseAll();
    delay(300); // Wait for mode change
    
    currentKeyboardMode = mode;
    lastModeChange = millis();
}

// Main data processing function
void processReceivedData(const std::string& data) {
    String dataStr = String(data.c_str());
    
    // Parse protocol and route to appropriate handler
    if (dataStr.startsWith(PROTOCOL_ENG)) {
        String englishText = dataStr.substring(strlen(PROTOCOL_ENG));
        processEnglishText(englishText);
    } 
    else if (dataStr.startsWith(PROTOCOL_KOR)) {
        String jamoKeys = dataStr.substring(strlen(PROTOCOL_KOR));
        processKoreanJamo(jamoKeys);
    } 
    else if (dataStr.startsWith(PROTOCOL_SPE)) {
        String specialCmd = dataStr.substring(strlen(PROTOCOL_SPE));
        processSpecialCommand(specialCmd);
    } 
    else if (dataStr.startsWith(PROTOCOL_CFG)) {
        String configData = dataStr.substring(strlen(PROTOCOL_CFG));
        processConfiguration(configData);
    } 
    else {
        // No protocol - treat as plain English text
        processEnglishText(dataStr);
    }
}

// Process English text input
void processEnglishText(const String& text) {
    if (!usbHidReady || text.length() == 0) return;
    
    try {
        ensureKeyboardMode(MODE_ENGLISH);
        typeWithSmartTiming(text);
    } catch (...) {
        errorCount++;
    }
}

// Process Korean jamo key sequence
void processKoreanJamo(const String& jamoKeys) {
    if (!usbHidReady || jamoKeys.length() == 0) return;
    
    try {
        ensureKeyboardMode(MODE_KOREAN);
        typeWithSmartTiming(jamoKeys);
    } catch (...) {
        errorCount++;
    }
}

// Process special commands
void processSpecialCommand(const String& command) {
    if (!usbHidReady) return;
    
    String cmd = command;
    cmd.toLowerCase();
    
    // Execute specific commands
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
        forceKeyboardMode(MODE_KOREAN);
    }
    else if (cmd == "eng") {
        forceKeyboardMode(MODE_ENGLISH);
    }
    else if (cmd == "reset_mode") {
        currentKeyboardMode = MODE_UNKNOWN;
    }
    
    delay(50);
}

// Process configuration changes
void processConfiguration(const String& config) {
    // Simple JSON parsing for speed_cps
    int speedIndex = config.indexOf("\"speed_cps\":");
    if (speedIndex != -1) {
        int valueStart = speedIndex + 12;
        int valueEnd = config.indexOf(',', valueStart);
        if (valueEnd == -1) valueEnd = config.indexOf('}', valueStart);
        
        if (valueEnd != -1) {
            String speedStr = config.substring(valueStart, valueEnd);
            speedStr.trim();
            int newSpeed = speedStr.toInt();
            
            if (newSpeed >= 1 && newSpeed <= 20) {
                updateTypingSpeed(newSpeed);
            }
        }
    }
}

// Update typing speed
void updateTypingSpeed(uint8_t newSpeedCPS) {
    typingSpeedCPS = newSpeedCPS;
    baseTypingDelay = 1000 / typingSpeedCPS;
    
    // Send confirmation via BLE
    if (bleManager) {
        String response = String("Speed updated: ") + String(typingSpeedCPS) + " cps";
        bleManager->sendData(response.c_str());
    }
}

// Type text with natural timing
void typeWithSmartTiming(const String& text) {
    uint32_t randomRange = baseTypingDelay / 3; // 30% variation
    
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
            // Uppercase with Shift
            Keyboard.press(KEY_LEFT_SHIFT);
            delay(20);
            Keyboard.press(c);
            delay(30);
            Keyboard.releaseAll();
            delay(baseTypingDelay + random(randomRange));
        } else {
            // Regular character
            Keyboard.write(c);
            delay(baseTypingDelay + random(randomRange));
        }
    }
}

// Initialize hardware
bool initializeHardware() {
    try {
        // Initialize LED pin (always safe)
        pinMode(RGB_LED_PIN, OUTPUT);
        digitalWrite(RGB_LED_PIN, LOW);
        yield(); // Feed watchdog
        
        // Try USB initialization with timeout
        unsigned long usbStartTime = millis();
        USB.begin();
        
        // Wait for USB with timeout (max 2 seconds)
        while (millis() - usbStartTime < 2000) {
            delay(100);
            yield(); // Feed watchdog
        }
        
        // Try keyboard initialization
        Keyboard.begin();
        delay(200);
        yield(); // Feed watchdog
        
        // Simple test without hanging
        usbHidReady = true;
        
        // Try initial mode setting (non-blocking)
        currentKeyboardMode = MODE_ENGLISH;
        
        return true;
        
    } catch (...) {
        // On any error, disable USB HID but continue
        usbHidReady = false;
        return false; // Indicate hardware init failed
    }
}

// Reset system
void resetSystem() {
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

// Setup function
void setup() {
    // Basic initialization with watchdog feeding
    Serial.begin(115200);
    delay(100);
    yield(); // Feed watchdog
    
    // Initialize hardware with error handling
    if (!initializeHardware()) {
        // Don't hang in infinite loop - just disable USB HID
        usbHidReady = false;
    }
    
    delay(500);
    yield(); // Feed watchdog
    
    // Initialize BLE with timeout protection
    try {
        bleManager = new BLENimbleManager();
        yield(); // Feed watchdog
        
        if (bleManager) {
            // Try BLE initialization with timeout
            unsigned long bleStartTime = millis();
            bool bleInitSuccess = false;
            
            // Attempt BLE init with 5 second timeout
            while (millis() - bleStartTime < 5000) {
                if (bleManager->begin()) {
                    bleInitSuccess = true;
                    break;
                }
                delay(100);
                yield(); // Feed watchdog
            }
            
            systemReady = bleInitSuccess;
        }
    } catch (...) {
        systemReady = false;
        // Clean up on failure
        if (bleManager) {
            delete bleManager;
            bleManager = nullptr;
        }
    }
    
    // Initialize timestamps
    lastStatusUpdate = millis();
    lastHeartbeat = millis();
    
    // Signal successful startup
    if (systemReady) {
        // Blink LED to indicate successful startup
        for (int i = 0; i < 3; i++) {
            digitalWrite(RGB_LED_PIN, HIGH);
            delay(100);
            digitalWrite(RGB_LED_PIN, LOW);
            delay(100);
        }
    }
}

// Main loop
void loop() {
    // Essential: Feed watchdog first
    yield();
    
    // Process BLE data with error protection
    if (systemReady && bleManager) {
        try {
            if (bleManager->hasReceivedData()) {
                std::string receivedData = bleManager->getReceivedData();
                if (!receivedData.empty()) {
                    processReceivedData(receivedData);
                }
            }
        } catch (...) {
            errorCount++;
            // More conservative error handling
            if (errorCount > 10) {
                // Try graceful recovery first
                if (bleManager) {
                    delete bleManager;
                    bleManager = nullptr;
                }
                systemReady = false;
                errorCount = 0; // Reset counter
                delay(1000); // Brief pause before continuing
            }
        }
    }
    
    // Periodic status update (every 30 seconds)
    if (millis() - lastStatusUpdate > 30000) {
        lastStatusUpdate = millis();
        // Optional: Try to restart BLE if it failed
        if (!systemReady && !bleManager) {
            // Attempt BLE restart (simplified)
            try {
                bleManager = new BLENimbleManager();
                if (bleManager && bleManager->begin()) {
                    systemReady = true;
                    errorCount = 0;
                }
            } catch (...) {
                if (bleManager) {
                    delete bleManager;
                    bleManager = nullptr;
                }
            }
        }
    }
    
    // LED heartbeat (every 5 seconds)
    if (millis() - lastHeartbeat > 5000) {
        lastHeartbeat = millis();
        digitalWrite(RGB_LED_PIN, !digitalRead(RGB_LED_PIN));
    }
    
    // Small delay with watchdog feeding
    delay(10);
    yield();
}