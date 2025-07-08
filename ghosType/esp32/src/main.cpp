#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

// T-Dongle-S3 LCD 핀 정의
#define TFT_CS   4
#define TFT_DC   5
#define TFT_RST  1
#define TFT_MOSI 3
#define TFT_SCLK 2

// 버튼 핀
#define BUTTON_PIN 0

// LCD 객체 생성
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

void setup() {
    // 시리얼 초기화
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("=== T-Dongle-S3 LCD Test ===");
    
    // 버튼 설정
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    
    // LCD 초기화 시도
    Serial.println("Initializing LCD...");
    
    // ST7735 0.96" 80x160 초기화
    tft.initR(INITR_MINI160x80);
    tft.setRotation(3);  // 가로 모드
    
    // 화면 지우기
    tft.fillScreen(ST7735_BLACK);
    
    // 테스트 텍스트
    tft.setCursor(0, 0);
    tft.setTextColor(ST7735_WHITE);
    tft.setTextSize(1);
    tft.println("T-Dongle-S3");
    tft.println("LCD Test");
    
    // 색상 테스트
    tft.fillRect(0, 20, 20, 20, ST7735_RED);
    tft.fillRect(20, 20, 20, 20, ST7735_GREEN);
    tft.fillRect(40, 20, 20, 20, ST7735_BLUE);
    
    Serial.println("LCD initialized!");
    Serial.println("Press BOOT button to test");
}

void loop() {
    static bool buttonPressed = false;
    static int pressCount = 0;
    
    // 버튼 확인
    if (digitalRead(BUTTON_PIN) == LOW && !buttonPressed) {
        buttonPressed = true;
        pressCount++;
        
        Serial.print("Button pressed! Count: ");
        Serial.println(pressCount);
        
        // LCD에 카운트 표시
        tft.fillRect(0, 50, 160, 20, ST7735_BLACK);  // 이전 텍스트 지우기
        tft.setCursor(0, 50);
        tft.setTextColor(ST7735_YELLOW);
        tft.print("Press: ");
        tft.println(pressCount);
        
    } else if (digitalRead(BUTTON_PIN) == HIGH) {
        buttonPressed = false;
    }
    
    delay(50);  // 디바운스
}