import { BLE_CONFIG } from './constants.js';

export class BLEManager {
    constructor() {
        this.device = null;
        this.server = null;
        this.service = null;
        this.rxCharacteristic = null;
        this.txCharacteristic = null;
    }

    log(message, type = 'info') {
        // Simple logging - can be enhanced later
        console.log(`[${type.toUpperCase()}] ${message}`);
        
        // Try to log to UI if available
        if (typeof window !== 'undefined' && window.logger) {
            window.logger.log(message, type);
        }
    }

    async connect() {
        try {
            this.log('🔍 GHOSTYPE 디바이스 검색 중...', 'info');
            this.updateStatus('🔍 디바이스 검색 중...', 'connecting');

            this.device = await navigator.bluetooth.requestDevice({
                filters: [
                    { namePrefix: 'GHOSTYPE' },
                    { services: [BLE_CONFIG.SERVICE_UUID] }
                ],
                optionalServices: [BLE_CONFIG.SERVICE_UUID]
            });

            this.log(`📱 디바이스 발견: ${this.device.name}`, 'success');
            this.device.addEventListener('gattserverdisconnected', () => this.onDisconnected());

            this.log('🔗 GATT 서버 연결 중...', 'info');
            this.server = await this.device.gatt.connect();

            this.log('📡 Nordic UART 서비스 검색 중...', 'info');
            this.service = await this.server.getPrimaryService(BLE_CONFIG.SERVICE_UUID);

            this.log('🔧 특성 설정 중...', 'info');
            this.rxCharacteristic = await this.service.getCharacteristic(BLE_CONFIG.RX_CHAR_UUID);
            this.txCharacteristic = await this.service.getCharacteristic(BLE_CONFIG.TX_CHAR_UUID);

            await this.txCharacteristic.startNotifications();
            this.txCharacteristic.addEventListener('characteristicvaluechanged', (event) => this.onDataReceived(event));

            this.log('🎉 연결 완료! 한글/영문 모두 지원됩니다', 'success');
            this.updateStatus('🟢 연결됨 - 한글/영문 입력 가능', 'connected');
            
            return true;

        } catch (error) {
            this.log(`❌ 연결 실패: ${error.message}`, 'error');
            this.updateStatus('🔴 연결 실패', 'disconnected');
            console.error('Connection failed:', error);
            return false;
        }
    }

    async disconnect() {
        if (this.device && this.device.gatt.connected) {
            await this.device.gatt.disconnect();
        }
    }

    async sendData(message) {
        if (!this.rxCharacteristic) {
            this.log('❌ 연결되지 않음', 'error');
            return false;
        }

        try {
            const encoder = new TextEncoder();
            const data = encoder.encode(message);
            
            await this.rxCharacteristic.writeValue(data);
            this.log(`📤 프로토콜 전송: "${message}"`, 'success');
            return true;
        } catch (error) {
            this.log(`❌ 전송 실패: ${error.message}`, 'error');
            return false;
        }
    }

    onDataReceived(event) {
        const value = new TextDecoder().decode(event.target.value);
        this.log(`📨 ESP32 응답: "${value}"`, 'data');
    }

    onDisconnected() {
        this.log('👋 디바이스 연결 해제됨', 'info');
        this.updateStatus('🔴 연결 해제됨', 'disconnected');
        this.updateUI(false);
        
        this.device = null;
        this.server = null;
        this.service = null;
        this.rxCharacteristic = null;
        this.txCharacteristic = null;
    }

    updateStatus(message, type) {
        const status = document.getElementById('status');
        status.textContent = message;
        status.className = `status ${type}`;
    }

    updateUI(connected) {
        document.getElementById('connectBtn').disabled = connected;
        document.getElementById('disconnectBtn').disabled = !connected;
        document.getElementById('sendBtn').disabled = !connected;
        
        const keyboardTest = document.getElementById('keyboardTest');
        
        if (connected) {
            keyboardTest.style.display = 'block';
            this.log('🇰🇷 한글 자모 변환 기능 활성화', 'info');
            this.log('⌨️ 모든 한글이 자동으로 자모 키로 변환됩니다', 'info');
        } else {
            keyboardTest.style.display = 'none';
        }
    }

    isConnected() {
        return this.rxCharacteristic !== null;
    }
}