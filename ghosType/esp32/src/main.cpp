// Minimal test firmware for ESP32-S3
// Test basic functionality without BLE

#include <Arduino.h>

void setup() {
    // Initialize serial with delay
    Serial.begin(115200);
    delay(2000); // Give time for serial connection
    
    Serial.println();
    Serial.println("=================================");
    Serial.println("ESP32-S3 Basic Test Starting...");
    Serial.println("=================================");
    Serial.flush();
    
    // Test basic GPIO (LED)
    Serial.println("Testing GPIO...");
    pinMode(40, OUTPUT); // RGB_LED_PIN
    digitalWrite(40, HIGH);
    delay(500);
    digitalWrite(40, LOW);
    Serial.println("GPIO test complete");
    
    Serial.println("Setup complete - entering main loop");
    Serial.flush();
}

void loop() {
    // Simple blink test
    static unsigned long lastBlink = 0;
    static bool ledState = false;
    
    if (millis() - lastBlink > 1000) {
        lastBlink = millis();
        ledState = !ledState;
        digitalWrite(40, ledState);
        
        Serial.print("Loop running... uptime: ");
        Serial.print(millis() / 1000);
        Serial.println(" seconds");
        Serial.flush();
    }
    
    // Essential: prevent watchdog
    delay(10);
    yield();
}