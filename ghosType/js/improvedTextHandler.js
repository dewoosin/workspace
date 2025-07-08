/**
 * Improved Text Handler with Sliding Window and Auto-cleanup
 * 슬라이딩 윈도우와 자동 정리를 적용한 개선된 텍스트 핸들러
 */

class ImprovedTextHandler {
    constructor(bleInterface) {
        this.ble = bleInterface;
        this.preprocessor = new HangulPreprocessor();
        
        // 슬라이딩 윈도우 설정
        this.WINDOW_SIZE = 5;          // 동시 처리 청크 수
        this.CHUNK_SIZE = 100;         // 청크당 글자수
        this.ACK_TIMEOUT = 5000;       // ACK 타임아웃 (5초)
        this.AUTO_CLEANUP_DELAY = 5000; // 자동 삭제 지연 (5초)
        
        // 전송 상태 관리
        this.chunks = new Map();       // 청크 ID -> 청크 데이터
        this.activeWindow = [];        // 현재 활성 윈도우
        this.completedChunks = new Set(); // 완료된 청크 ID
        this.failedChunks = new Map();   // 실패한 청크 (재시도용)
        
        // 통계
        this.stats = {
            totalChunks: 0,
            sentChunks: 0,
            typedChunks: 0,
            failedChunks: 0,
            retries: 0
        };
    }
    
    /**
     * 개선된 대용량 텍스트 전송
     * @param {string} text - 전송할 텍스트
     * @param {Object} options - 전송 옵션
     */
    async sendLongTextImproved(text, options = {}) {
        console.log(`📤 텍스트 전송 시작: ${text.length}자`);
        
        // 1. 텍스트를 청크로 분할
        const chunks = this.createChunksWithMetadata(text);
        this.stats.totalChunks = chunks.length;
        
        // 2. ESP32에 전송 시작 알림
        await this.notifyTransmissionStart(chunks.length);
        
        // 3. 슬라이딩 윈도우 방식으로 전송
        let chunkIndex = 0;
        
        while (chunkIndex < chunks.length || this.activeWindow.length > 0) {
            // 윈도우 채우기
            while (this.activeWindow.length < this.WINDOW_SIZE && chunkIndex < chunks.length) {
                const chunk = chunks[chunkIndex++];
                this.activeWindow.push(chunk);
                this.sendChunkWithAck(chunk, options);
            }
            
            // ACK 대기 및 윈도우 업데이트
            await this.updateWindow();
            
            // 진행률 보고
            this.reportProgress();
        }
        
        // 4. 전송 완료
        await this.notifyTransmissionComplete();
        console.log(`✅ 전송 완료: ${this.stats.typedChunks}/${this.stats.totalChunks} 청크`);
    }
    
    /**
     * 메타데이터가 포함된 청크 생성
     * @param {string} text - 원본 텍스트
     * @returns {Array} 청크 배열
     */
    createChunksWithMetadata(text) {
        const chunks = [];
        let position = 0;
        
        while (position < text.length) {
            const chunkText = text.slice(position, position + this.CHUNK_SIZE);
            const chunkId = this.generateChunkId();
            
            const chunk = {
                id: chunkId,
                sequence: chunks.length,
                text: chunkText,
                checksum: this.calculateChecksum(chunkText),
                retryCount: 0,
                status: 'pending',
                timestamp: Date.now()
            };
            
            chunks.push(chunk);
            this.chunks.set(chunkId, chunk);
            
            position += this.CHUNK_SIZE;
        }
        
        return chunks;
    }
    
    /**
     * ACK 기반 청크 전송
     * @param {Object} chunk - 청크 데이터
     * @param {Object} options - 전송 옵션
     */
    async sendChunkWithAck(chunk, options) {
        try {
            chunk.status = 'sending';
            chunk.sentAt = Date.now();
            
            // 한글 처리
            let processedText = chunk.text;
            if (this.containsKorean(chunk.text)) {
                processedText = this.preprocessor.hangulToQwerty(chunk.text);
            }
            
            // 페이로드 생성
            const payload = {
                chunk_id: chunk.id,
                sequence: chunk.sequence,
                text: processedText,
                checksum: chunk.checksum,
                speed_cps: options.speed_cps || 10,
                interval_ms: options.interval_ms || 100
            };
            
            // BLE 전송
            await this.ble.sendJSON(payload);
            this.stats.sentChunks++;
            
            // ACK 타임아웃 설정
            chunk.timeoutId = setTimeout(() => {
                this.handleChunkTimeout(chunk);
            }, this.ACK_TIMEOUT);
            
        } catch (error) {
            console.error(`청크 ${chunk.id} 전송 실패:`, error);
            this.handleChunkFailure(chunk);
        }
    }
    
    /**
     * 윈도우 업데이트 (ACK 수신 대기)
     */
    async updateWindow() {
        // BLE 알림 확인 (ESP32에서 ACK 수신)
        const notifications = await this.ble.checkNotifications();
        
        for (const notification of notifications) {
            if (notification.type === 'CHUNK_ACK') {
                this.handleChunkAck(notification.chunk_id);
            } else if (notification.type === 'CHUNK_ERROR') {
                this.handleChunkError(notification.chunk_id, notification.error);
            }
        }
        
        // 타임아웃된 청크 제거
        this.activeWindow = this.activeWindow.filter(chunk => {
            return chunk.status !== 'timeout' && chunk.status !== 'completed';
        });
        
        // 100ms 대기
        await this.delay(100);
    }
    
    /**
     * 청크 ACK 처리
     * @param {string} chunkId - 청크 ID
     */
    handleChunkAck(chunkId) {
        const chunk = this.chunks.get(chunkId);
        if (!chunk) return;
        
        // 타임아웃 취소
        if (chunk.timeoutId) {
            clearTimeout(chunk.timeoutId);
        }
        
        // 상태 업데이트
        chunk.status = 'completed';
        chunk.completedAt = Date.now();
        this.completedChunks.add(chunkId);
        this.stats.typedChunks++;
        
        console.log(`✅ 청크 ${chunk.sequence} 완료 (${chunk.completedAt - chunk.sentAt}ms)`);
        
        // 자동 정리 예약
        this.scheduleChunkCleanup(chunkId);
    }
    
    /**
     * 청크 타임아웃 처리
     * @param {Object} chunk - 청크 데이터
     */
    handleChunkTimeout(chunk) {
        console.warn(`⏱️ 청크 ${chunk.sequence} 타임아웃`);
        
        chunk.status = 'timeout';
        chunk.retryCount++;
        
        if (chunk.retryCount < 3) {
            // 재시도
            console.log(`🔄 청크 ${chunk.sequence} 재시도 (${chunk.retryCount}/3)`);
            this.failedChunks.set(chunk.id, chunk);
            this.stats.retries++;
            
            // 재전송 예약
            setTimeout(() => {
                chunk.status = 'pending';
                this.activeWindow.push(chunk);
            }, 1000);
        } else {
            // 최종 실패
            chunk.status = 'failed';
            this.stats.failedChunks++;
            console.error(`❌ 청크 ${chunk.sequence} 최종 실패`);
        }
    }
    
    /**
     * 청크 자동 정리
     * @param {string} chunkId - 청크 ID
     */
    scheduleChunkCleanup(chunkId) {
        setTimeout(() => {
            const chunk = this.chunks.get(chunkId);
            if (chunk && chunk.status === 'completed') {
                // 메모리에서 청크 삭제
                this.chunks.delete(chunkId);
                console.log(`🗑️ 청크 ${chunk.sequence} 메모리에서 삭제됨`);
            }
        }, this.AUTO_CLEANUP_DELAY);
    }
    
    /**
     * 진행률 보고
     */
    reportProgress() {
        const progress = {
            total: this.stats.totalChunks,
            sent: this.stats.sentChunks,
            completed: this.stats.typedChunks,
            failed: this.stats.failedChunks,
            retries: this.stats.retries,
            percentage: Math.round((this.stats.typedChunks / this.stats.totalChunks) * 100),
            memoryUsage: this.chunks.size
        };
        
        // 콜백 호출
        if (this.onProgress) {
            this.onProgress(progress);
        }
    }
    
    /**
     * 체크섬 계산
     * @param {string} text - 텍스트
     * @returns {string} 체크섬
     */
    calculateChecksum(text) {
        let hash = 0;
        for (let i = 0; i < text.length; i++) {
            const char = text.charCodeAt(i);
            hash = ((hash << 5) - hash) + char;
            hash = hash & hash; // Convert to 32-bit integer
        }
        return Math.abs(hash).toString(16);
    }
    
    /**
     * 청크 ID 생성
     * @returns {string} 고유 ID
     */
    generateChunkId() {
        return Date.now().toString(36) + Math.random().toString(36).substr(2, 9);
    }
    
    /**
     * 한글 포함 여부 확인
     * @param {string} text - 텍스트
     * @returns {boolean} 한글 포함 여부
     */
    containsKorean(text) {
        return /[가-힣]/.test(text);
    }
    
    /**
     * 전송 시작 알림
     * @param {number} totalChunks - 전체 청크 수
     */
    async notifyTransmissionStart(totalChunks) {
        await this.ble.sendCommand(`GHTYPE_START:${totalChunks}`);
    }
    
    /**
     * 전송 완료 알림
     */
    async notifyTransmissionComplete() {
        await this.ble.sendCommand('GHTYPE_END');
    }
    
    /**
     * 지연 함수
     * @param {number} ms - 밀리초
     */
    delay(ms) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }
}

// Export
if (typeof module !== 'undefined' && module.exports) {
    module.exports = ImprovedTextHandler;
} else if (typeof window !== 'undefined') {
    window.ImprovedTextHandler = ImprovedTextHandler;
}