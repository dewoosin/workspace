/**
 * Web BLE Interface for ESP32 HID Communication
 * Handles connection and data transmission with Hangul preprocessing
 * ESP32 HID 통신을 위한 웹 BLE 인터페이스
 */

class WebBLEInterface {
    constructor() {
        // BLE service and characteristic UUIDs (must match ESP32)
        this.SERVICE_UUID = '12345678-1234-5678-9012-123456789abc';
        this.RX_CHAR_UUID = '12345678-1234-5678-9012-123456789abd';
        this.TX_CHAR_UUID = '12345678-1234-5678-9012-123456789abe';
        
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
     * Connect to ESP32 device via BLE
     * BLE를 통해 ESP32 장치에 연결
     */
    async connect() {
        try {
            console.log('Scanning for GHOSTYPE device...');
            
            // Request BLE device
            this.device = await navigator.bluetooth.requestDevice({
                filters: [{ name: 'GHOSTYPE' }],
                optionalServices: [this.SERVICE_UUID]
            });
            
            // Connect to GATT server
            console.log('Connecting to GATT server...');
            this.server = await this.device.gatt.connect();
            
            // Get service
            console.log('Getting service...');
            this.service = await this.server.getPrimaryService(this.SERVICE_UUID);
            
            // Get characteristics
            console.log('Getting characteristics...');
            this.rxCharacteristic = await this.service.getCharacteristic(this.RX_CHAR_UUID);
            this.txCharacteristic = await this.service.getCharacteristic(this.TX_CHAR_UUID);
            
            // Set up notifications
            await this.txCharacteristic.startNotifications();
            this.txCharacteristic.addEventListener('characteristicvaluechanged', 
                this.handleNotification.bind(this));
            
            this.connected = true;
            console.log('✅ Connected to GHOSTYPE device');
            
            return true;
        } catch (error) {
            console.error('Connection failed:', error);
            this.connected = false;
            throw error;
        }
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
        
        // Check MTU size (ESP32 is configured for 512 bytes)
        if (bytes.length > 512) {
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
     */
    async runSystemTest() {
        if (!this.connected) {
            console.error('Not connected to device');
            return;
        }
        
        console.log('🧪 Running system test...\n');
        
        const testInputs = [
            { text: "Hello World!", delay: 2000 },
            { text: "안녕하세요", delay: 2000 },
            { text: "Hello 안녕 World!", delay: 3000 },
            { text: "대한민국 Korea 화이팅!", delay: 3000 },
            { text: "Test 되 vs 돼 example", delay: 3000 }
        ];
        
        for (const test of testInputs) {
            console.log(`\nTesting: "${test.text}"`);
            await this.sendText(test.text);
            await this.delay(test.delay);
        }
        
        console.log('\n✅ System test completed');
    }
}

// Export for use
if (typeof module !== 'undefined' && module.exports) {
    module.exports = WebBLEInterface;
} else if (typeof window !== 'undefined') {
    window.WebBLEInterface = WebBLEInterface;
}