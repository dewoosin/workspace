import { PROTOCOLS, DEFAULT_CONFIG } from './constants.js';
import { analyzeText, convertHangulToJamoKeys } from './korean-converter-improved.js';
import { logger } from './logger.js';
import { MessageHistory } from './message-history.js';

export class UIController {
    constructor(bleManager) {
        this.bleManager = bleManager;
        this.countdownTimer = null;
        this.countdownPending = null;
        this.currentTypingSpeed = DEFAULT_CONFIG.TYPING_SPEED;
        this.messageHistory = new MessageHistory();
        
        this.initializeEventListeners();
        this.loadSettings();
    }

    initializeEventListeners() {
        // Message input events
        document.getElementById('messageInput').addEventListener('input', () => this.updateConversionPreview());
        document.getElementById('messageInput').addEventListener('keypress', (event) => {
            if (event.key === 'Enter') {
                event.preventDefault();
                this.startCountdown();
            }
        });
        
        // Paste events
        this.setupPasteHandlers();
        
        // Modal textarea events
        document.getElementById('messageModalTextarea').addEventListener('input', () => this.updateModalPreview());
        
        // Keyboard shortcuts
        document.addEventListener('keydown', (event) => {
            if (event.key === 'Escape') {
                if (document.getElementById('logModal').style.display === 'flex') {
                    logger.toggleModal();
                }
                if (document.getElementById('messageModal').style.display === 'flex') {
                    this.closeMessageModal();
                }
                if (document.getElementById('historyModal').style.display === 'flex') {
                    this.closeHistoryModal();
                }
            }
        });
    }

    setupPasteHandlers() {
        const pasteHandler = (event) => {
            event.preventDefault();
            
            const clipboardData = event.clipboardData || window.clipboardData;
            let pastedText = clipboardData.getData('text');
            
            // Clean the pasted text to remove any problematic characters
            pastedText = pastedText.replace(/[^\x00-\x7F\uAC00-\uD7A3\u3130-\u318F]/g, '');
            
            // Insert the cleaned text at cursor position
            const element = event.target;
            const startPos = element.selectionStart;
            const endPos = element.selectionEnd;
            const textBefore = element.value.substring(0, startPos);
            const textAfter = element.value.substring(endPos);
            
            element.value = textBefore + pastedText + textAfter;
            
            // Set cursor position after pasted text
            const newCursorPos = startPos + pastedText.length;
            element.setSelectionRange(newCursorPos, newCursorPos);
            
            // Update the preview
            if (element.id === 'messageInput') {
                this.updateConversionPreview();
            } else {
                this.updateModalPreview();
            }
        };

        document.getElementById('messageInput').addEventListener('paste', pasteHandler);
        document.getElementById('messageModalTextarea').addEventListener('paste', pasteHandler);
    }

    loadSettings() {
        const savedSpeed = localStorage.getItem('ghostype_typing_speed');
        if (savedSpeed) {
            this.currentTypingSpeed = parseInt(savedSpeed);
            document.getElementById('typingSpeed').value = savedSpeed;
        }
        
    }

    // Typing Speed Control
    updateTypingSpeed() {
        const speedSelect = document.getElementById('typingSpeed');
        this.currentTypingSpeed = parseInt(speedSelect.value);
        
        // Save to localStorage
        localStorage.setItem('ghostype_typing_speed', this.currentTypingSpeed);
        
        // Send configuration to ESP32 if connected
        if (this.bleManager.isConnected()) {
            const configData = {
                mode: "typing",
                speed_cps: this.currentTypingSpeed
            };
            const configProtocol = `${PROTOCOLS.CONFIG}${JSON.stringify(configData)}`;
            
            this.bleManager.sendData(configProtocol).then(success => {
                if (success) {
                    logger.log(`⚡ 타이핑 속도 변경: ${this.currentTypingSpeed} chars/sec`, 'success');
                }
            });
        } else {
            logger.log(`⚡ 타이핑 속도 설정: ${this.currentTypingSpeed} chars/sec (연결 후 적용)`, 'info');
        }
    }

    // Language segmentation function
    segmentTextByLanguage(text) {
        const segments = [];
        let currentSegment = '';
        let currentLanguage = null;
        
        for (let i = 0; i < text.length; i++) {
            const char = text[i];
            const code = char.charCodeAt(0);
            
            // Determine character language
            let charLanguage;
            if (code >= 0xAC00 && code <= 0xD7A3) {
                // Hangul syllables
                charLanguage = 'korean';
            } else if ((code >= 65 && code <= 90) || (code >= 97 && code <= 122) || 
                      (code >= 32 && code <= 126) || code === 10 || code === 13 || code === 9) {
                // English letters, ASCII printable chars, or special chars (space, newline, etc.)
                charLanguage = 'english';
            } else {
                // Other characters - treat as english for simplicity
                charLanguage = 'english';
            }
            
            // If language changes, save current segment and start new one
            if (currentLanguage !== null && currentLanguage !== charLanguage) {
                if (currentSegment.length > 0) {
                    segments.push({
                        text: currentSegment,
                        language: currentLanguage
                    });
                }
                currentSegment = char;
                currentLanguage = charLanguage;
            } else {
                // Continue current segment
                currentSegment += char;
                currentLanguage = charLanguage;
            }
        }
        
        // Add final segment
        if (currentSegment.length > 0) {
            segments.push({
                text: currentSegment,
                language: currentLanguage
            });
        }
        
        return segments;
    }

    // 텍스트에서 엔터키를 분리해서 프로토콜 생성
    processTextWithEnters(text, language) {
        const blocks = [];
        const parts = text.split('\n');
        
        for (let i = 0; i < parts.length; i++) {
            const part = parts[i];
            
            if (part.length > 0) {
                // 텍스트가 있으면 추가
                if (language === 'korean') {
                    const jamoKeys = convertHangulToJamoKeys(part);
                    blocks.push(`#TEXT:${jamoKeys}`);
                } else {
                    blocks.push(`#TEXT:${part}`);
                }
            }
            
            // 마지막이 아니면 엔터키 추가 (줄바꿈 표시)
            if (i < parts.length - 1) {
                blocks.push(`#CMD:ENTER`);
            }
        }
        
        return blocks;
    }

    // Generate block-based protocol
    generateBlockProtocol(segments) {
        const protocolBlocks = [];
        let convertedText = '';
        let hasKorean = false;
        
        for (const segment of segments) {
            if (segment.language === 'korean') {
                hasKorean = true;
                protocolBlocks.push(`#CMD:HANGUL`);
                
                // 엔터키가 포함된 텍스트 처리
                const textBlocks = this.processTextWithEnters(segment.text, 'korean');
                protocolBlocks.push(...textBlocks);
                
                // 변환된 텍스트는 엔터키 포함해서 저장
                const jamoKeys = convertHangulToJamoKeys(segment.text);
                convertedText += jamoKeys;
            } else {
                protocolBlocks.push(`#CMD:ENGLISH`);
                
                // 엔터키가 포함된 텍스트 처리
                const textBlocks = this.processTextWithEnters(segment.text, 'english');
                protocolBlocks.push(...textBlocks);
                
                convertedText += segment.text;
            }
        }
        
        return {
            protocol: protocolBlocks.join('\n'),
            convertedText: convertedText,
            hasKorean: hasKorean
        };
    }

    // Text Conversion
    convertTextWithProtocol(text) {
        // 디버깅: 입력 텍스트에 엔터키가 있는지 확인
        console.log('convertTextWithProtocol 입력:', JSON.stringify(text));
        console.log('엔터키 포함?', text.includes('\n'));
        
        const analysis = analyzeText(text);
        const type = analysis.type || analysis; // Handle both old and new format
        
        // For mixed language or Korean text, use block-based protocol
        if (type === 'mixed') {
            console.log('혼합 텍스트 감지 - 블록 기반 프로토콜 사용');
            
            const segments = this.segmentTextByLanguage(text);
            console.log('언어별 세그먼트:', segments);
            
            const blockResult = this.generateBlockProtocol(segments);
            console.log('생성된 프로토콜:', blockResult.protocol);
            
            return {
                original: text,
                converted: blockResult.convertedText,
                protocol: blockResult.protocol,
                type: 'mixed',
                description: '혼합 언어 블록 프로토콜'
            };
        } else if (type === 'korean') {
            // Pure Korean - use block protocol for consistency
            console.log('순수 한글 텍스트 - 블록 프로토콜 사용');
            
            const jamoKeys = convertHangulToJamoKeys(text);
            
            // 엔터키 처리
            const protocolBlocks = [`#CMD:HANGUL`];
            const textBlocks = this.processTextWithEnters(text, 'korean');
            protocolBlocks.push(...textBlocks);
            const protocol = protocolBlocks.join('\n');
            
            return {
                original: text,
                converted: jamoKeys,
                protocol: protocol,
                type: 'korean',
                description: '한글 블록 프로토콜'
            };
        } else {
            // Pure English - use block protocol for consistency
            console.log('순수 영문 텍스트 - 블록 프로토콜 사용');
            
            // 엔터키 처리
            const protocolBlocks = [`#CMD:ENGLISH`];
            const textBlocks = this.processTextWithEnters(text, 'english');
            protocolBlocks.push(...textBlocks);
            const protocol = protocolBlocks.join('\n');
            
            return {
                original: text,
                converted: text,
                protocol: protocol,
                type: 'english',
                description: '영문 블록 프로토콜'
            };
        }
    }

    // Preview Updates
    updateConversionPreview() {
        const input = document.getElementById('messageInput');
        const preview = document.getElementById('conversionPreview');
        const previewText = document.getElementById('previewText');
        const protocolInfo = document.getElementById('protocolInfo');
        const text = input.value;
        
        if (!text) {
            preview.className = 'conversion-preview';
            previewText.textContent = '메시지를 입력하면 변환 결과가 여기에 표시됩니다';
            protocolInfo.textContent = '';
            return;
        }
        
        const result = this.convertTextWithProtocol(text);
        
        // 미리보기 스타일 업데이트
        preview.className = `conversion-preview ${result.type}`;
        
        // 변환 결과 표시 (엔터키 등 특수 문자 표시)
        const displayOriginal = result.original
            .replace(/\n/g, '↵<br>')
            .replace(/\t/g, '→')
            .replace(/\r/g, '↓');
        
        const displayConverted = result.converted
            .replace(/\n/g, '↵<br>')
            .replace(/\t/g, '→')
            .replace(/\r/g, '↓');
        
        if (result.type === 'korean' || result.type === 'mixed') {
            previewText.innerHTML = `
                <strong>원본:</strong> ${displayOriginal}<br>
                <strong>자모:</strong> ${displayConverted}<br>
                <strong>설명:</strong> ${result.description}
            `;
        } else {
            previewText.innerHTML = `
                <strong>텍스트:</strong> ${displayOriginal}<br>
                <strong>설명:</strong> ${result.description}
            `;
        }
        
        protocolInfo.textContent = `전송 프로토콜: ${result.protocol}`;
    }

    updateModalPreview() {
        const textarea = document.getElementById('messageModalTextarea');
        const previewText = document.getElementById('modalPreviewText');
        const protocolInfo = document.getElementById('modalProtocolInfo');
        const text = textarea.value;
        
        if (!text) {
            previewText.textContent = '메시지를 입력하면 변환 결과가 여기에 표시됩니다';
            protocolInfo.textContent = '';
            return;
        }
        
        const result = this.convertTextWithProtocol(text);
        
        // 특수 문자 표시 처리
        const displayOriginal = result.original
            .replace(/\n/g, '↵<br>')
            .replace(/\t/g, '→')
            .replace(/\r/g, '↓');
        
        const displayConverted = result.converted
            .replace(/\n/g, '↵<br>')
            .replace(/\t/g, '→')
            .replace(/\r/g, '↓');
        
        if (result.type === 'korean' || result.type === 'mixed') {
            previewText.innerHTML = `
                <strong>원본:</strong> ${displayOriginal}<br>
                <strong>자모:</strong> ${displayConverted}<br>
                <strong>설명:</strong> ${result.description}
            `;
        } else {
            previewText.innerHTML = `
                <strong>텍스트:</strong> ${displayOriginal}<br>
                <strong>설명:</strong> ${result.description}
            `;
        }
        
        protocolInfo.textContent = `전송 프로토콜: ${result.protocol}`;
    }

    // Modal Functions
    openMessageModal() {
        const modal = document.getElementById('messageModal');
        const textarea = document.getElementById('messageModalTextarea');
        const input = document.getElementById('messageInput');
        
        // Copy current value to modal
        textarea.value = input.value;
        modal.style.display = 'flex';
        
        // Focus textarea and update preview
        textarea.focus();
        this.updateModalPreview();
    }

    closeMessageModal() {
        const modal = document.getElementById('messageModal');
        modal.style.display = 'none';
    }

    applyMessageModal() {
        const textarea = document.getElementById('messageModalTextarea');
        const input = document.getElementById('messageInput');
        
        // Copy value back to input
        input.value = textarea.value;
        this.updateConversionPreview();
        this.closeMessageModal();
    }

    // Countdown Functions
    startCountdown() {
        const input = document.getElementById('messageInput');
        const message = input.value.trim();
        
        if (!message) {
            logger.log('⚠️ 메시지를 입력하세요', 'error');
            return;
        }

        if (!this.bleManager.isConnected()) {
            logger.log('❌ 먼저 GHOSTYPE 디바이스에 연결하세요', 'error');
            return;
        }

        const result = this.convertTextWithProtocol(message);
        this.startCountdownWithResult(result, () => {
            this.bleManager.sendData(result.protocol);
            // Add to history
            this.addMessageToHistory(result.original, result.type, result.converted);
            input.value = '';
            this.updateConversionPreview();
        });
    }

    startCountdownWithResult(result, callback) {
        if (!this.bleManager.isConnected()) {
            logger.log('❌ 연결되지 않음', 'error');
            return;
        }

        const modal = document.getElementById('countdownModal');
        const numberEl = document.getElementById('countdownNumber');
        const messageEl = document.getElementById('countdownMessage');
        const originalEl = document.getElementById('originalText');
        const convertedEl = document.getElementById('convertedText');
        const protocolEl = document.getElementById('protocolText');
        
        modal.style.display = 'flex';
        originalEl.textContent = result.original;
        convertedEl.textContent = result.converted;
        protocolEl.textContent = result.protocol;
        
        let count = DEFAULT_CONFIG.COUNTDOWN_SECONDS;
        this.countdownPending = callback;
        
        const updateCountdown = () => {
            numberEl.textContent = count;
            
            if (count > 0) {
                messageEl.textContent = `${count}초 후 입력이 시작됩니다...`;
                count--;
                this.countdownTimer = setTimeout(updateCountdown, 1000);
            } else {
                messageEl.textContent = '입력 중... ⌨️';
                numberEl.textContent = '⌨️';
                numberEl.style.animation = 'none';
                
                setTimeout(() => {
                    modal.style.display = 'none';
                    if (this.countdownPending) {
                        this.countdownPending();
                        this.countdownPending = null;
                        logger.log(`⌨️ 전송: ${result.description} - "${result.original}"`, 'success');
                    }
                    numberEl.style.animation = 'pulse 1s ease-in-out infinite';
                }, 1000);
            }
        };
        
        updateCountdown();
        logger.log(`⏰ 5초 카운트다운: ${result.description}`, 'info');
    }

    cancelCountdown() {
        if (this.countdownTimer) {
            clearTimeout(this.countdownTimer);
            this.countdownTimer = null;
        }
        this.countdownPending = null;
        
        const modal = document.getElementById('countdownModal');
        modal.style.display = 'none';
        
        logger.log('❌ 입력 취소됨', 'info');
    }

    // Test Functions
    testText(text) {
        const result = this.convertTextWithProtocol(text);
        this.startCountdownWithResult(result, () => {
            this.bleManager.sendData(result.protocol);
        });
    }

    testSpecial(command) {
        const protocol = `${PROTOCOLS.SPECIAL}${command}`;
        const result = {
            original: command,
            converted: command,
            protocol: protocol,
            type: 'special',
            description: '특수 키 명령'
        };
        
        this.startCountdownWithResult(result, () => {
            this.bleManager.sendData(protocol);
        });
    }

    // History Modal Functions
    openHistoryModal() {
        const modal = document.getElementById('historyModal');
        const searchInput = document.getElementById('historySearch');
        
        modal.style.display = 'flex';
        this.updateHistoryDisplay();
        
        // Focus search input
        searchInput.focus();
        
        // Setup search event listener
        searchInput.addEventListener('input', () => this.searchHistory());
    }

    closeHistoryModal() {
        const modal = document.getElementById('historyModal');
        modal.style.display = 'none';
    }

    updateHistoryDisplay(searchQuery = '') {
        const historyList = document.getElementById('historyList');
        const historyStats = document.getElementById('historyStats');
        
        const messages = searchQuery ? 
            this.messageHistory.searchMessages(searchQuery) : 
            this.messageHistory.getMessages();
        
        const stats = this.messageHistory.getStats();
        
        // Update stats
        historyStats.innerHTML = `
            <span>Total: ${stats.total}</span>
            <span>Today: ${stats.today}</span>
            <span>This Week: ${stats.thisWeek}</span>
        `;
        
        // Update history list
        if (messages.length === 0) {
            historyList.innerHTML = '<div class="history-empty">No messages found. Start typing to build your history!</div>';
            return;
        }
        
        historyList.innerHTML = messages.map(item => `
            <div class="history-item">
                <div class="history-item-content">
                    <div class="history-item-message">${this.escapeHtml(item.message)}</div>
                    <div class="history-item-details">
                        ${this.messageHistory.formatTimestamp(item.timestamp)} • ${item.type} • ${item.preview}
                    </div>
                </div>
                <div class="history-item-actions">
                    <button class="history-item-use" onclick="useHistoryMessage('${item.id}')">Use</button>
                    <button class="history-item-delete" onclick="deleteHistoryMessage('${item.id}')">×</button>
                </div>
            </div>
        `).join('');
    }

    searchHistory() {
        const searchInput = document.getElementById('historySearch');
        const query = searchInput.value.trim();
        this.updateHistoryDisplay(query);
    }

    useHistoryMessage(messageId) {
        const message = this.messageHistory.getMessage(messageId);
        if (!message) {
            logger.log('❌ Message not found', 'error');
            return;
        }
        
        const currentInput = document.getElementById('messageInput').value.trim();
        
        if (currentInput && currentInput !== message.message) {
            // Ask for confirmation if current input is different
            if (confirm('This will replace your current message. Continue?')) {
                this.setMessageInput(message.message);
                this.closeHistoryModal();
            }
        } else {
            this.setMessageInput(message.message);
            this.closeHistoryModal();
        }
    }

    deleteHistoryMessage(messageId) {
        if (confirm('Delete this message from history?')) {
            this.messageHistory.deleteMessage(messageId);
            this.updateHistoryDisplay();
            logger.log('🗑️ Message deleted from history', 'info');
        }
    }

    clearHistory() {
        if (confirm('Delete all message history? This cannot be undone.')) {
            this.messageHistory.clearHistory();
            this.updateHistoryDisplay();
            logger.log('🗑️ All history cleared', 'info');
        }
    }

    exportHistory() {
        const jsonData = this.messageHistory.exportHistory();
        const blob = new Blob([jsonData], { type: 'application/json' });
        const url = URL.createObjectURL(blob);
        
        const a = document.createElement('a');
        a.href = url;
        a.download = `ghostype-history-${new Date().toISOString().split('T')[0]}.json`;
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);
        
        logger.log('📁 History exported successfully', 'success');
    }

    setMessageInput(message) {
        const input = document.getElementById('messageInput');
        input.value = message;
        this.updateConversionPreview();
    }

    escapeHtml(text) {
        const div = document.createElement('div');
        div.textContent = text;
        return div.innerHTML;
    }

    // Add message to history when sent
    addMessageToHistory(message, type, convertedText = '') {
        this.messageHistory.addMessage(message, type, convertedText);
    }
}