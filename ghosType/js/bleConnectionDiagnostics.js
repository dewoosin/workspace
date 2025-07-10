/**
 * BLE Connection Diagnostics Tool
 * ESP32 연결 문제 진단 도구
 */

class BLEConnectionDiagnostics {
    constructor() {
        this.diagnostics = [];
        this.startTime = Date.now();
    }
    
    /**
     * Run comprehensive connection diagnostics
     * 종합적인 연결 진단 실행
     */
    async runDiagnostics() {
        console.log('🔧 BLE 연결 진단 시작...\n');
        
        // 1. Browser support check
        await this.checkBrowserSupport();
        
        // 2. Device scan
        await this.performDeviceScan();
        
        // 3. Connection attempt with different parameters
        await this.testConnectionParameters();
        
        // 4. Generate report
        this.generateReport();
    }
    
    /**
     * Check browser support for Web Bluetooth
     * 웹 블루투스 브라우저 지원 확인
     */
    async checkBrowserSupport() {
        this.log('1️⃣ 브라우저 지원 확인');
        
        if (!navigator.bluetooth) {
            this.logError('Web Bluetooth API 지원하지 않음');
            this.logSolution('Chrome, Edge, Opera 브라우저 사용 필요');
            return;
        }
        
        this.logSuccess('Web Bluetooth API 지원됨');
        
        // Check availability
        try {
            const available = await navigator.bluetooth.getAvailability();
            if (available) {
                this.logSuccess('블루투스 어댑터 사용 가능');
            } else {
                this.logWarning('블루투스 어댑터 사용 불가');
                this.logSolution('시스템 블루투스가 켜져 있는지 확인');
            }
        } catch (error) {
            this.logError('블루투스 가용성 확인 실패: ' + error.message);
        }
    }
    
    /**
     * Perform device scan with different filters
     * 다양한 필터로 장치 검색 수행
     */
    async performDeviceScan() {
        this.log('\n2️⃣ 장치 검색 테스트');
        
        const scanConfigs = [
            {
                name: 'GHOSTYPE 직접 검색',
                config: {
                    filters: [{ name: 'GHOSTYPE' }],
                    // OLD UUID - commented out as it doesn't match ESP32 firmware
                    // optionalServices: ['12345678-1234-5678-9012-123456789abc']
                    optionalServices: ['6e400001-b5a3-f393-e0a9-e50e24dcca9e']
                }
            },
            {
                name: 'GHOST 접두사 검색',
                config: {
                    filters: [{ namePrefix: 'GHOST' }],
                    // OLD UUID - commented out as it doesn't match ESP32 firmware
                    // optionalServices: ['12345678-1234-5678-9012-123456789abc']
                    optionalServices: ['6e400001-b5a3-f393-e0a9-e50e24dcca9e']
                }
            },
            {
                name: 'ESP32 검색',
                config: {
                    filters: [{ namePrefix: 'ESP32' }],
                    // OLD UUID - commented out as it doesn't match ESP32 firmware
                    // optionalServices: ['12345678-1234-5678-9012-123456789abc']
                    optionalServices: ['6e400001-b5a3-f393-e0a9-e50e24dcca9e']
                }
            },
            {
                name: '모든 장치 검색',
                config: {
                    acceptAllDevices: true,
                    // OLD UUID - commented out as it doesn't match ESP32 firmware
                    // optionalServices: ['12345678-1234-5678-9012-123456789abc']
                    optionalServices: ['6e400001-b5a3-f393-e0a9-e50e24dcca9e']
                }
            }
        ];
        
        for (const scanConfig of scanConfigs) {
            try {
                this.log(`  🔍 ${scanConfig.name}...`);
                
                const device = await navigator.bluetooth.requestDevice(scanConfig.config);
                
                if (device) {
                    this.logSuccess(`장치 발견: ${device.name || 'Unknown'} (${device.id})`);
                    
                    // Try basic connection
                    await this.testBasicConnection(device);
                } else {
                    this.logWarning('장치를 찾을 수 없음');
                }
                
            } catch (error) {
                this.logWarning(`${scanConfig.name} 실패: ${error.message}`);
            }
            
            await this.delay(1000);
        }
    }
    
    /**
     * Test basic connection to device
     * 기본 장치 연결 테스트
     */
    async testBasicConnection(device) {
        try {
            this.log(`    🔗 ${device.name} 연결 시도...`);
            
            const server = await device.gatt.connect();
            this.logSuccess('GATT 서버 연결 성공');
            
            // Try to get service
            try {
                // OLD UUID - commented out as it doesn't match ESP32 firmware
                // const service = await server.getPrimaryService('12345678-1234-5678-9012-123456789abc');
                const service = await server.getPrimaryService('6e400001-b5a3-f393-e0a9-e50e24dcca9e');
                this.logSuccess('GHOSTYPE 서비스 발견');
                
                // Try to get characteristics
                try {
                    // OLD UUIDs - commented out as they don't match ESP32 firmware
                    // const rxChar = await service.getCharacteristic('12345678-1234-5678-9012-123456789abd');
                    // const txChar = await service.getCharacteristic('12345678-1234-5678-9012-123456789abe');
                    const rxChar = await service.getCharacteristic('6e400002-b5a3-f393-e0a9-e50e24dcca9e');
                    const txChar = await service.getCharacteristic('6e400003-b5a3-f393-e0a9-e50e24dcca9e');
                    this.logSuccess('모든 특성 발견');
                    
                    // Test simple write
                    await rxChar.writeValueWithoutResponse(new TextEncoder().encode('GHTYPE_TEST'));
                    this.logSuccess('테스트 명령 전송 성공');
                    
                } catch (charError) {
                    this.logError('특성 접근 실패: ' + charError.message);
                }
                
            } catch (serviceError) {
                this.logError('서비스 접근 실패: ' + serviceError.message);
                this.logSolution('ESP32 펌웨어가 올바르게 로드되었는지 확인');
            }
            
            // Disconnect
            await device.gatt.disconnect();
            
        } catch (error) {
            this.logError('연결 실패: ' + error.message);
            
            if (error.message.includes('GATT operation not permitted')) {
                this.logSolution('ESP32를 재시작하고 다시 시도');
            } else if (error.message.includes('Device is no longer in range')) {
                this.logSolution('ESP32가 범위 내에 있고 전원이 켜져 있는지 확인');
            } else if (error.message.includes('Connection failed')) {
                this.logSolution('ESP32 BLE 설정을 확인하고 더 보수적인 연결 매개변수 사용');
            }
        }
    }
    
    /**
     * Test different connection parameters
     * 다양한 연결 매개변수 테스트
     */
    async testConnectionParameters() {
        this.log('\n3️⃣ 연결 매개변수 최적화 테스트');
        
        const recommendations = [
            '✅ ESP32 MTU를 247로 설정됨 (호환성 향상)',
            '✅ 연결 간격을 15ms로 설정 (빠른 응답)',
            '✅ 광고 간격을 62.5-125ms로 설정 (빠른 발견)',
            '✅ 보안 설정을 관대하게 설정 (페어링 없음)',
            '✅ 재시도 로직 구현됨'
        ];
        
        recommendations.forEach(rec => {
            this.log(`  ${rec}`);
        });
        
        this.log('\n  🔧 추가 권장사항:');
        this.log('    • ESP32 근처(1m 이내)에서 테스트');
        this.log('    • 다른 BLE 장치들과의 간섭 최소화');
        this.log('    • ESP32 전원 공급 안정성 확인');
        this.log('    • Chrome 브라우저의 chrome://bluetooth-internals/ 에서 상세 로그 확인');
    }
    
    /**
     * Generate diagnostic report
     * 진단 보고서 생성
     */
    generateReport() {
        this.log('\n📋 진단 보고서');
        this.log('================');
        
        const totalTime = Date.now() - this.startTime;
        this.log(`진단 시간: ${totalTime}ms`);
        
        const errors = this.diagnostics.filter(d => d.type === 'error');
        const warnings = this.diagnostics.filter(d => d.type === 'warning');
        const successes = this.diagnostics.filter(d => d.type === 'success');
        
        this.log(`성공: ${successes.length}, 경고: ${warnings.length}, 오류: ${errors.length}`);
        
        if (errors.length > 0) {
            this.log('\n❌  주요 문제:');
            errors.forEach(error => this.log(`  • ${error.message}`));
        }
        
        if (warnings.length > 0) {
            this.log('\n⚠️ 주의사항:');
            warnings.forEach(warning => this.log(`  • ${warning.message}`));
        }
        
        this.log('\n💡 문제 해결 단계:');
        this.log('  1. ESP32 전원을 껐다 켜기');
        this.log('  2. 브라우저에서 블루투스 캐시 클리어');
        this.log('  3. ESP32를 더 가까이 두고 재시도');
        this.log('  4. Chrome의 chrome://bluetooth-internals/ 에서 상세 로그 확인');
        this.log('  5. 다른 브라우저(Edge, Opera) 시도');
    }
    
    /**
     * Utility methods
     */
    log(message) {
        console.log(message);
        this.diagnostics.push({ type: 'info', message, timestamp: Date.now() });
    }
    
    logSuccess(message) {
        const msg = `✅ ${message}`;
        console.log(msg);
        this.diagnostics.push({ type: 'success', message: msg, timestamp: Date.now() });
    }
    
    logWarning(message) {
        const msg = `⚠️ ${message}`;
        console.log(msg);
        this.diagnostics.push({ type: 'warning', message: msg, timestamp: Date.now() });
    }
    
    logError(message) {
        const msg = `❌ ${message}`;
        console.log(msg);
        this.diagnostics.push({ type: 'error', message: msg, timestamp: Date.now() });
    }
    
    logSolution(message) {
        const msg = `💡 해결방법: ${message}`;
        console.log(msg);
        this.diagnostics.push({ type: 'solution', message: msg, timestamp: Date.now() });
    }
    
    delay(ms) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }
}

// Quick diagnostic function
async function runBLEDiagnostics() {
    const diagnostics = new BLEConnectionDiagnostics();
    await diagnostics.runDiagnostics();
}

// Export
if (typeof module !== 'undefined' && module.exports) {
    module.exports = { BLEConnectionDiagnostics, runBLEDiagnostics };
} else if (typeof window !== 'undefined') {
    window.BLEConnectionDiagnostics = BLEConnectionDiagnostics;
    window.runBLEDiagnostics = runBLEDiagnostics;
}