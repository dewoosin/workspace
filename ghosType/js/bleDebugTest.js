/**
 * BLE Debug Test - ESP32 간단한 연결 테스트
 * Simple connection test for ESP32
 */

async function testBLEConnection() {
    console.log('🧪 BLE 연결 테스트 시작...');
    
    try {
        // 1. 블루투스 지원 확인
        if (!navigator.bluetooth) {
            throw new Error('이 브라우저는 Web Bluetooth를 지원하지 않습니다.');
        }
        
        console.log('✅ Web Bluetooth 지원됨');
        
        // 2. 블루투스 가용성 확인
        const available = await navigator.bluetooth.getAvailability();
        if (!available) {
            throw new Error('블루투스가 비활성화되어 있습니다.');
        }
        
        console.log('✅ 블루투스 활성화됨');
        
        // 3. 장치 검색 (여러 필터 시도)
        console.log('🔍 ESP32 장치 검색 중...');
        
        const device = await navigator.bluetooth.requestDevice({
            filters: [
                { name: 'ESP32' },
                { namePrefix: 'ESP32' }
            ],
            optionalServices: ['12345678-1234-5678-9012-123456789abc']
        });
        
        console.log(`🎯 장치 발견: ${device.name} (ID: ${device.id})`);
        
        // 4. GATT 연결
        console.log('🔗 GATT 서버 연결 중...');
        const server = await device.gatt.connect();
        console.log('✅ GATT 서버 연결 성공');
        
        // 5. 서비스 접근
        console.log('🛠️ 서비스 탐색 중...');
        const service = await server.getPrimaryService('12345678-1234-5678-9012-123456789abc');
        console.log('✅ 서비스 발견');
        
        // 6. 특성 접근
        console.log('📡 특성 설정 중...');
        const rxChar = await service.getCharacteristic('12345678-1234-5678-9012-123456789abd');
        const txChar = await service.getCharacteristic('12345678-1234-5678-9012-123456789abe');
        console.log('✅ 특성 설정 완료');
        
        // 7. 알림 설정
        console.log('🔔 알림 설정 중...');
        await txChar.startNotifications();
        txChar.addEventListener('characteristicvaluechanged', (event) => {
            const value = new TextDecoder().decode(event.target.value);
            console.log('📨 ESP32 응답:', value);
        });
        console.log('✅ 알림 설정 완료');
        
        // 8. 테스트 데이터 전송
        console.log('📤 테스트 데이터 전송 중...');
        const testData = 'Hello ESP32!';
        await rxChar.writeValueWithoutResponse(new TextEncoder().encode(testData));
        console.log(`✅ 데이터 전송 완료: "${testData}"`);
        
        // 9. 성공 메시지
        console.log('🎉 BLE 연결 테스트 성공!');
        console.log('💡 ESP32 LED가 켜져 있는지 확인하세요 (연결됨 표시)');
        
        return {
            success: true,
            device: device,
            server: server,
            service: service,
            rxChar: rxChar,
            txChar: txChar
        };
        
    } catch (error) {
        console.error('❌ BLE 연결 테스트 실패:', error);
        
        // 상세 오류 분석
        if (error.message.includes('User cancelled')) {
            console.log('💡 해결방법: 장치 선택 대화상자에서 ESP32를 선택하세요');
        } else if (error.message.includes('GATT')) {
            console.log('💡 해결방법: ESP32를 재시작하고 다시 시도하세요');
        } else if (error.message.includes('Service not found')) {
            console.log('💡 해결방법: ESP32 펌웨어가 올바르게 업로드되었는지 확인하세요');
        } else if (error.message.includes('Device is no longer in range')) {
            console.log('💡 해결방법: ESP32를 더 가까이 두고 시도하세요');
        }
        
        return {
            success: false,
            error: error.message
        };
    }
}

// 자동 재시도 함수
async function testWithRetry(maxRetries = 3) {
    for (let attempt = 1; attempt <= maxRetries; attempt++) {
        console.log(`\n🔄 시도 ${attempt}/${maxRetries}`);
        
        const result = await testBLEConnection();
        
        if (result.success) {
            return result;
        }
        
        if (attempt < maxRetries) {
            console.log('⏳ 3초 후 재시도...');
            await new Promise(resolve => setTimeout(resolve, 3000));
        }
    }
    
    console.log('💥 모든 시도 실패');
    return { success: false };
}

// 단순 스캔 테스트
async function simpleScanTest() {
    console.log('🔍 단순 장치 스캔 테스트...');
    
    try {
        const device = await navigator.bluetooth.requestDevice({
            acceptAllDevices: true,
            optionalServices: ['12345678-1234-5678-9012-123456789abc']
        });
        
        console.log('📱 발견된 장치들:');
        console.log(`  이름: ${device.name || '(이름 없음)'}`);
        console.log(`  ID: ${device.id}`);
        
        return device;
        
    } catch (error) {
        console.error('❌ 스캔 실패:', error);
        return null;
    }
}

// 글로벌 함수로 등록
if (typeof window !== 'undefined') {
    window.testBLEConnection = testBLEConnection;
    window.testWithRetry = testWithRetry;
    window.simpleScanTest = simpleScanTest;
}