@echo off
echo ESP32-S3 플래시 메모리를 완전히 지웁니다...
echo.
echo 주의: 모든 데이터가 삭제됩니다!
echo.
pause

esptool.py --chip esp32s3 --port COM3 erase_flash

echo.
echo 플래시 지우기 완료!
echo 이제 새로운 펌웨어를 업로드할 수 있습니다.
pause