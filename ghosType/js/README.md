# GHOSTYPE JavaScript Preprocessor

A sophisticated JavaScript preprocessor that formats user input for ESP32 HID typing with automatic Korean-English language switching.

## 🌟 Features

### 1. **Automatic Language Detection**
- Detects Korean (한글) and English text automatically
- Inserts language toggle markers at transition points
- Maintains context for symbols and punctuation

### 2. **Hangul-to-QWERTY Conversion**
- Accurate 2-set (두벌식) keyboard layout mapping
- Handles all Korean jamo (자모) decomposition
- Supports complex vowels (ㅘ, ㅙ, ㅚ, etc.)
- Processes compound consonants (ㄺ, ㄼ, etc.)

### 3. **Web Bluetooth Integration**
- Direct BLE connection to ESP32 device
- Automatic text preprocessing before transmission
- Configurable typing speed (1-50 CPS)
- Real-time status notifications

## 📁 File Structure

```
js/
├── hangulPreprocessor.js   # Core Korean text processing
├── webBLEInterface.js      # BLE communication handler
├── index.html              # Web UI for testing
├── test/
│   └── preprocessorTest.js # Comprehensive test suite
└── README.md               # This file
```

## 🚀 Usage

### Basic Example

```javascript
// Initialize preprocessor
const preprocessor = new HangulPreprocessor();

// Process mixed text
const input = "Hello 안녕하세요 World!";
const result = preprocessor.processText(input);
// Output: "Hello ⌨HANGUL_TOGGLE⌨dkssudgktpdy ⌨HANGUL_TOGGLE⌨World!"

// Format for ESP32
const formatted = preprocessor.formatForESP32(input);
// Returns: { text: "...", hasToggle: true, toggleMarker: "⌨HANGUL_TOGGLE⌨" }
```

### BLE Communication

```javascript
// Initialize BLE interface
const ble = new WebBLEInterface();

// Connect to ESP32
await ble.connect();

// Send mixed language text
await ble.sendText("Hello 안녕하세요!", { speed_cps: 10 });

// The interface automatically:
// 1. Preprocesses text
// 2. Handles language switching
// 3. Sends appropriate commands to ESP32
```

## 🧪 Testing

### Run Tests
```bash
node test/preprocessorTest.js
```

### Test Coverage
- ✅ Basic Hangul conversion (가→rk, 윤→dbs)
- ✅ Complex jamo (맑→akfr, 넓→spfq)
- ✅ Language switching detection
- ✅ Edge cases (empty, symbols, mixed)
- ✅ Performance (>1000 chars/ms)

## 🔧 Configuration

### Typing Speed
```javascript
// Set default speed (characters per second)
ble.defaultConfig.speed_cps = 10;

// Or per message
await ble.sendText(text, { speed_cps: 15 });
```

### Custom Toggle Marker
```javascript
// Change the language toggle marker
preprocessor.HANGUL_TOGGLE = '[SWITCH]';
```

## 📊 Conversion Examples

| Korean | QWERTY | Description |
|--------|---------|-------------|
| 가 | rk | ㄱ + ㅏ |
| 윤 | dbs | ㅇ + ㅠ + ㄴ |
| 하늘 | gksrmf | 하(ㅎ+ㅏ) + 늘(ㄴ+ㅡ+ㄹ) |
| 되 | enl | ㄷ + ㅚ |
| 돼 | eho | ㄷ + ㅙ |
| 맑 | akfr | ㅁ + ㅏ + ㄺ |

## 🌐 Browser Compatibility

- Chrome 56+ (Web Bluetooth support)
- Edge 79+
- Opera 43+
- Chrome Android 56+

**Note**: Web Bluetooth is not supported in Firefox or Safari.

## 🔒 Security

- BLE communication uses characteristic UUIDs
- No sensitive data stored
- All processing done locally in browser

## 📝 License

This project is part of the GHOSTYPE system for smart keyboard switching.