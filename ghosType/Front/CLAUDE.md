# 웹 프론트엔드 아키텍처 - GHOSTYPE

## 개요

Front/ 디렉토리에는 GHOSTYPE BLE 키보드 시스템의 주요 브라우저 기반 인터페이스 역할을 하는 정교한 **모듈형 웹 애플리케이션**이 포함되어 있습니다. 이는 레거시 모놀리식 구현에서 완전한 아키텍처 진화를 나타내며, 최신 ES6+ 모듈, 포괄적인 한국어 텍스트 처리, 고급 Web Bluetooth 통합 및 광범위한 디버깅 기능을 특징으로 합니다. 이 시스템은 지능적인 폴백과 iOS 마이그레이션 안내를 통해 **크로스 플랫폼 브라우저 지원**을 제공합니다.

## 아키텍처

### 최신 모듈형 설계
```javascript
// ES6 모듈 아키텍처
Front/
├── scripts/                    # 핵심 JavaScript 모듈
│   ├── main.js                # 애플리케이션 오케스트레이션 및 초기화
│   ├── ble-manager.js         # Web Bluetooth API 추상화
│   ├── korean-converter-improved.js # 고급 한글 처리
│   ├── ui-controller.js       # UI 상태 관리 및 상호작용
│   ├── logger.js              # 중앙화된 로깅 시스템
│   └── message-history.js     # 영구 메시지 저장소
├── styles/                     # 모듈형 CSS 아키텍처
│   ├── main.css               # 핵심 스타일 및 레이아웃
│   ├── components.css         # UI 컴포넌트 스타일
│   ├── modals.css             # 모달 다이얼로그 스타일
│   └── responsive.css         # 모바일 반응형
├── components/                 # 재사용 가능한 HTML 컴포넌트
├── config.js                  # 중앙화된 구성
└── index-refactored.html      # 최신 구현
```

### 구성 중심 아키텍처
```javascript
// config.js - 중앙화된 애플리케이션 설정
export const CONFIG = {
    APP: {
        NAME: 'GHOSTYPE',
        VERSION: '2.1.0',
        DEBUG: true
    },
    BLE: {
        DEVICE_NAME_PREFIX: 'GHOSTYPE',
        SERVICE_UUID: '6e400001-b5a3-f393-e0a9-e50e24dcca9e',
        RX_CHAR_UUID: '6e400002-b5a3-f393-e0a9-e50e24dcca9e',
        TX_CHAR_UUID: '6e400003-b5a3-f393-e0a9-e50e24dcca9e',
        CONNECTION_TIMEOUT: 10000,
        RECONNECT_ATTEMPTS: 3
    },
    UI: {
        DEFAULT_TYPING_SPEED: 6,
        COUNTDOWN_SECONDS: 5,
        AUTO_CLEAR_AFTER_SEND: true
    }
};
```

### 모듈 상호작용 패턴
```javascript
// main.js를 오케스트레이터로 하는 허브 앤 스포크 아키텍처
main.js (오케스트레이터)
├── BLEManager (Web Bluetooth 추상화)
├── UIController (사용자 인터페이스 관리)
├── KoreanConverter (텍스트 처리)
├── Logger (디버깅 및 진단)
└── MessageHistory (영구 저장소)
```

## 컴포넌트 플로우

### 1. **애플리케이션 초기화 플로우**
```javascript
// main.js - 애플리케이션 부트스트랩
document.addEventListener('DOMContentLoaded', async () => {
    try {
        // 핵심 모듈 초기화
        bleManager = new BLEManager();
        uiController = new UIController(bleManager);
        
        // 이벤트 리스너 설정
        setupEventListeners();
        
        // UI 상태 초기화
        uiController.initialize();
        
        logger.log('애플리케이션이 성공적으로 초기화되었습니다', 'success');
    } catch (error) {
        logger.log(`초기화 실패: ${error.message}`, 'error');
    }
});
```

### 2. **Web Bluetooth 연결 플로우**
```javascript
// BLE 연결 프로세스
navigator.bluetooth.requestDevice({
    filters: [{ namePrefix: 'GHOSTYPE' }],
    optionalServices: ['6e400001-b5a3-f393-e0a9-e50e24dcca9e']
})
→ 장치 선택
→ GATT 서버 연결
→ 서비스 검색
→ 특성 설정
→ 준비 상태
```

### 3. **한국어 텍스트 처리 파이프라인**
```javascript
// 고급 한글 처리 워크플로우
function processKoreanText(inputText) {
    // 1. 언어 감지 및 세분화
    const segments = segmentTextByLanguage(inputText);
    
    // 2. 한국어 텍스트 변환
    const convertedSegments = segments.map(segment => {
        if (segment.language === 'korean') {
            return convertHangulToQwerty(segment.text);
        }
        return segment.text;
    });
    
    // 3. 프로토콜 생성
    const protocol = generateProtocol(segments);
    
    // 4. 실시간 미리보기 업데이트
    updatePreview(convertedSegments, protocol);
    
    return { segments: convertedSegments, protocol };
}
```

### 4. **완전한 사용자 상호작용 플로우**
```
사용자 입력 → 언어 감지 → 텍스트 변환 → 프로토콜 생성 → 실시간 미리보기
    ↓             ↓            ↓              ↓              ↓
텍스트 영역 → 유니코드 분석 → 한글 → QWERTY → 명령 블록 → UI 업데이트
    ↓
BLE 전송 → ESP32 처리 → USB HID 출력 → 호스트 컴퓨터
    ↓           ↓            ↓              ↓
연결 확인 → 명령 큐 → 키보드 이벤트 → 텍스트 입력
```

### 5. **고급 한국어 처리**
```javascript
// 포괄적인 유니코드 한글 분해
function decomposeHangul(char) {
    const code = char.charCodeAt(0);
    if (code < 0xAC00 || code > 0xD7A3) return null;
    
    const syllableIndex = code - 0xAC00;
    const chosungIndex = Math.floor(syllableIndex / 588);
    const jungsungIndex = Math.floor((syllableIndex % 588) / 28);
    const jongsungIndex = syllableIndex % 28;
    
    const result = [chosung[chosungIndex], jungsung[jungsungIndex]];
    if (jongsungIndex > 0) result.push(jongsung[jongsungIndex]);
    
    return result;
}

// 94% 개선: 32 → 62개 총 매핑
const qwertyToJamo = {
    // 기본 자음
    'q': 'ㅂ', 'w': 'ㅈ', 'e': 'ㄷ', 'r': 'ㄱ', 't': 'ㅅ',
    'a': 'ㅁ', 's': 'ㄴ', 'd': 'ㅇ', 'f': 'ㄹ', 'g': 'ㅎ',
    'z': 'ㅋ', 'x': 'ㅌ', 'c': 'ㅊ', 'v': 'ㅍ',
    // 쌍자음을 위한 Shift 조합
    'Q': 'ㅃ', 'W': 'ㅉ', 'E': 'ㄸ', 'R': 'ㄲ', 'T': 'ㅆ',
    // 완전한 모음 시스템
    'y': 'ㅛ', 'u': 'ㅕ', 'i': 'ㅑ', 'o': 'ㅐ', 'p': 'ㅔ',
    'h': 'ㅗ', 'j': 'ㅓ', 'k': 'ㅏ', 'l': 'ㅣ',
    'b': 'ㅠ', 'n': 'ㅜ', 'm': 'ㅡ'
    // ... 완전한 커버리지를 위한 추가 매핑
};
```

## 기술적 이슈

### 현재 브라우저 제한사항

#### 1. **플랫폼 호환성 매트릭스**
| 브라우저 | 플랫폼 | Web Bluetooth 지원 | 상태 |
|---------|----------|----------------------|--------|
| Chrome 70+ | Windows/macOS/Linux | ✅ 전체 | 지원됨 |
| Chrome 56+ | Android | ✅ 전체 | 지원됨 |
| Edge 79+ | Windows | ✅ 전체 | 지원됨 |
| Opera 57+ | 모든 플랫폼 | ✅ 전체 | 지원됨 |
| Safari | iOS/macOS | ❌ 없음 | **중요한 격차** |
| Firefox | 모든 플랫폼 | ❌ 없음 | 제한적 지원 |

#### 2. **iOS Web Bluetooth 제한**
```javascript
// iOS 감지 및 사용자 안내
function detectiOSAndProvideAlternatives() {
    const isIOS = /iPad|iPhone|iPod/.test(navigator.userAgent);
    const isSafari = /Safari/.test(navigator.userAgent) && !/Chrome/.test(navigator.userAgent);
    
    if (isIOS || isSafari) {
        showModal({
            title: 'iOS/Safari 미지원',
            message: 'Web Bluetooth는 iOS Safari에서 사용할 수 없습니다. 네이티브 iOS 앱을 사용해주세요.',
            actions: [
                { text: 'iOS 앱 다운로드', action: () => redirectToAppStore() },
                { text: '계속 진행', action: () => showLimitedFeatureWarning() }
            ]
        });
    }
}
```

#### 3. **보안 및 권한 제약**
- **HTTPS 요구사항**: BLE 접근을 위한 보안 컨텍스트 필수
- **사용자 제스처 필요**: BLE 작업은 사용자 상호작용으로 시작되어야 함
- **권한 지속성**: 브라우저 세션 간 권한이 유지되지 않음
- **교차 출처 제한**: BLE 접근은 안전한 동일 출처 컨텍스트로 제한

#### 4. **성능 제한사항**
```javascript
// 브라우저별 성능 고려사항
const performanceBottlenecks = {
    '문자열 처리': '메인 스레드에서 무거운 한국어 텍스트 변환',
    '메모리 사용': '대용량 변환 테이블 및 메시지 기록',
    'BLE 지연': '브라우저 BLE 스택 오버헤드 (20-50ms)',
    'UI 차단': '동기 텍스트 처리가 UI 스레드를 차단'
};
```

### 기술적 과제

#### 1. **Web Bluetooth API 제한사항**
```javascript
// BLE 연결 제약
const bleConstraints = {
    mtuSize: 185,  // 호환성을 위한 보수적인 MTU
    connectionTimeout: 10000,  // 10초 타임아웃
    maxReconnectAttempts: 3,
    chunkSize: 150  // 대용량 메시지를 위한 데이터 청킹
};
```

#### 2. **한국어 텍스트 처리 복잡성**
```javascript
// 복잡한 한국어 문자 처리
const complexCases = {
    '복합 모음': 'ㅘ → ㅗ + ㅏ, ㅢ → ㅡ + ㅣ',
    '복합 자음': 'ㄳ → ㄱ + ㅅ, ㄺ → ㄹ + ㄱ',
    '특수 음절': '돼, 뭐 등에 대한 맞춤 처리',
    '혼합 언어': '세분화 및 모드 전환 최적화'
};
```

#### 3. **실시간 처리 요구사항**
```javascript
// 성능 벤치마크
const processingMetrics = {
    '한국어 변환': '1000자 약 2ms',
    '프로토콜 생성': '복잡한 텍스트의 경우 100ms 미만',
    'UI 업데이트': '차단 없는 실시간 미리보기',
    '메모리 사용': '제한된 기록 (최대 50개 항목)'
};
```

## 개선 권장사항

### 1. **프로그레시브 웹 앱 향상**
```javascript
// 서비스 워커 구현
class ServiceWorkerManager {
    async registerSW() {
        if ('serviceWorker' in navigator) {
            const registration = await navigator.serviceWorker.register('/sw.js');
            this.setupBackgroundSync(registration);
            this.enableOfflineFunctionality();
        }
    }
    
    setupBackgroundSync(registration) {
        // 오프라인일 때 BLE 작업 큐
        // 연결 복원 시 동기화
    }
    
    enableOfflineFunctionality() {
        // 한국어 변환 테이블 캐시
        // 오프라인 텍스트 처리 활성화
        // 대기 중인 메시지 저장
    }
}
```

### 2. **고급 오류 처리 및 복구**
```javascript
// 강력한 오류 처리 프레임워크
class ErrorHandler {
    constructor() {
        this.retryStrategies = new Map();
        this.fallbackMethods = new Map();
    }
    
    async handleBLEError(error, context) {
        logger.log(`BLE 오류: ${error.message}`, 'error');
        
        switch (error.name) {
            case 'NotConnectedError':
                return this.attemptReconnection(context);
            case 'SecurityError':
                return this.handlePermissionError();
            case 'NetworkError':
                return this.retryWithBackoff(context);
            default:
                return this.genericErrorRecovery(error, context);
        }
    }
    
    async attemptReconnection(context) {
        for (let i = 0; i < CONFIG.BLE.RECONNECT_ATTEMPTS; i++) {
            try {
                await this.reconnectBLE();
                return true;
            } catch (retryError) {
                await this.exponentialBackoff(i);
            }
        }
        return false;
    }
}
```

### 3. **성능 최적화 프레임워크**
```javascript
// 무거운 처리를 위한 웹 워커
class TextProcessingWorker {
    constructor() {
        this.worker = new Worker('/workers/korean-processor.js');
        this.setupWorkerCommunication();
    }
    
    async processLargeText(text) {
        return new Promise((resolve, reject) => {
            this.worker.postMessage({ 
                type: 'PROCESS_KOREAN', 
                text: text 
            });
            
            this.worker.onmessage = (e) => {
                if (e.data.type === 'PROCESSING_COMPLETE') {
                    resolve(e.data.result);
                }
            };
        });
    }
}

// 메모리 관리 최적화
class MemoryManager {
    constructor() {
        this.memoryThreshold = 50 * 1024 * 1024; // 50MB
        this.cleanupInterval = 300000; // 5분
    }
    
    monitorMemoryUsage() {
        if (performance.memory) {
            const usage = performance.memory.usedJSHeapSize;
            if (usage > this.memoryThreshold) {
                this.performCleanup();
            }
        }
    }
    
    performCleanup() {
        // 오래된 메시지 기록 삭제
        // 대용량 객체 가비지 수집
        // 변환 테이블 저장소 최적화
    }
}
```

### 4. **크로스 플랫폼 호환성 솔루션**
```javascript
// 플랫폼 어댑터 패턴
class PlatformAdapter {
    static create() {
        const platform = this.detectPlatform();
        
        switch (platform) {
            case 'ios':
                return new IOSAdapter();
            case 'android':
                return new AndroidAdapter();
            case 'desktop':
                return new DesktopAdapter();
            default:
                return new DefaultAdapter();
        }
    }
    
    static detectPlatform() {
        const userAgent = navigator.userAgent;
        if (/iPad|iPhone|iPod/.test(userAgent)) return 'ios';
        if (/Android/.test(userAgent)) return 'android';
        return 'desktop';
    }
}

class IOSAdapter extends PlatformAdapter {
    async initializeBLE() {
        // iOS 앱 다운로드 프롬프트 표시
        throw new Error('iOS에서는 Web Bluetooth가 지원되지 않습니다');
    }
    
    getAlternativeSolutions() {
        return {
            nativeApp: 'https://apps.apple.com/ghostype',
            webRTCBridge: 'WebRTC 브리지 서버 사용',
            manualEntry: '복사-붙여넣기 워크플로우'
        };
    }
}
```

### 5. **고급 UI/UX 향상**
```javascript
// 지능적인 UI 적응
class SmartUIController extends UIController {
    constructor(bleManager) {
        super(bleManager);
        this.platformAdapter = PlatformAdapter.create();
        this.userPreferences = new UserPreferenceManager();
    }
    
    adaptInterfaceForPlatform() {
        if (this.platformAdapter.isMobile()) {
            this.enableMobileOptimizations();
            this.setupTouchGestures();
        }
        
        if (this.platformAdapter.isIOS()) {
            this.showIOSAlternatives();
            this.disableBLEFeatures();
        }
    }
    
    enableAccessibilityFeatures() {
        // 고대비 모드
        // 스크린 리더 지원
        // 키보드 탐색
        // 음성 입력 통합
    }
}
```

## 미래 서버 통합 계획

### 1. **서버 매개 아키텍처 전환**
```javascript
// 미래 하이브리드 아키텍처
class HybridConnectionManager {
    constructor() {
        this.directBLE = new DirectBLEManager();
        this.serverProxy = new ServerProxyManager();
        this.connectionMode = 'auto'; // auto, direct, server
    }
    
    async connect() {
        switch (this.connectionMode) {
            case 'direct':
                return this.connectDirectBLE();
            case 'server':
                return this.connectViaServer();
            case 'auto':
                return this.tryDirectThenServer();
        }
    }
    
    async tryDirectThenServer() {
        try {
            // 먼저 직접 BLE 시도
            return await this.connectDirectBLE();
        } catch (bleError) {
            logger.log('직접 BLE 실패, 서버 프록시 시도', 'warning');
            return await this.connectViaServer();
        }
    }
}
```

### 2. **WebSocket 기반 BLE 프록시**
```javascript
// iOS 및 non-BLE 브라우저를 위한 서버 프록시
class ServerProxyManager {
    constructor() {
        this.websocket = null;
        this.apiEndpoint = CONFIG.SERVER.WEBSOCKET_URL;
        this.sessionId = this.generateSessionId();
    }
    
    async connectViaServer() {
        this.websocket = new WebSocket(this.apiEndpoint);
        
        this.websocket.onopen = () => {
            this.registerClient();
            this.requestDeviceList();
        };
        
        this.websocket.onmessage = (event) => {
            this.handleServerMessage(JSON.parse(event.data));
        };
    }
    
    async sendTypingCommand(text) {
        const message = {
            type: 'TYPING_COMMAND',
            sessionId: this.sessionId,
            deviceId: this.selectedDeviceId,
            command: this.generateProtocol(text),
            timestamp: Date.now()
        };
        
        this.websocket.send(JSON.stringify(message));
    }
}
```

### 3. **API 통합 프레임워크**
```javascript
// RESTful API 통합
class APIManager {
    constructor() {
        this.baseURL = CONFIG.SERVER.API_BASE_URL;
        this.authToken = null;
        this.refreshToken = null;
    }
    
    async authenticate(credentials) {
        const response = await fetch(`${this.baseURL}/auth/login`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(credentials)
        });
        
        const data = await response.json();
        this.authToken = data.accessToken;
        this.refreshToken = data.refreshToken;
        
        this.scheduleTokenRefresh();
    }
    
    async getDeviceList() {
        return this.authorizedRequest('/devices');
    }
    
    async sendTypingCommand(deviceId, command) {
        return this.authorizedRequest(`/devices/${deviceId}/type`, {
            method: 'POST',
            body: JSON.stringify({ command })
        });
    }
}
```

### 4. **크로스 플랫폼 세션 관리**
```javascript
// 플랫폼 간 통합 세션 관리
class SessionManager {
    constructor() {
        this.sessionData = {
            devicePreferences: {},
            typingHistory: [],
            userSettings: {},
            connectionHistory: []
        };
    }
    
    async syncWithServer() {
        try {
            // 로컬 세션 데이터 업로드
            await this.uploadSessionData();
            
            // 서버 세션 데이터 다운로드
            const serverData = await this.downloadSessionData();
            
            // 병합 및 충돌 해결
            this.mergeSessionData(serverData);
        } catch (error) {
            logger.log('세션 동기화 실패, 로컬 데이터 사용', 'warning');
        }
    }
    
    enableCrossPlatformSync() {
        // 웹과 모바일 간 설정 동기화
        // 타이핑 기록 및 환경설정 공유
        // 장치 핸드오프 활성화
    }
}
```

### 5. **점진적 향상 전략**
```javascript
// 기능 감지 및 점진적 향상
class FeatureDetector {
    static getCapabilities() {
        return {
            webBluetooth: 'bluetooth' in navigator,
            serviceWorker: 'serviceWorker' in navigator,
            webRTC: 'RTCPeerConnection' in window,
            webSocket: 'WebSocket' in window,
            localStorage: 'localStorage' in window,
            indexedDB: 'indexedDB' in window
        };
    }
    
    static createOptimalExperience() {
        const capabilities = this.getCapabilities();
        
        if (capabilities.webBluetooth) {
            return new DirectBLEExperience();
        } else if (capabilities.webSocket) {
            return new ServerProxyExperience();
        } else {
            return new FallbackExperience();
        }
    }
}
```

## 개발 통합 가이드라인

### 1. **모듈 종속성 관리**
```javascript
// 명확한 모듈 인터페이스 및 종속성
const moduleGraph = {
    'main.js': ['ble-manager', 'ui-controller', 'logger'],
    'ble-manager.js': ['config', 'logger'],
    'ui-controller.js': ['korean-converter', 'message-history', 'logger'],
    'korean-converter-improved.js': ['config'],
    'message-history.js': ['config', 'logger']
};
```

### 2. **iOS 구현과의 상호 참조**
- **한국어 처리**: Flutter 앱에서 사용된 동일한 알고리즘
- **BLE 프로토콜**: 동일한 Nordic UART 서비스 및 명령 구조
- **설정 관리**: 플랫폼 간 일관된 구성
- **오류 처리**: 병렬 오류 복구 전략

### 3. **서버 통합 준비**
```javascript
// 미래 서버 통합을 위한 구성
const SERVER_CONFIG = {
    WEBSOCKET_URL: 'wss://api.ghostype.com/ws',
    API_BASE_URL: 'https://api.ghostype.com/v1',
    AUTH_ENDPOINTS: {
        login: '/auth/login',
        refresh: '/auth/refresh',
        logout: '/auth/logout'
    },
    DEVICE_ENDPOINTS: {
        list: '/devices',
        connect: '/devices/{id}/connect',
        type: '/devices/{id}/type',
        status: '/devices/{id}/status'
    }
};
```

---

**현재 상태**: ✅ **프로덕션 준비 완료** (직접 BLE 모드)  
**iOS 지원**: ❌ **네이티브 앱 필요**  
**서버 통합**: 🔄 **아키텍처 준비됨**  
**최종 업데이트**: 2025년 7월  
**지원 브라우저**: Chrome 70+, Edge 79+, Opera 57+  
**플랫폼 커버리지**: Windows, macOS, Linux, Android