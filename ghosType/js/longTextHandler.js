/**
 * Long Text Handler for GHOSTYPE
 * 긴 텍스트를 청크로 분할하여 전송하는 핸들러
 */

class LongTextHandler {
    constructor(bleInterface) {
        this.ble = bleInterface;
        this.preprocessor = new HangulPreprocessor();
        
        // 청크 설정
        this.CHUNK_SIZE = 100;        // 한 청크당 글자수
        this.CHUNK_DELAY = 500;      // 청크 간 지연 (ms)
        this.MAX_QUEUE_SIZE = 50;    // 최대 대기열 크기
        
        // 전송 상태
        this.isTransmitting = false;
        this.currentQueue = [];
        this.progress = {
            total: 0,
            sent: 0,
            typed: 0
        };
        
        // 콜백 함수
        this.onProgress = null;
        this.onComplete = null;
        this.onError = null;
    }
    
    /**
     * 긴 텍스트를 청크로 분할
     * @param {string} text - 원본 텍스트
     * @returns {Array} 청크 배열
     */
    splitIntoChunks(text) {
        const chunks = [];
        let currentChunk = '';
        let currentMode = null;
        
        for (let i = 0; i < text.length; i++) {
            const char = text[i];
            const isKorean = this.preprocessor.isHangul(char);
            const charMode = isKorean ? 'korean' : 'english';
            
            // 모드가 바뀌거나 청크 크기 초과 시 새 청크 생성
            if ((currentMode && currentMode !== charMode) || 
                currentChunk.length >= this.CHUNK_SIZE) {
                
                if (currentChunk.length > 0) {
                    chunks.push({
                        text: currentChunk,
                        mode: currentMode,
                        index: chunks.length
                    });
                    currentChunk = '';
                }
            }
            
            currentMode = charMode;
            currentChunk += char;
        }
        
        // 마지막 청크 추가
        if (currentChunk.length > 0) {
            chunks.push({
                text: currentChunk,
                mode: currentMode,
                index: chunks.length
            });
        }
        
        return chunks;
    }
    
    /**
     * 긴 텍스트 전송 시작
     * @param {string} text - 전송할 텍스트
     * @param {Object} options - 전송 옵션
     */
    async sendLongText(text, options = {}) {
        if (this.isTransmitting) {
            throw new Error('이미 전송 중입니다');
        }
        
        try {
            this.isTransmitting = true;
            
            // 진행 상황 초기화
            this.progress = {
                total: text.length,
                sent: 0,
                typed: 0
            };
            
            // 텍스트를 청크로 분할
            const chunks = this.splitIntoChunks(text);
            console.log(`텍스트를 ${chunks.length}개 청크로 분할`);
            
            // 각 청크 전송
            for (let i = 0; i < chunks.length; i++) {
                const chunk = chunks[i];
                
                // 진행 상황 업데이트
                this.updateProgress(chunk.text.length, 'sending');
                
                // 청크 전송
                await this.sendChunk(chunk, options);
                
                // 청크 간 지연 (ESP32 처리 시간 확보)
                if (i < chunks.length - 1) {
                    await this.delay(this.CHUNK_DELAY);
                }
                
                // 진행 상황 업데이트
                this.updateProgress(chunk.text.length, 'typed');
            }
            
            // 완료 콜백
            if (this.onComplete) {
                this.onComplete(this.progress);
            }
            
        } catch (error) {
            if (this.onError) {
                this.onError(error);
            }
            throw error;
        } finally {
            this.isTransmitting = false;
        }
    }
    
    /**
     * 단일 청크 전송
     * @param {Object} chunk - 청크 데이터
     * @param {Object} options - 전송 옵션
     */
    async sendChunk(chunk, options) {
        console.log(`청크 ${chunk.index + 1} 전송: ${chunk.mode} (${chunk.text.length}자)`);
        
        // 한글 텍스트는 QWERTY로 변환
        let processedText = chunk.text;
        if (chunk.mode === 'korean') {
            processedText = this.preprocessor.hangulToQwerty(chunk.text);
        }
        
        // 이전 청크와 모드가 다르면 언어 전환
        if (chunk.index > 0 && this.needsModeSwitch(chunk)) {
            await this.ble.sendCommand('GHTYPE_SPE:haneng');
            await this.delay(200);
        }
        
        // JSON 페이로드 생성 및 전송
        const payload = {
            text: processedText,
            speed_cps: options.speed_cps || 10,
            interval_ms: options.interval_ms || 100,
            chunk_info: {
                index: chunk.index,
                total: this.progress.total,
                mode: chunk.mode
            }
        };
        
        await this.ble.sendJSON(payload);
    }
    
    /**
     * 모드 전환 필요 여부 확인
     * @param {Object} chunk - 현재 청크
     * @returns {boolean} 전환 필요 여부
     */
    needsModeSwitch(chunk) {
        // 이전 청크 정보를 통해 모드 전환 필요성 판단
        // 실제 구현에서는 이전 모드를 추적해야 함
        return true;
    }
    
    /**
     * 진행 상황 업데이트
     * @param {number} count - 처리된 글자수
     * @param {string} type - 'sending' 또는 'typed'
     */
    updateProgress(count, type) {
        if (type === 'sending') {
            this.progress.sent += count;
        } else if (type === 'typed') {
            this.progress.typed += count;
        }
        
        // 진행률 계산
        const percentage = Math.round((this.progress.typed / this.progress.total) * 100);
        
        // 콜백 호출
        if (this.onProgress) {
            this.onProgress({
                ...this.progress,
                percentage
            });
        }
    }
    
    /**
     * 지연 함수
     * @param {number} ms - 지연 시간
     */
    delay(ms) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }
    
    /**
     * 전송 중단
     */
    abort() {
        this.isTransmitting = false;
        this.currentQueue = [];
        console.log('텍스트 전송이 중단되었습니다');
    }
    
    /**
     * 예상 소요 시간 계산
     * @param {number} textLength - 텍스트 길이
     * @param {number} cps - 초당 문자수
     * @returns {Object} 예상 시간 정보
     */
    estimateTime(textLength, cps = 10) {
        const typingTime = textLength / cps;
        const chunkCount = Math.ceil(textLength / this.CHUNK_SIZE);
        const delayTime = (chunkCount - 1) * (this.CHUNK_DELAY / 1000);
        const totalTime = typingTime + delayTime;
        
        return {
            typingTime: Math.round(typingTime),
            delayTime: Math.round(delayTime),
            totalTime: Math.round(totalTime),
            formatted: this.formatTime(totalTime)
        };
    }
    
    /**
     * 시간 포맷팅
     * @param {number} seconds - 초
     * @returns {string} 포맷된 시간
     */
    formatTime(seconds) {
        const hours = Math.floor(seconds / 3600);
        const minutes = Math.floor((seconds % 3600) / 60);
        const secs = seconds % 60;
        
        if (hours > 0) {
            return `${hours}시간 ${minutes}분 ${secs}초`;
        } else if (minutes > 0) {
            return `${minutes}분 ${secs}초`;
        } else {
            return `${secs}초`;
        }
    }
}

// Export
if (typeof module !== 'undefined' && module.exports) {
    module.exports = LongTextHandler;
} else if (typeof window !== 'undefined') {
    window.LongTextHandler = LongTextHandler;
}