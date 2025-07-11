@echo off
echo T-Dongle-S3 업로드 문제 해결
echo.
echo 1. 장치 관리자에서 COM 포트 번호 확인
echo 2. T-Dongle-S3를 뺐다가 다시 연결
echo 3. BOOT 버튼을 누른 상태에서 업로드 시작
echo.
echo 준비되면 아무 키나 누르세요...
pause

echo.
echo BOOT 버튼을 누른 상태에서 업로드를 시작합니다...
pio run -t upload --upload-port COM4

echo.
echo 업로드가 완료되면 BOOT 버튼을 놓으세요.
pause