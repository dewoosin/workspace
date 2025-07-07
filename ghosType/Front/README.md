# GHOSTYPE Frontend - Modular Architecture

A refactored, modular web application for GHOSTYPE BLE keyboard interface with Korean and English text input support.

## 📁 Project Structure

```
Front/
├── index.html              # Original monolithic file
├── index-refactored.html   # New modular HTML structure
├── config.js              # Application configuration
├── README.md              # This documentation
│
├── styles/                # CSS modules
│   ├── reset.css          # CSS reset and base styles
│   ├── layout.css         # Layout and container styles
│   ├── components.css     # UI component styles
│   ├── modals.css         # Modal and overlay styles
│   └── responsive.css     # Responsive design rules
│
├── scripts/               # JavaScript modules
│   ├── main.js            # Application entry point
│   ├── constants.js       # Application constants
│   ├── ble-manager.js     # Bluetooth Low Energy management
│   ├── ui-controller.js   # UI state and event handling
│   ├── korean-converter.js # Korean text conversion utilities
│   └── logger.js          # Logging and debug utilities
│
├── components/            # Reusable HTML components
│   ├── modal-template.html     # Generic modal template
│   └── test-button-group.html  # Test button group component
│
└── assets/               # Static assets (future use)
    └── (images, fonts, etc.)
```

## 🚀 Features

### Core Functionality
- **Web Bluetooth Integration**: Connect to GHOSTYPE ESP32 devices
- **Korean Text Support**: Automatic Hangul to Jamo key conversion
- **Mixed Language Input**: Support for Korean, English, and mixed text
- **Real-time Preview**: Live conversion preview before sending
- **Typing Speed Control**: Configurable typing speed (3, 6, 10 CPS)
- **Special Commands**: Support for special keys and shortcuts

### Architecture Benefits
- **Modular Design**: Separated concerns for easier maintenance
- **ES6 Modules**: Modern JavaScript module system
- **Responsive Design**: Mobile-friendly responsive layout
- **Accessibility**: ARIA labels and semantic HTML
- **Configuration-driven**: Easy customization through config files

## 🛠️ Technical Details

### CSS Architecture
- **reset.css**: Base styles and CSS reset
- **layout.css**: Grid, flexbox, and layout utilities
- **components.css**: Individual UI component styles
- **modals.css**: Modal dialogs and overlays
- **responsive.css**: Mobile and tablet breakpoints

### JavaScript Modules
- **main.js**: Application initialization and global functions
- **ble-manager.js**: Bluetooth connection and data transmission
- **ui-controller.js**: UI state management and event handling
- **korean-converter.js**: Hangul decomposition and conversion
- **logger.js**: Centralized logging and debugging
- **constants.js**: Shared constants and configurations

### Component System
- **Modal Template**: Reusable modal dialog structure
- **Button Groups**: Configurable test button collections
- **Configuration**: Centralized app settings in `config.js`

## 🔧 Usage

### Development
1. Use `index-refactored.html` as the main entry point
2. Modify styles in the appropriate CSS files
3. Update JavaScript functionality in the relevant modules
4. Configure application settings in `config.js`

### Customization
- **Add New Test Buttons**: Update `config.js` TEST_BUTTONS section
- **Modify Styling**: Edit the appropriate CSS file in `styles/`
- **Add Features**: Create new modules in `scripts/` directory
- **Localization**: Extend the MESSAGES section in `config.js`

### Browser Compatibility
- **Required**: Chrome/Edge with Web Bluetooth support
- **Responsive**: Works on desktop, tablet, and mobile devices
- **Modern JavaScript**: Requires ES6 module support

## 📱 Responsive Breakpoints

- **Desktop**: > 600px (full layout)
- **Mobile**: ≤ 600px (stacked layout, larger touch targets)

## 🔗 Dependencies

- **No External Libraries**: Pure vanilla JavaScript and CSS
- **Web Bluetooth API**: Built-in browser support required
- **ES6 Modules**: Modern browser module system

## 🚦 Getting Started

1. **Open the refactored version**:
   ```
   Open index-refactored.html in a supported browser
   ```

2. **Connect to device**:
   - Click "연결" button
   - Select GHOSTYPE device from list
   - Wait for connection confirmation

3. **Test functionality**:
   - Use preset test buttons
   - Type custom messages
   - Adjust typing speed as needed

## 🔍 Migration Notes

### From Original to Refactored
- **HTML**: Cleaner structure with semantic elements
- **CSS**: Organized into logical modules
- **JavaScript**: Separated into focused modules
- **Functionality**: All original features preserved
- **Performance**: Improved loading and maintainability

### Breaking Changes
- **File Structure**: Multiple files instead of single HTML
- **Module Loading**: ES6 imports require modern browser
- **Global Functions**: Some functions moved to modules

## 🛡️ Security Considerations

- **Clipboard Access**: Secure clipboard API usage
- **Bluetooth Permissions**: User-initiated connections only
- **Input Sanitization**: Text cleaning for safe transmission
- **No External CDN**: All code served locally

## 📈 Performance Optimizations

- **Module Loading**: Lazy loading of JavaScript modules
- **CSS Organization**: Efficient cascade and specificity
- **Event Delegation**: Optimized event handling
- **Responsive Images**: Future-ready asset optimization

## 🔮 Future Enhancements

- **Component Library**: Expand reusable components
- **State Management**: Add centralized state management
- **Testing Framework**: Unit and integration tests
- **Build System**: Bundling and optimization tools
- **PWA Features**: Offline support and app installation

## 📝 Contributing

When adding new features:
1. Follow the modular architecture pattern
2. Add appropriate CSS to the relevant stylesheet
3. Create focused JavaScript modules
4. Update configuration files as needed
5. Maintain responsive design principles
6. Include accessibility attributes

## 🐛 Debugging

- **Console Logs**: Check browser developer console
- **Log Modal**: Use built-in log viewer in the app
- **Module Loading**: Ensure ES6 module support
- **Bluetooth**: Verify Web Bluetooth API availability