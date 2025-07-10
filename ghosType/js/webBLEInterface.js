/**
 * Web BLE Interface for ESP32 HID Communication
 * Handles connection and data transmission with Hangul preprocessing
 * ESP32 HID 통신을 위한 웹 BLE 인터페이스
 */

class WebBLEInterface {
    constructor() {
        // BLE service and characteristic UUIDs (must match ESP32)
        // OLD UUIDs - commented out as they don't match ESP32 firmware
        // this.SERVICE_UUID = '12345678-1234-5678-9012-123456789abc';
        // this.RX_CHAR_UUID = '12345678-1234-5678-9012-123456789abd';
        // this.TX_CHAR_UUID = '12345678-1234-5678-9012-123456789abe';
        
        // Current UUIDs matching ESP32 firmware
        this.SERVICE_UUID = '6e400001-b5a3-f393-e0a9-e50e24dcca9e';
        this.RX_CHAR_UUID = '6e400002-b5a3-f393-e0a9-e50e24dcca9e';
        this.TX_CHAR_UUID = '6e400003-b5a3-f393-e0a9-e50e24dcca9e';
        
        // BLE connection state
        this.device = null;
        this.server = null;
        this.service = null;
        this.rxCharacteristic = null;
        this.txCharacteristic = null;
        this.connected = false;
        
        // Initialize Hangul preprocessor
        this.preprocessor = new HangulPreprocessor();
        
        // Default typing configuration
        this.defaultConfig = {
            speed_cps: 6,      // Characters per second
            interval_ms: 100   // Pause after certain characters
        };
    }
    
    /**
     * Connect to ESP32 device via BLE with improved error handling
     * 개선된 오류 처리로 ESP32 장치에 BLE 연결
     */
    async connect() {
        try {
            console.log('🔍 GHOSTYPE 장치 검색 중...');
            
            // 연결 시도 전 기존 연결 정리
            if (this.device && this.device.gatt.connected) {
                await this.device.gatt.disconnect();
                await this.delay(500);
            }
            
            // BLE 장치 요청 (더 관대한 필터)
            console.log('📱 BLE 장치 요청...');
            this.device = await navigator.bluetooth.requestDevice({
                filters: [
                    { name: 'ESP32' },        // 간단한 테스트 펌웨어용
                    { namePrefix: 'ESP32' },  // ESP32 기본 이름
                    { name: 'GHOSTYPE' },
                    { namePrefix: 'GHOST' }   // 백업용
                ],
                optionalServices: [this.SERVICE_UUID],
                acceptAllDevices: false
            });
            
            console.log(`🎯 장치 발견: ${this.device.name || 'Unknown'}`);
            
            // 연결 끊김 이벤트 리스너 추가
            this.device.addEventListener('gattserverdisconnected', () => {
                console.log('⚠️ 장치 연결이 끊어졌습니다');
                this.connected = false;
                this.cleanup();
            });
            
            // GATT 서버 연결 (재시도 로직)
            console.log('🔗 GATT 서버 연결 중...');
            let retryCount = 0;
            const maxRetries = 3;
            
            while (retryCount < maxRetries) {
                try {
                    this.server = await this.device.gatt.connect();
                    console.log('✅ GATT 서버 연결 성공');
                    break;
                } catch (connectError) {
                    retryCount++;
                    console.warn(`❌ GATT 연결 실패 (${retryCount}/${maxRetries}):`, connectError.message);
                    
                    if (retryCount < maxRetries) {
                        console.log(`🔄 ${2000 * retryCount}ms 후 재시도...`);
                        await this.delay(2000 * retryCount);
                    } else {
                        throw new Error(`GATT 서버 연결 실패: ${connectError.message}`);
                    }
                }
            }
            
            // 서비스 가져오기
            console.log('🛠️ 서비스 탐색 중...');
            try {
                this.service = await this.server.getPrimaryService(this.SERVICE_UUID);
                console.log('✅ 서비스 발견');
            } catch (serviceError) {
                throw new Error(`서비스를 찾을 수 없습니다: ${serviceError.message}`);
            }
            
            // 특성 가져오기
            console.log('📡 특성 설정 중...');
            try {
                this.rxCharacteristic = await this.service.getCharacteristic(this.RX_CHAR_UUID);
                this.txCharacteristic = await this.service.getCharacteristic(this.TX_CHAR_UUID);
                console.log('✅ 특성 설정 완료');
            } catch (charError) {
                throw new Error(`특성을 찾을 수 없습니다: ${charError.message}`);
            }
            
            // 알림 설정
            console.log('🔔 알림 설정 중...');
            try {
                await this.txCharacteristic.startNotifications();
                this.txCharacteristic.addEventListener('characteristicvaluechanged', 
                    this.handleNotification.bind(this));
                console.log('✅ 알림 설정 완료');
            } catch (notifyError) {
                console.warn('⚠️ 알림 설정 실패:', notifyError.message);
                // 알림 실패는 치명적이지 않으므로 계속 진행
            }
            
            // 연결 테스트
            console.log('🧪 연결 테스트 중...');
            try {
                await this.sendCommand('GHTYPE_TEST');
                console.log('✅ 연결 테스트 성공');
            } catch (testError) {
                console.warn('⚠️ 연결 테스트 실패:', testError.message);
                // 테스트 실패도 치명적이지 않으므로 계속 진행
            }
            
            this.connected = true;
            console.log('🎉 GHOSTYPE 장치 연결 완료!');
            
            return true;
            
        } catch (error) {
            console.error('💥 연결 실패:', error);
            this.connected = false;
            this.cleanup();
            
            // 사용자 친화적인 오류 메시지
            let userMessage = '연결에 실패했습니다. ';
            if (error.message.includes('User cancelled')) {
                userMessage = '사용자가 연결을 취소했습니다.';
            } else if (error.message.includes('GATT')) {
                userMessage = 'ESP32 장치와 연결할 수 없습니다. 장치가 켜져 있고 범위 내에 있는지 확인하세요.';
            } else if (error.message.includes('서비스')) {
                userMessage = 'GHOSTYPE 서비스를 찾을 수 없습니다. 펌웨어가 올바른지 확인하세요.';
            }
            
            throw new Error(userMessage);
        }
    }
    
    /**
     * Clean up connection resources
     * 연결 리소스 정리
     */
    cleanup() {
        if (this.txCharacteristic) {
            try {
                this.txCharacteristic.removeEventListener('characteristicvaluechanged', 
                    this.handleNotification.bind(this));
            } catch (e) {
                // 무시
            }
        }
        
        this.rxCharacteristic = null;
        this.txCharacteristic = null;
        this.service = null;
        this.server = null;
    }
    
    /**
     * Disconnect from ESP32 device
     * ESP32 장치와의 연결 해제
     */
    disconnect() {
        if (this.device && this.device.gatt.connected) {
            this.device.gatt.disconnect();
        }
        this.connected = false;
        console.log('Disconnected from device');
    }
    
    /**
     * Handle notifications from ESP32
     * ESP32로부터의 알림 처리
     */
    handleNotification(event) {
        const value = event.target.value;
        const decoder = new TextDecoder();
        const message = decoder.decode(value);
        console.log('ESP32 notification:', message);
        
        // Handle different response types
        if (message.startsWith('OK:')) {
            const charCount = message.substring(3);
            console.log(`✅ Typed ${charCount} characters successfully`);
        } else if (message.startsWith('SPD:')) {
            const speed = message.substring(4);
            console.log(`⚡ Speed updated to ${speed} CPS`);
        }
    }
    
    /**
     * Send text to ESP32 with automatic language preprocessing
     * 자동 언어 전처리를 통해 ESP32로 텍스트 전송
     */
    async sendText(text, options = {}) {
        if (!this.connected || !this.rxCharacteristic) {
            throw new Error('Not connected to device');
        }
        
        // Merge with default config
        const config = { ...this.defaultConfig, ...options };
        
        // Preprocess text for Korean-English switching
        const processed = this.preprocessor.formatForESP32(text);
        
        // Handle language toggle markers by sending special commands
        if (processed.hasToggle) {
            // Split text by toggle markers and send each part with mode switch
            const parts = processed.text.split(processed.toggleMarker);
            
            for (let i = 0; i < parts.length; i++) {
                if (parts[i].length === 0) continue;
                
                // Send mode switch command if needed (except for first part)
                if (i > 0) {
                    await this.sendCommand('GHTYPE_SPE:haneng');
                    await this.delay(200); // Wait for mode switch
                }
                
                // Send the text part
                await this.sendJSON({
                    text: parts[i],
                    speed_cps: config.speed_cps,
                    interval_ms: config.interval_ms
                });
                
                // Small delay between parts
                await this.delay(100);
            }
        } else {
            // No language switching needed - send as single JSON
            await this.sendJSON({
                text: processed.text,
                speed_cps: config.speed_cps,
                interval_ms: config.interval_ms
            });
        }
    }
    
    /**
     * Send raw JSON data to ESP32
     * ESP32로 원시 JSON 데이터 전송
     */
    async sendJSON(data) {
        const json = JSON.stringify(data);
        const encoder = new TextEncoder();
        const bytes = encoder.encode(json);
        
        // Check MTU size (ESP32 is configured for 247 bytes)
        if (bytes.length > 247) {
            console.warn('Data exceeds MTU size, may be fragmented');
        }
        
        try {
            await this.rxCharacteristic.writeValueWithoutResponse(bytes);
            console.log('Sent JSON:', json);
        } catch (error) {
            console.error('Failed to send data:', error);
            throw error;
        }
    }
    
    /**
     * Send special command to ESP32
     * ESP32로 특수 명령 전송
     */
    async sendCommand(command) {
        const encoder = new TextEncoder();
        const bytes = encoder.encode(command);
        
        try {
            await this.rxCharacteristic.writeValueWithoutResponse(bytes);
            console.log('Sent command:', command);
        } catch (error) {
            console.error('Failed to send command:', error);
            throw error;
        }
    }
    
    /**
     * Utility delay function
     * 유틸리티 지연 함수
     */
    delay(ms) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }
    
    /**
     * Update typing speed configuration
     * 타이핑 속도 설정 업데이트
     */
    async updateSpeed(cps) {
        if (cps < 1 || cps > 50) {
            throw new Error('Speed must be between 1-50 CPS');
        }
        
        await this.sendCommand(`GHTYPE_CFG:{"mode":"typing","speed_cps":${cps}}`);
        this.defaultConfig.speed_cps = cps;
    }
    
    /**
     * Test the system with various inputs
     * 다양한 입력으로 시스템 테스트
     * UNUSED: This test function is not called anywhere in the current implementation
     */
    // async runSystemTest() {
    //     if (!this.connected) {
    //         console.error('Not connected to device');
    //         return;
    //     }
    //     
    //     console.log('🧪 Running system test...\n');
    //     
    //     const testInputs = [
    //         { text: "Hello World!", delay: 2000 },
    //         { text: "안녕하세요", delay: 2000 },
    //         { text: "Hello 안녕 World!", delay: 3000 },
    //         { text: "대한민국 Korea 화이팅!", delay: 3000 },
    //         { text: "Test 되 vs 돼 example", delay: 3000 }
    //     ];
    //     
    //     for (const test of testInputs) {
    //         console.log(`\nTesting: "${test.text}"`);
    //         await this.sendText(test.text);
    //         await this.delay(test.delay);
    //     }
    //     
    //     console.log('\n✅ System test completed');
    // }
}

// Export for use
if (typeof module !== 'undefined' && module.exports) {
    module.exports = WebBLEInterface;
} else if (typeof window !== 'undefined') {
    window.WebBLEInterface = WebBLEInterface;
}