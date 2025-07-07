// main.cpp - GHOSTYPE Production Firmware
// Minimal implementation for stability

#include <Arduino.h>
#include "BLEConfig.h"
#include "BLENimbleManager.h"
#include <USB.h>
#include <USBHIDKeyboard.h>

// Global objects
BLENimbleManager* bleManager = nullptr;
USBHIDKeyboard Keyboard;

// System state
bool systemReady = false;
bool usbHidReady = false;
bool bleReady = false;
uint8_t errorCount = 0;

// Timing
unsigned long lastHeartbeat = 0;
unsigned long lastBleCheck = 0;

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

// Protocol constants
static const char* PROTOCOL_ENG = "GHTYPE_ENG:";
static const char* PROTOCOL_KOR = "GHTYPE_KOR:";
static const char* PROTOCOL_SPE = "GHTYPE_SPE:";
static const char* PROTOCOL_CFG = "GHTYPE_CFG:";

// Function declarations
void processReceivedData(const std::string& data);
void processEnglishText(const String& text);
void processKoreanJamo(const String& jamoKeys);
void processSpecialCommand(const String& command);
void typeWithSmartTiming(const String& text);
void ensureKeyboardMode(KeyboardMode targetMode);
void forceKeyboardMode(KeyboardMode mode);

// Safe keyboard mode switching
void ensureKeyboardMode(KeyboardMode targetMode) {
    if (currentKeyboardMode == targetMode) return;
    if (millis() - lastModeChange < 200) return; // Rate limit
    
    forceKeyboardMode(targetMode);
}

// Force keyboard mode switch
void forceKeyboardMode(KeyboardMode mode) {
    if (!usbHidReady) return;
    
    Keyboard.press(KEY_LEFT_ALT);
    delay(50);
    Keyboard.press(KEY_LEFT_SHIFT);
    delay(50);
    Keyboard.releaseAll();
    delay(300);
    
    currentKeyboardMode = mode;
    lastModeChange = millis();
}

// Data processing
void processReceivedData(const std::string& data) {
    String dataStr = String(data.c_str());
    
    if (dataStr.startsWith(PROTOCOL_ENG)) {
        String text = dataStr.substring(strlen(PROTOCOL_ENG));
        processEnglishText(text);
    } 
    else if (dataStr.startsWith(PROTOCOL_KOR)) {
        String jamo = dataStr.substring(strlen(PROTOCOL_KOR));
        processKoreanJamo(jamo);
    } 
    else if (dataStr.startsWith(PROTOCOL_SPE)) {
        String cmd = dataStr.substring(strlen(PROTOCOL_SPE));
        processSpecialCommand(cmd);
    }
    else {
        processEnglishText(dataStr);
    }
}

// Process English text
void processEnglishText(const String& text) {
    if (!usbHidReady || text.length() == 0) return;
    
    ensureKeyboardMode(MODE_ENGLISH);
    typeWithSmartTiming(text);
}

// Process Korean jamo
void processKoreanJamo(const String& jamoKeys) {
    if (!usbHidReady || jamoKeys.length() == 0) return;
    
    ensureKeyboardMode(MODE_KOREAN);
    typeWithSmartTiming(jamoKeys);
}

// Process special commands
void processSpecialCommand(const String& command) {
    if (!usbHidReady) return;
    
    String cmd = command;
    cmd.toLowerCase();
    
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
    
    delay(50);
}

// Type with timing
void typeWithSmartTiming(const String& text) {
    uint32_t randomRange = baseTypingDelay / 3;
    
    for (int i = 0; i < text.length(); i++) {
        char c = text.charAt(i);
        
        if (c >= 'A' && c <= 'Z') {
            Keyboard.press(KEY_LEFT_SHIFT);
            delay(20);
            Keyboard.press(c);
            delay(30);
            Keyboard.releaseAll();
            delay(baseTypingDelay + random(randomRange));
        } else {
            Keyboard.write(c);
            delay(baseTypingDelay + random(randomRange));
        }
        
        yield(); // Feed watchdog during long typing
    }
}

void setup() {
    // Initialize serial first
    Serial.begin(115200);
    delay(500);
    Serial.println();
    Serial.println("=== GHOSTYPE Starting ===");
    Serial.flush();
    
    // Initialize LED
    pinMode(RGB_LED_PIN, OUTPUT);
    digitalWrite(RGB_LED_PIN, HIGH); // Turn on during init
    
    // Initialize USB HID
    Serial.println("Initializing USB HID...");
    try {
        USB.begin();
        delay(1000);
        Keyboard.begin();
        delay(500);
        usbHidReady = true;
        currentKeyboardMode = MODE_ENGLISH;
        Serial.println("USB HID: OK");
    } catch (...) {
        Serial.println("USB HID: FAILED");
        usbHidReady = false;
    }
    
    // Initialize BLE
    Serial.println("Initializing BLE...");
    Serial.flush();
    
    try {
        bleManager = new BLENimbleManager();
        if (bleManager) {
            Serial.println("BLE Manager created");
            Serial.flush();
            
            // Try BLE initialization
            if (bleManager->begin()) {
                bleReady = true;
                systemReady = true;
                Serial.println("BLE: OK");
            } else {
                Serial.println("BLE: begin() failed");
                delete bleManager;
                bleManager = nullptr;
            }
        } else {
            Serial.println("BLE Manager creation failed");
        }
    } catch (...) {
        Serial.println("BLE: Exception occurred");
        if (bleManager) {
            delete bleManager;
            bleManager = nullptr;
        }
    }
    
    // Initialization complete
    digitalWrite(RGB_LED_PIN, LOW); // Turn off LED
    lastHeartbeat = millis();
    lastBleCheck = millis();
    
    Serial.println("=== Initialization Complete ===");
    Serial.printf("USB HID: %s\n", usbHidReady ? "Ready" : "Failed");
    Serial.printf("BLE: %s\n", bleReady ? "Ready" : "Failed");
    Serial.flush();
}

void loop() {
    yield(); // Feed watchdog
    
    // Process BLE data
    if (bleReady && bleManager) {
        try {
            if (bleManager->hasReceivedData()) {
                std::string data = bleManager->getReceivedData();
                if (!data.empty()) {
                    processReceivedData(data);
                }
            }
        } catch (...) {
            errorCount++;
            if (errorCount > 5) {
                Serial.println("Too many BLE errors, disabling BLE");
                bleReady = false;
                if (bleManager) {
                    delete bleManager;
                    bleManager = nullptr;
                }
                errorCount = 0;
            }
        }
    }
    
    // LED heartbeat every 2 seconds
    if (millis() - lastHeartbeat > 2000) {
        lastHeartbeat = millis();
        digitalWrite(RGB_LED_PIN, !digitalRead(RGB_LED_PIN));
    }
    
    // BLE status check every 10 seconds
    if (millis() - lastBleCheck > 10000) {
        lastBleCheck = millis();
        if (bleReady && bleManager) {
            Serial.printf("BLE Status: %s\n", 
                bleManager->isAnyDeviceConnected() ? "Connected" : "Advertising");
        }
    }
    
    delay(10);
}