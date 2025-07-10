/**
 * Language-Switching Protocol Module
 * 
 * This module implements a structured command protocol for switching between
 * Korean and English input modes while maintaining the existing Korean-to-QWERTY
 * conversion system.
 * 
 * Protocol Format:
 * #CMD:HANGUL    - Switch to Korean input mode
 * #CMD:ENGLISH   - Switch to English input mode  
 * #TEXT:{content} - Type the specified content
 * 
 * Example:
 * Input: "안녕Hellow 난 jason이야!"
 * Output:
 *   #CMD:HANGUL
 *   #TEXT:dkssud
 *   #CMD:ENGLISH
 *   #TEXT:Hellow 
 *   #CMD:HANGUL
 *   #TEXT:sk 
 *   #CMD:ENGLISH
 *   #TEXT:jason
 *   #CMD:HANGUL
 *   #TEXT:dlwnd!
 */

import { convertHangulToJamoKeys, analyzeText } from './korean-converter-improved.js';

// Command constants
export const PROTOCOL_COMMANDS = {
    HANGUL: '#CMD:HANGUL',
    ENGLISH: '#CMD:ENGLISH',
    TEXT_PREFIX: '#TEXT:',
    // Future extensibility
    TAB: '#CMD:TAB',
    ENTER: '#CMD:ENTER',
    SHIFT: '#CMD:SHIFT',
    CTRL: '#CMD:CTRL',
    ALT: '#CMD:ALT'
};

// Character type detection
export function detectCharacterLanguage(char) {
    const code = char.charCodeAt(0);
    
    // Korean Hangul syllables (완성형 한글)
    if (code >= 0xAC00 && code <= 0xD7A3) {
        return 'korean';
    }
    
    // Korean Jamo characters (자모)
    if ((code >= 0x1100 && code <= 0x11FF) || // Hangul Jamo
        (code >= 0x3130 && code <= 0x318F) || // Hangul Compatibility Jamo
        (code >= 0xA960 && code <= 0xA97F) || // Hangul Jamo Extended-A
        (code >= 0xD7B0 && code <= 0xD7FF)) { // Hangul Jamo Extended-B
        return 'korean';
    }
    
    // English letters
    if ((code >= 65 && code <= 90) || (code >= 97 && code <= 122)) {
        return 'english';
    }
    
    // Numbers and common symbols - treat as English context
    if (code >= 32 && code <= 126) {
        return 'english';
    }
    
    // Control characters (newline, tab, etc.) - neutral
    if (code < 32) {
        return 'control';
    }
    
    // Default to English for unknown characters
    return 'english';
}

// Segment text by language blocks
export function segmentTextByLanguage(text) {
    if (!text || typeof text !== 'string') {
        return [];
    }
    
    const segments = [];
    let currentSegment = null;
    
    for (let i = 0; i < text.length; i++) {
        const char = text[i];
        const language = detectCharacterLanguage(char);
        
        // Handle control characters specially
        if (language === 'control') {
            // Finalize current segment if any
            if (currentSegment && currentSegment.text.length > 0) {
                segments.push({ ...currentSegment });
            }
            
            // Add control character as separate segment
            segments.push({
                language: 'control',
                text: char,
                startIndex: i,
                endIndex: i
            });
            
            currentSegment = null;
            continue;
        }
        
        // Start new segment or continue existing one
        if (!currentSegment || currentSegment.language !== language) {
            // Finalize previous segment
            if (currentSegment && currentSegment.text.length > 0) {
                segments.push({ ...currentSegment });
            }
            
            // Start new segment
            currentSegment = {
                language: language,
                text: char,
                startIndex: i,
                endIndex: i
            };
        } else {
            // Continue existing segment
            currentSegment.text += char;
            currentSegment.endIndex = i;
        }
    }
    
    // Finalize last segment
    if (currentSegment && currentSegment.text.length > 0) {
        segments.push(currentSegment);
    }
    
    return segments;
}

// Generate command protocol from text
export function generateLanguageProtocol(text) {
    if (!text || typeof text !== 'string') {
        return [];
    }
    
    const segments = segmentTextByLanguage(text);
    const commands = [];
    
    for (const segment of segments) {
        // Handle control characters
        if (segment.language === 'control') {
            // Convert common control characters to commands
            switch (segment.text) {
                case '\n':
                case '\r':
                    commands.push(PROTOCOL_COMMANDS.ENTER);
                    break;
                case '\t':
                    commands.push(PROTOCOL_COMMANDS.TAB);
                    break;
                default:
                    // For other control characters, include as text
                    commands.push(`${PROTOCOL_COMMANDS.TEXT_PREFIX}${segment.text}`);
                    break;
            }
            continue;
        }
        
        // Add language switch command
        if (segment.language === 'korean') {
            commands.push(PROTOCOL_COMMANDS.HANGUL);
            // Convert Korean text to QWERTY jamo keys
            const jamoKeys = convertHangulToJamoKeys(segment.text);
            commands.push(`${PROTOCOL_COMMANDS.TEXT_PREFIX}${jamoKeys}`);
        } else {
            // English or other characters
            commands.push(PROTOCOL_COMMANDS.ENGLISH);
            commands.push(`${PROTOCOL_COMMANDS.TEXT_PREFIX}${segment.text}`);
        }
    }
    
    return commands;
}

// Generate protocol as single string (newline-separated)
export function generateProtocolString(text) {
    const commands = generateLanguageProtocol(text);
    return commands.join('\n');
}

// Analyze protocol complexity
export function analyzeProtocol(text) {
    const segments = segmentTextByLanguage(text);
    const commands = generateLanguageProtocol(text);
    
    const analysis = {
        originalText: text,
        segments: segments,
        commands: commands,
        stats: {
            totalSegments: segments.length,
            koreanSegments: segments.filter(s => s.language === 'korean').length,
            englishSegments: segments.filter(s => s.language === 'english').length,
            controlSegments: segments.filter(s => s.language === 'control').length,
            languageSwitches: 0,
            totalCommands: commands.length
        }
    };
    
    // Count language switches
    let currentLanguage = null;
    for (const segment of segments) {
        if (segment.language !== 'control' && segment.language !== currentLanguage) {
            analysis.stats.languageSwitches++;
            currentLanguage = segment.language;
        }
    }
    
    return analysis;
}

// Validate protocol commands
export function validateProtocol(protocolString) {
    if (!protocolString || typeof protocolString !== 'string') {
        return { valid: false, errors: ['Protocol string is empty or invalid'] };
    }
    
    const lines = protocolString.split('\n');
    const errors = [];
    let lastCommand = null;
    
    for (let i = 0; i < lines.length; i++) {
        const line = lines[i].trim();
        
        if (line === '') {
            continue; // Skip empty lines
        }
        
        // Check if it's a valid command
        if (line.startsWith('#CMD:')) {
            if (!Object.values(PROTOCOL_COMMANDS).includes(line)) {
                errors.push(`Line ${i + 1}: Unknown command '${line}'`);
            }
            lastCommand = line;
        } else if (line.startsWith('#TEXT:')) {
            // TEXT commands should follow a language command
            if (lastCommand !== PROTOCOL_COMMANDS.HANGUL && 
                lastCommand !== PROTOCOL_COMMANDS.ENGLISH) {
                errors.push(`Line ${i + 1}: TEXT command without preceding language command`);
            }
        } else {
            errors.push(`Line ${i + 1}: Invalid protocol format '${line}'`);
        }
    }
    
    return {
        valid: errors.length === 0,
        errors: errors,
        lineCount: lines.length
    };
}

// Convert existing text processing to new protocol format
export function convertToProtocol(text, typingSpeed = 10) {
    const protocolString = generateProtocolString(text);
    const analysis = analyzeProtocol(text);
    
    // Maintain compatibility with existing JSON protocol
    const legacyProtocol = {
        text: text,
        speed_cps: typingSpeed,
        type: analysis.stats.koreanSegments > 0 ? 
              (analysis.stats.englishSegments > 0 ? 'mixed' : 'korean') : 'english'
    };
    
    return {
        // New protocol format
        protocol: protocolString,
        analysis: analysis,
        // Legacy compatibility
        legacy: JSON.stringify(legacyProtocol),
        // Enhanced metadata
        metadata: {
            version: '2.0',
            protocolType: 'structured-commands',
            generatedAt: new Date().toISOString(),
            stats: analysis.stats
        }
    };
}

// Future extensibility: Helper functions for special commands
export function createSpecialCommand(command, ...args) {
    switch (command.toLowerCase()) {
        case 'tab':
            return PROTOCOL_COMMANDS.TAB;
        case 'enter':
        case 'return':
            return PROTOCOL_COMMANDS.ENTER;
        case 'shift':
            return PROTOCOL_COMMANDS.SHIFT;
        case 'ctrl':
        case 'control':
            return PROTOCOL_COMMANDS.CTRL;
        case 'alt':
            return PROTOCOL_COMMANDS.ALT;
        default:
            return `#CMD:${command.toUpperCase()}`;
    }
}

// Testing and debugging utilities
export function testProtocol() {
    console.group('Language Protocol Testing');
    
    const testCases = [
        '안녕',
        'Hello',
        '안녕Hellow 난 jason이야!',
        'Test\nNewline\tTab',
        '한글English한글',
        '123 안녕 456',
        'Mixed 텍스트 with symbols!@#'
    ];
    
    testCases.forEach(testCase => {
        console.log(`\n--- Testing: "${testCase}" ---`);
        const result = convertToProtocol(testCase);
        console.log('Segments:', result.analysis.segments);
        console.log('Protocol:');
        console.log(result.protocol);
        console.log('Stats:', result.analysis.stats);
    });
    
    console.groupEnd();
}

// Export utilities for backward compatibility
export {
    segmentTextByLanguage as segmentText,
    generateLanguageProtocol as generateProtocol,
    convertToProtocol as processText
};