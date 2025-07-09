import { BLEManager } from './ble-manager.js';
import { UIController } from './ui-controller.js';
import { logger } from './logger.js';
import { PROTOCOLS } from './constants.js';

// Import improved Korean converter for testing
import { runDiagnostics } from './korean-converter-improved.js';

// Global instances
let bleManager = null;
let uiController = null;

// Initialize application
function initialize() {
    // Make logger globally available
    window.logger = logger;
    
    // Check Web Bluetooth support
    if (!navigator.bluetooth) {
        logger.log('❌ 이 브라우저는 Web Bluetooth를 지원하지 않습니다', 'error');
        updateStatus('🚫 Web Bluetooth 미지원', 'disconnected');
    }

    // Create instances
    bleManager = new BLEManager();
    uiController = new UIController(bleManager);

    // Initial state
    bleManager.updateUI(false);
    uiController.updateConversionPreview();
    logger.log('💡 연결 후 한글/영문을 자유롭게 입력해보세요!', 'info');

    // Close modals when clicking outside
    setupModalClickHandlers();
    
    // Run Korean converter diagnostics in development
    if (window.location.hostname === 'localhost' || window.location.hostname === '127.0.0.1') {
        console.log('🔧 Running Korean converter diagnostics...');
        runDiagnostics();
    }
}

// Update status display
function updateStatus(message, type) {
    const status = document.getElementById('status');
    status.textContent = message;
    status.className = `status ${type}`;
}

// Setup modal click handlers
function setupModalClickHandlers() {
    document.getElementById('logModal').addEventListener('click', function(event) {
        if (event.target === this) {
            logger.toggleModal();
        }
    });

    document.getElementById('messageModal').addEventListener('click', function(event) {
        if (event.target === this) {
            uiController.closeMessageModal();
        }
    });

    document.getElementById('countdownModal').addEventListener('click', function(event) {
        if (event.target === this) {
            uiController.cancelCountdown();
        }
    });

    document.getElementById('historyModal').addEventListener('click', function(event) {
        if (event.target === this) {
            uiController.closeHistoryModal();
        }
    });
}

// Global functions for HTML onclick handlers
window.connectDevice = async function() {
    const connected = await bleManager.connect();
    
    if (connected) {
        // 자동 전송 제거 - 사용자가 필요할 때만 전송
    }
};

window.disconnectDevice = async function() {
    await bleManager.disconnect();
};

window.toggleLogModal = function() {
    logger.toggleModal();
};

window.openMessageModal = function() {
    uiController.openMessageModal();
};

window.closeMessageModal = function() {
    uiController.closeMessageModal();
};

window.applyMessageModal = function() {
    uiController.applyMessageModal();
};

window.startCountdown = function() {
    uiController.startCountdown();
};

window.cancelCountdown = function() {
    uiController.cancelCountdown();
};

window.testText = function(text) {
    uiController.testText(text);
};

window.testSpecial = function(command) {
    uiController.testSpecial(command);
};

window.updateTypingSpeed = function() {
    uiController.updateTypingSpeed();
};

window.openHistoryModal = function() {
    uiController.openHistoryModal();
};

window.closeHistoryModal = function() {
    uiController.closeHistoryModal();
};

window.useHistoryMessage = function(messageId) {
    uiController.useHistoryMessage(messageId);
};

window.deleteHistoryMessage = function(messageId) {
    uiController.deleteHistoryMessage(messageId);
};

window.clearHistory = function() {
    uiController.clearHistory();
};

window.exportHistory = function() {
    uiController.exportHistory();
};

// Initialize when DOM is ready
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', initialize);
} else {
    initialize();
}