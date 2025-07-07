// Logger Module
class Logger {
    constructor() {
        this.modalVisible = false;
    }

    log(message, type = 'info') {
        const log = document.getElementById('logModalBody');
        const entry = document.createElement('div');
        entry.className = `log-entry log-${type}`;
        entry.innerHTML = `[${new Date().toLocaleTimeString()}] ${message}`;
        log.appendChild(entry);
        log.scrollTop = log.scrollHeight;
    }

    toggleModal() {
        const modal = document.getElementById('logModal');
        this.modalVisible = !this.modalVisible;
        modal.style.display = this.modalVisible ? 'flex' : 'none';
    }
}

export const logger = new Logger();