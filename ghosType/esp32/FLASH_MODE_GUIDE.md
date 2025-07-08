# ESP32-S3 플래시 모드 진입 방법

## 방법 1: BOOT + RESET 버튼 사용
1. **BOOT 버튼을 누른 상태에서**
2. **RESET 버튼을 짧게 눌렀다 떼기**
3. **BOOT 버튼을 계속 누르고 있다가**
4. **1-2초 후 BOOT 버튼 떼기**
5. 이제 플래시 모드로 진입됨

## 방법 2: 전원 연결 시 BOOT 버튼
1. USB 케이블 분리
2. **BOOT 버튼을 누른 상태에서**
3. **USB 케이블 연결**
4. **1-2초 후 BOOT 버튼 떼기**

## 플래시 지우기 명령
```bash
# Windows
python -m esptool --chip esp32s3 --port COM3 erase_flash

# 또는 (PlatformIO 환경)
pio run --target erase

# 또는 (수동으로 포트 지정)
python -m esptool --chip esp32s3 --port COM3 --baud 115200 erase_flash
```

## 확인 방법
- 플래시 모드 진입 시 시리얼 모니터에 출력이 멈춤
- Windows 장치 관리자에서 COM 포트가 정상적으로 보임
- 부트 루프가 멈춤

## 플래시 후 테스트
1. 플래시 지우기 완료 후
2. 간단한 Blink 예제 업로드
3. 정상 작동 확인
4. 그 다음 BLE 코드 추가