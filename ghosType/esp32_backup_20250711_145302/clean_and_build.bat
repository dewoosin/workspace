@echo off
echo Cleaning PlatformIO build cache...
rd /s /q .pio
rd /s /q .pioenvs
rd /s /q .piolibdeps
echo.
echo Build cache cleaned!
echo.
echo Starting fresh build...
pio run
pause