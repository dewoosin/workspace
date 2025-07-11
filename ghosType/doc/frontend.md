# GHOSTYPE Flutter 앱 아키텍처

## 개요
GHOSTYPE Flutter 앱은 보안망 환경에서 AI 기반 텍스트 자동 입력을 위한 모바일 클라이언트입니다. 사용자와 ESP32 HID 키보드 사이의 브릿지 역할을 하며, AI 통합과 BLE 통신을 담당합니다.

> 📖 **전체 시스템 아키텍처**: [CLAUDE.md](../CLAUDE.md) 참조

## 핵심 기능
- **통합 입력 인터페이스**: 단일 채팅창으로 모든 기능 접근
- **AI 서비스 통합**: ChatGPT, Claude 등과 연동
- **OCR 화면 분석**: 카메라로 오류 화면 촬영 및 분석
- **BLE 통신**: ESP32와 안정적인 무선 연결
- **히스토리 관리**: 전송 내역 저장 및 재사용

## 프로젝트 구조

```
ios_app/                           # Flutter 프로젝트 루트
├── lib/
│   ├── main.dart                 # 앱 진입점
│   ├── screens/                  # UI 화면
│   │   ├── chat_screen.dart      # 메인 채팅 인터페이스
│   │   ├── history_screen.dart   # 전송 히스토리
│   │   ├── settings_screen.dart  # 설정 화면
│   │   └── camera_screen.dart    # OCR 카메라
│   ├── services/                 # 비즈니스 로직
│   │   ├── ble_service.dart      # BLE 통신 관리
│   │   ├── api_service.dart      # 서버 API 통신
│   │   ├── ai_service.dart       # AI 통합 (GPT/Claude)
│   │   └── ocr_service.dart      # 이미지 분석
│   ├── models/                   # 데이터 모델
│   │   ├── message.dart          # 채팅 메시지
│   │   ├── ble_device.dart       # BLE 장치 정보
│   │   └── command.dart          # ESP32 명령
│   ├── widgets/                  # 재사용 컴포넌트
│   │   ├── message_bubble.dart   # 메시지 말풍선
│   │   ├── ble_status.dart       # BLE 연결 상태
│   │   └── typing_indicator.dart # 타이핑 표시
│   └── utils/                    # 유틸리티
│       ├── constants.dart        # 상수 정의
│       ├── text_processor.dart   # 텍스트 전처리
│       └── theme.dart            # 앱 테마
├── android/                      # Android 플랫폼 코드
├── ios/                          # iOS 플랫폼 코드
└── pubspec.yaml                  # 의존성 관리
```



## 화면 설계

### 1. 메인 채팅 화면 (ChatScreen)
```dart
// 핵심 UI 구성
- 상단: BLE 연결 상태 표시
- 중앙: 채팅 메시지 목록 (사용자/AI 구분)
- 하단: 통합 입력창 + 카메라/전송 버튼

// 주요 기능
- 텍스트 입력 시 실시간 유효성 검사
- AI 응답 스트리밍 표시
- 메시지별 [저장], [전송] 액션 버튼
- 특수 명령어 지원 (예: "전송내역" 입력)
```

### 2. 전송 팝업 (SendDialog)
```dart
// 사용자가 전송 버튼 클릭 시
AlertDialog(
  title: "이 내용을...",
  actions: [
    TextButton("AI 요청"),    // → AI 서비스 호출
    TextButton("전송"),       // → BLE 전송 시작
  ]
)
```

### 3. BLE 연결 화면 (BleConnectionScreen)
```dart
// BLE 장치 스캔 및 연결
- 자동 스캔 시작
- 발견된 ESP32 장치 목록 표시
- 연결 상태 실시간 업데이트
- 연결 후 자동으로 이전 화면 복귀
```

### 4. OCR 카메라 화면 (CameraScreen)
```dart
// 화면 촬영 및 분석
- 카메라 프리뷰
- 촬영 가이드라인 표시
- 촬영 후 이미지 확인
- OCR 처리 중 로딩 표시
- 분석 결과 표시 및 AI 해석
```

## 상태 관리 아키텍처

### Provider 패턴 구조
```dart
// 주요 Provider
- ChatProvider        // 채팅 메시지 및 AI 상태
- BleProvider        // BLE 연결 및 장치 관리
- HistoryProvider    // 전송 히스토리
- SettingsProvider   // 사용자 설정

// 상태 흐름
App
└── MultiProvider
    ├── ChatProvider
    ├── BleProvider
    └── HistoryProvider
```

### BLE 상태 관리 (실제 구현)
```dart
class BleProvider extends ChangeNotifier {
  // BLE UUID - ESP32와 일치
  static const String serviceUuid = '6e400001-b5a3-f393-e0a9-e50e24dcca9e';
  static const String rxCharUuid = '6e400002-b5a3-f393-e0a9-e50e24dcca9e';
  static const String txCharUuid = '6e400003-b5a3-f393-e0a9-e50e24dcca9e';
  
  // 연결 상태
  BluetoothDevice? _device;
  BluetoothCharacteristic? _rxCharacteristic;
  BluetoothCharacteristic? _txCharacteristic;
  bool _isConnected = false;
  
  // 로그 메시지
  final List<LogMessage> _logs = [];
}
```

## 데이터 흐름

### 1. 텍스트 처리 흐름 (현재 구현)
```
사용자 입력 → TextConversionProvider → 한글 분해 → QWERTY 매핑
    ↓                                        ↓              ↓
UI 미리보기 ← 변환 결과 ← 자모 분리 ← 유니코드 분석
```

### 2. BLE 전송 흐름 (현재 구현)
```
전송 요청 → 로컬 프로토콜 생성 → 명령 시퀀스 조립
    ↓                              ↓
BLE 연결 확인 → ESP32로 직접 전송 → ESP32 응답 수신
    ↓                              ↓
UI 피드백 → 로그 기록 → 전송 완료
```

### 3. OCR 분석 흐름 (미구현)
```
⚠️ 서버가 없어 OCR 기능은 현재 구현되지 않음

향후 계획:
카메라 촬영 → 이미지 압축 → 서버 OCR API
    ↓                          ↓
미리보기 표시 → 텍스트 추출 → AI 분석 요청
    ↓                          ↓
결과 표시 ← 해결책 수신 ← AI 응답
```

## BLE 통신 프로토콜

### 서비스 및 특성 UUID (실제 구현)
```dart
// Nordic UART 서비스 (ESP32와 웹에서 동일하게 사용)
static const serviceUuid = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
static const rxCharUuid = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";
static const txCharUuid = "6e400003-b5a3-f393-e0a9-e50e24dcca9e";

// 실제 구현된 프로토콜
- #CMD:HANGUL        // 한글 모드 전환
- #CMD:ENGLISH       // 영문 모드 전환
- #TEXT:content      // 텍스트 전송
- #CMD:ENTER         // 엔터키 입력
```

### 패킷 구조
```dart
// BLE 패킷 분할 (MTU: 512 bytes)
class BlePacket {
  final int sequence;      // 패킷 순서
  final int total;         // 전체 패킷 수
  final Uint8List data;    // 실제 데이터
  final String checksum;   // 무결성 검증
}
```

## 보안 고려사항

### 데이터 보안
- API 키는 서버에만 저장 (클라이언트 노출 금지)
- 사용자 인증 토큰 안전한 저장 (Flutter Secure Storage)
- BLE 통신 암호화 (개발 중)

### 권한 관리
```yaml
# iOS (Info.plist)
- NSBluetoothAlwaysUsageDescription
- NSCameraUsageDescription
- NSPhotoLibraryUsageDescription

# Android (AndroidManifest.xml)
- android.permission.BLUETOOTH
- android.permission.CAMERA
- android.permission.INTERNET
```

## 성능 최적화

### 메모리 관리
- 채팅 메시지 페이지네이션 (100개 단위)
- 이미지 압축 후 업로드 (최대 2MB)
- BLE 스캔 타임아웃 설정 (30초)

### 배터리 최적화
- BLE 스캔 주기적 중단
- 백그라운드 작업 최소화
- 화면 꺼짐 시 연결 유지 옵션

## 테스트 전략

### 단위 테스트
```dart
// 주요 테스트 대상
- TextProcessor: 한글/영문 변환 로직
- BlePacketizer: 패킷 분할/조합
- CommandParser: 명령어 파싱
```

### 통합 테스트
```dart
// 시나리오 테스트
- AI 요청 → 응답 → 전송 전체 플로우
- BLE 연결 → 전송 → 완료 확인
- OCR 촬영 → 분석 → 결과 표시
```

## 관련 문서
- **[backend.md](backend.md)**: 서버 API 명세
- **[../CLAUDE.md](../CLAUDE.md)**: 전체 시스템 아키텍처
- **[README.md](README.md)**: 프로젝트 개요



