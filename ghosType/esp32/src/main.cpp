// ESP32-S3 BLE Test Firmware
#include <Arduino.h>
#include <NimBLEDevice.h>

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println();
    Serial.println("=================================");
    Serial.println("ESP32-S3 BLE Test Starting...");
    Serial.println("=================================");
    
    // Initialize LED
    pinMode(40, OUTPUT);
    digitalWrite(40, HIGH);
    delay(500);
    digitalWrite(40, LOW);
    
    // Initialize BLE
    Serial.println("Initializing BLE...");
    NimBLEDevice::init("ESP32-S3-Test");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);
    
    // Start advertising
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID("12345678-1234-1234-1234-123456789abc");
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMaxPreferred(0x12);
    pAdvertising->start();
    
    Serial.println("BLE advertising started");
    Serial.println("Setup complete");
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