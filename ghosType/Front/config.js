// Application Configuration
export const CONFIG = {
    // Application Settings
    APP: {
        NAME: 'GHOSTYPE',
        VERSION: '2.1.0',
        DESCRIPTION: 'BLE 키보드 웹 테스트 (한글 지원)'
    },

    // BLE Configuration
    BLE: {
        DEVICE_NAME_PREFIX: 'GHOSTYPE',
        CONNECTION_TIMEOUT: 10000,
        RECONNECT_ATTEMPTS: 3
    },

    // UI Settings
    UI: {
        DEFAULT_TYPING_SPEED: 6,
        COUNTDOWN_SECONDS: 5,
        TYPING_SPEEDS: [
            { value: 3, label: '3 (Slow)' },
            { value: 6, label: '6 (Normal)' },
            { value: 10, label: '10 (Fast)' }
        ]
    },

    // Test Button Configurations
    TEST_BUTTONS: {
        english: [
            { label: 'Hello World!', value: 'Hello World!' },
            { label: '영문 테스트', value: 'GHOSTYPE Test 123' },
            { label: '타이핑 연습', value: 'The quick brown fox' },
            { label: '프로그래밍', value: 'Programming is fun!' }
        ],
        korean: [
            { label: '안녕하세요', value: '안녕하세요' },
            { label: '테스트입니다', value: '테스트입니다' },
            { label: '한글 키보드', value: '한글 키보드' },
            { label: '프로그래밍', value: '프로그래밍' }
        ],
        mixed: [
            { label: 'Hello 안녕', value: 'Hello 안녕' },
            { label: 'GHOSTYPE 한글', value: 'GHOSTYPE 한글' },
            { label: 'Programming은 재미있어', value: 'Programming은 재미있어' },
            { label: '오늘 날씨가 nice하네요', value: '오늘 날씨가 nice하네요' }
        ],
        special: [
            { label: 'Enter', value: 'enter', type: 'special' },
            { label: 'Tab', value: 'tab', type: 'special' },
            { label: 'Space', value: 'space', type: 'special' },
            { label: 'Backspace', value: 'backspace', type: 'special' },
            { label: 'Ctrl+C', value: 'ctrl+c', type: 'special' },
            { label: 'Ctrl+V', value: 'ctrl+v', type: 'special' }
        ]
    },

    // Localization
    MESSAGES: {
        ko: {
            CONNECTING: '🔍 디바이스 검색 중...',
            CONNECTED: '🟢 연결됨 - 한글/영문 입력 가능',
            DISCONNECTED: '🔴 연결 안됨 - 연결 버튼을 클릭하세요',
            CONNECTION_FAILED: '🔴 연결 실패',
            DEVICE_FOUND: '📱 디바이스 발견',
            BLUETOOTH_NOT_SUPPORTED: '❌ 이 브라우저는 Web Bluetooth를 지원하지 않습니다',
            ENTER_MESSAGE: '⚠️ 메시지를 입력하세요',
            CONNECT_FIRST: '❌ 먼저 GHOSTYPE 디바이스에 연결하세요'
        }
    }
};