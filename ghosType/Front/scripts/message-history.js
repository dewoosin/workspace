/**
 * Message History Manager
 * Handles storage, retrieval, and management of sent messages
 */

const STORAGE_KEY = 'ghostype_message_history';
const MAX_HISTORY_ITEMS = 50; // Limit to prevent excessive storage

export class MessageHistory {
    constructor() {
        this.messages = [];
        this.loadFromStorage();
    }

    /**
     * Add a new message to history
     * @param {string} message - The message text
     * @param {string} type - Message type (korean, english, mixed)
     * @param {string} convertedText - The converted jamo keys
     */
    addMessage(message, type = 'unknown', convertedText = '') {
        if (!message || typeof message !== 'string' || message.trim().length === 0) {
            return;
        }

        const historyItem = {
            id: Date.now() + Math.random(), // Unique ID
            message: message.trim(),
            type: type,
            convertedText: convertedText,
            timestamp: new Date().toISOString(),
            preview: this.generatePreview(message.trim())
        };

        // Remove duplicate messages (same text)
        this.messages = this.messages.filter(item => item.message !== historyItem.message);
        
        // Add to beginning of array (most recent first)
        this.messages.unshift(historyItem);
        
        // Limit array size
        if (this.messages.length > MAX_HISTORY_ITEMS) {
            this.messages = this.messages.slice(0, MAX_HISTORY_ITEMS);
        }
        
        this.saveToStorage();
        return historyItem;
    }

    /**
     * Get all messages in reverse chronological order
     * @returns {Array} Array of message objects
     */
    getMessages() {
        return [...this.messages]; // Return copy to prevent mutation
    }

    /**
     * Get a specific message by ID
     * @param {string|number} id - Message ID
     * @returns {Object|null} Message object or null
     */
    getMessage(id) {
        return this.messages.find(item => item.id == id) || null;
    }

    /**
     * Delete a message from history
     * @param {string|number} id - Message ID
     * @returns {boolean} Success status
     */
    deleteMessage(id) {
        const initialLength = this.messages.length;
        this.messages = this.messages.filter(item => item.id != id);
        const deleted = this.messages.length < initialLength;
        
        if (deleted) {
            this.saveToStorage();
        }
        
        return deleted;
    }

    /**
     * Clear all message history
     */
    clearHistory() {
        this.messages = [];
        this.saveToStorage();
    }

    /**
     * Search messages by text content
     * @param {string} query - Search query
     * @returns {Array} Matching messages
     */
    searchMessages(query) {
        if (!query || query.trim().length === 0) {
            return this.getMessages();
        }
        
        const searchTerm = query.toLowerCase().trim();
        return this.messages.filter(item => 
            item.message.toLowerCase().includes(searchTerm) ||
            item.convertedText.toLowerCase().includes(searchTerm)
        );
    }

    /**
     * Get history statistics
     * @returns {Object} Statistics object
     */
    getStats() {
        const now = new Date();
        const today = new Date(now.getFullYear(), now.getMonth(), now.getDate());
        const week = new Date(today.getTime() - 7 * 24 * 60 * 60 * 1000);
        
        const stats = {
            total: this.messages.length,
            today: 0,
            thisWeek: 0,
            byType: { korean: 0, english: 0, mixed: 0, unknown: 0 }
        };
        
        this.messages.forEach(item => {
            const messageDate = new Date(item.timestamp);
            
            if (messageDate >= today) {
                stats.today++;
            }
            if (messageDate >= week) {
                stats.thisWeek++;
            }
            
            stats.byType[item.type] = (stats.byType[item.type] || 0) + 1;
        });
        
        return stats;
    }

    /**
     * Generate preview text (first 15 characters)
     * @param {string} message - Full message
     * @returns {string} Preview text
     */
    generatePreview(message) {
        if (message.length <= 15) {
            return message;
        }
        return message.substring(0, 15) + '...';
    }

    /**
     * Format timestamp for display
     * @param {string} isoString - ISO timestamp string
     * @returns {string} Formatted timestamp
     */
    formatTimestamp(isoString) {
        const date = new Date(isoString);
        const now = new Date();
        const diffMs = now - date;
        const diffMins = Math.floor(diffMs / 60000);
        const diffHours = Math.floor(diffMs / 3600000);
        const diffDays = Math.floor(diffMs / 86400000);

        if (diffMins < 1) {
            return 'Just now';
        } else if (diffMins < 60) {
            return `${diffMins}m ago`;
        } else if (diffHours < 24) {
            return `${diffHours}h ago`;
        } else if (diffDays < 7) {
            return `${diffDays}d ago`;
        } else {
            return date.toLocaleDateString();
        }
    }

    /**
     * Load history from localStorage
     */
    loadFromStorage() {
        try {
            const stored = localStorage.getItem(STORAGE_KEY);
            if (stored) {
                const data = JSON.parse(stored);
                if (Array.isArray(data)) {
                    this.messages = data;
                    // Validate and clean up data
                    this.messages = this.messages
                        .filter(item => item.message && item.timestamp)
                        .slice(0, MAX_HISTORY_ITEMS);
                }
            }
        } catch (error) {
            console.error('Failed to load message history from storage:', error);
            this.messages = [];
        }
    }

    /**
     * Save history to localStorage
     */
    saveToStorage() {
        try {
            localStorage.setItem(STORAGE_KEY, JSON.stringify(this.messages));
        } catch (error) {
            console.error('Failed to save message history to storage:', error);
        }
    }

    /**
     * Export history as JSON
     * @returns {string} JSON string of history
     */
    exportHistory() {
        return JSON.stringify({
            exportDate: new Date().toISOString(),
            version: '1.0',
            messages: this.messages
        }, null, 2);
    }

    /**
     * Import history from JSON
     * @param {string} jsonString - JSON string to import
     * @param {boolean} merge - Whether to merge with existing or replace
     * @returns {boolean} Success status
     */
    importHistory(jsonString, merge = false) {
        try {
            const data = JSON.parse(jsonString);
            if (data.messages && Array.isArray(data.messages)) {
                if (merge) {
                    // Merge and deduplicate
                    const existingMessages = new Set(this.messages.map(m => m.message));
                    const newMessages = data.messages.filter(m => !existingMessages.has(m.message));
                    this.messages = [...this.messages, ...newMessages];
                } else {
                    // Replace
                    this.messages = data.messages;
                }
                
                // Clean up and limit
                this.messages = this.messages
                    .filter(item => item.message && item.timestamp)
                    .slice(0, MAX_HISTORY_ITEMS);
                
                this.saveToStorage();
                return true;
            }
        } catch (error) {
            console.error('Failed to import history:', error);
        }
        return false;
    }
}