@echo off
echo ====================================
echo ESP32-S3 긴급 플래시 지우기
echo ====================================
echo.
echo 1. BOOT 버튼을 누른 상태에서
echo 2. RESET 버튼을 짧게 눌렀다 떼고
echo 3. BOOT 버튼을 1-2초 더 누르고 있다가 떼세요
echo.
echo 준비되면 아무 키나 누르세요...
pause > nul

echo.
echo 플래시 지우기 시도 중...
python -m esptool --chip esp32s3 --port COM3 --baud 115200 erase_flash

if errorlevel 1 (
    echo.
    echo 실패! 다른 포트 시도...
    echo.
    python -m esptool --chip esp32s3 --port COM4 --baud 115200 erase_flash
)

if errorlevel 1 (
    echo.
    echo 여전히 실패! 수동으로 포트를 확인하세요.
    echo 장치 관리자에서 COM 포트 번호 확인 필요
)

echo.
pause