# GHOSTYPE Flutter 앱 컨텍스트

## 주요 역할
- 사용자 입력 수집 또는 AI 요청
- OCR 기반 이미지 전송
- BLE 연결 및 데이터 전송
- 히스토리 조회 및 재사용

## 프로젝트 구조 (예시)
frontend/
├── lib/
│ ├── screens/ # 입력 / AI / 히스토리 / 이미지 분석
│ ├── services/ # BLE, API 통신, OCR 처리
│ ├── models/ # 데이터 모델
│ ├── utils/ # 유틸리티 함수
│ └── main.dart



## 화면 흐름
- 홈: 텍스트 입력 or 이미지 업로드
- AI 요청: 프롬프트 → 결과 보기 → 전송
- OCR: 사진 촬영 → 결과 분석 → 전송
- 히스토리: 이전 입력 목록 조회, 재전송
- BLE 연결: 기기 선택, 상태 확인

## 상태 관리
- 전역 상태: Provider 또는 Riverpod 기반
- API 상태: FutureBuilder 또는 상태 캡슐화
- BLE 연결 상태: stream 기반 실시간 처리

## 전송 흐름
1. 사용자가 텍스트 or 프롬프트 입력
2. 서버에 전송
3. 전처리된 키 시퀀스를 받아 BLE로 전송
4. 전송 완료 후 UI 피드백 표시

## 테스트 전략
- 단위 테스트: Dart test
- 화면 테스트: golden test (선택)



