# BLE 검색 테스트 방법들

## 1. 스마트폰 앱 (가장 확실한 방법)
- **Android**: nRF Connect 앱 다운로드
- **iPhone**: LightBlue Explorer 앱 다운로드
- 앱에서 "GHOSTYPE-S3" 검색

## 2. Windows 블루투스 설정
- 설정 → 블루투스 및 기타 디바이스
- "블루투스 또는 기타 디바이스 추가"
- "GHOSTYPE-S3" 검색

## 3. Chrome Web Bluetooth (문제가 많을 수 있음)
- chrome://bluetooth-internals/
- "Start Scan" 클릭
- 주의: Web Bluetooth는 제한이 많음

## 4. 명령어로 확인 (Windows)
```cmd
# PowerShell에서
Get-PnpDevice -Class Bluetooth
```

## 확인 순서
1. 스마트폰 앱으로 먼저 테스트 (가장 신뢰할 만함)
2. Windows 블루투스 설정으로 테스트
3. 마지막에 Web Bluetooth 테스트

## 참고
- ESP32는 BLE 4.0+ 지원
- 일부 구형 장치에서는 검색 안 될 수 있음
- 거리: 1m 이내에서 테스트