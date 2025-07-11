import 'package:flutter/foundation.dart';
import 'package:shared_preferences/shared_preferences.dart';

class TextConversionProvider extends ChangeNotifier {
  // Korean to QWERTY mapping
  static const Map<String, String> qwertyToJamo = {
    // Basic Consonants
    'q': 'ㅂ', 'w': 'ㅈ', 'e': 'ㄷ', 'r': 'ㄱ', 't': 'ㅅ',
    'a': 'ㅁ', 's': 'ㄴ', 'd': 'ㅇ', 'f': 'ㄹ', 'g': 'ㅎ',
    'z': 'ㅋ', 'x': 'ㅌ', 'c': 'ㅊ', 'v': 'ㅍ',
    // Basic Vowels
    'y': 'ㅛ', 'u': 'ㅕ', 'i': 'ㅑ', 'o': 'ㅐ', 'p': 'ㅔ',
    'h': 'ㅗ', 'j': 'ㅓ', 'k': 'ㅏ', 'l': 'ㅣ',
    'b': 'ㅠ', 'n': 'ㅜ', 'm': 'ㅡ',
    // Shift + Consonants
    'Q': 'ㅃ', 'W': 'ㅉ', 'E': 'ㄸ', 'R': 'ㄲ', 'T': 'ㅆ',
    // Shift + Vowels
    'O': 'ㅒ', 'P': 'ㅖ'
  };

  // Jamo arrays for decomposition
  static const List<String> chosung = [
    'ㄱ', 'ㄲ', 'ㄴ', 'ㄷ', 'ㄸ', 'ㄹ', 'ㅁ', 'ㅂ', 'ㅃ', 'ㅅ',
    'ㅆ', 'ㅇ', 'ㅈ', 'ㅉ', 'ㅊ', 'ㅋ', 'ㅌ', 'ㅍ', 'ㅎ'
  ];
  
  static const List<String> jungsung = [
    'ㅏ', 'ㅐ', 'ㅑ', 'ㅒ', 'ㅓ', 'ㅔ', 'ㅕ', 'ㅖ', 'ㅗ', 'ㅘ',
    'ㅙ', 'ㅚ', 'ㅛ', 'ㅜ', 'ㅝ', 'ㅞ', 'ㅟ', 'ㅠ', 'ㅡ', 'ㅢ', 'ㅣ'
  ];
  
  static const List<String> jongsung = [
    '', 'ㄱ', 'ㄲ', 'ㄳ', 'ㄴ', 'ㄵ', 'ㄶ', 'ㄷ', 'ㄹ', 'ㄺ',
    'ㄻ', 'ㄼ', 'ㄽ', 'ㄾ', 'ㄿ', 'ㅀ', 'ㅁ', 'ㅂ', 'ㅄ', 'ㅅ',
    'ㅆ', 'ㅇ', 'ㅈ', 'ㅊ', 'ㅋ', 'ㅌ', 'ㅍ', 'ㅎ'
  ];

  // State
  String _inputText = '';
  String _convertedText = '';
  String _protocol = '';
  int _typingSpeed = 10;
  int _countdownSeconds = 5;
  final List<MessageHistoryItem> _messageHistory = [];

  // Getters
  String get inputText => _inputText;
  String get convertedText => _convertedText;
  String get protocol => _protocol;
  int get typingSpeed => _typingSpeed;
  int get countdownSeconds => _countdownSeconds;
  List<MessageHistoryItem> get messageHistory => _messageHistory;

  TextConversionProvider() {
    _loadSettings();
  }

  Future<void> _loadSettings() async {
    final prefs = await SharedPreferences.getInstance();
    _typingSpeed = prefs.getInt('typing_speed') ?? 10;
    _countdownSeconds = prefs.getInt('countdown_seconds') ?? 5;
    notifyListeners();
  }

  Future<void> setTypingSpeed(int speed) async {
    _typingSpeed = speed;
    final prefs = await SharedPreferences.getInstance();
    await prefs.setInt('typing_speed', speed);
    notifyListeners();
  }

  Future<void> setCountdownSeconds(int seconds) async {
    _countdownSeconds = seconds;
    final prefs = await SharedPreferences.getInstance();
    await prefs.setInt('countdown_seconds', seconds);
    notifyListeners();
  }

  void updateInputText(String text) {
    _inputText = text;
    _convertText();
    notifyListeners();
  }

  void _convertText() {
    final analysis = _analyzeText(_inputText);
    
    if (analysis == TextType.mixed) {
      final segments = _segmentTextByLanguage(_inputText);
      _protocol = _generateBlockProtocol(segments);
    } else if (analysis == TextType.korean) {
      final jamoKeys = _convertHangulToJamoKeys(_inputText);
      _protocol = '#CMD:HANGUL\n${_processTextWithEnters(jamoKeys, 'korean').join('\n')}';
      _convertedText = jamoKeys;
    } else {
      _protocol = '#CMD:ENGLISH\n${_processTextWithEnters(_inputText, 'english').join('\n')}';
      _convertedText = _inputText;
    }
  }

  TextType _analyzeText(String text) {
    bool hasKorean = false;
    bool hasEnglish = false;

    for (int i = 0; i < text.length; i++) {
      final code = text.codeUnitAt(i);
      if (code >= 0xAC00 && code <= 0xD7A3) {
        hasKorean = true;
      } else if ((code >= 65 && code <= 90) || (code >= 97 && code <= 122)) {
        hasEnglish = true;
      }
    }

    if (hasKorean && hasEnglish) return TextType.mixed;
    if (hasKorean) return TextType.korean;
    return TextType.english;
  }

  List<TextSegment> _segmentTextByLanguage(String text) {
    final segments = <TextSegment>[];
    String currentSegment = '';
    Language? currentLanguage;

    for (int i = 0; i < text.length; i++) {
      final char = text[i];
      final code = char.codeUnitAt(0);
      
      Language charLanguage;
      if (code >= 0xAC00 && code <= 0xD7A3) {
        charLanguage = Language.korean;
      } else {
        charLanguage = Language.english;
      }

      if (currentLanguage != null && currentLanguage != charLanguage) {
        if (currentSegment.isNotEmpty) {
          segments.add(TextSegment(text: currentSegment, language: currentLanguage));
        }
        currentSegment = char;
        currentLanguage = charLanguage;
      } else {
        currentSegment += char;
        currentLanguage = charLanguage;
      }
    }

    if (currentSegment.isNotEmpty && currentLanguage != null) {
      segments.add(TextSegment(text: currentSegment, language: currentLanguage));
    }

    return segments;
  }

  String _generateBlockProtocol(List<TextSegment> segments) {
    final protocolBlocks = <String>[];
    _convertedText = '';

    for (final segment in segments) {
      if (segment.language == Language.korean) {
        protocolBlocks.add('#CMD:HANGUL');
        final jamoKeys = _convertHangulToJamoKeys(segment.text);
        protocolBlocks.addAll(_processTextWithEnters(jamoKeys, 'korean'));
        _convertedText += jamoKeys;
      } else {
        protocolBlocks.add('#CMD:ENGLISH');
        protocolBlocks.addAll(_processTextWithEnters(segment.text, 'english'));
        _convertedText += segment.text;
      }
    }

    return protocolBlocks.join('\n');
  }

  List<String> _processTextWithEnters(String text, String language) {
    final blocks = <String>[];
    final parts = text.split('\n');

    for (int i = 0; i < parts.length; i++) {
      if (parts[i].isNotEmpty) {
        blocks.add('#TEXT:${parts[i]}');
      }
      if (i < parts.length - 1) {
        blocks.add('#CMD:ENTER');
      }
    }

    return blocks;
  }

  String _convertHangulToJamoKeys(String text) {
    String result = '';
    
    for (int i = 0; i < text.length; i++) {
      final char = text[i];
      final jamos = _decomposeHangul(char);
      
      if (jamos != null) {
        for (final jamo in jamos) {
          result += _jamoToQwerty(jamo);
        }
      } else {
        result += char;
      }
    }
    
    return result;
  }

  List<String>? _decomposeHangul(String char) {
    final code = char.codeUnitAt(0);
    
    if (code < 0xAC00 || code > 0xD7A3) {
      return null;
    }
    
    final syllableIndex = code - 0xAC00;
    final chosungIndex = syllableIndex ~/ 588;
    final jungsungIndex = (syllableIndex % 588) ~/ 28;
    final jongsungIndex = syllableIndex % 28;
    
    final result = <String>[chosung[chosungIndex], jungsung[jungsungIndex]];
    if (jongsungIndex > 0) {
      result.add(jongsung[jongsungIndex]);
    }
    
    return result;
  }

  String _jamoToQwerty(String jamo) {
    final jamoToQwerty = Map.fromEntries(
      qwertyToJamo.entries.map((e) => MapEntry(e.value, e.key))
    );
    return jamoToQwerty[jamo] ?? jamo;
  }

  void addToHistory(String message, TextType type) {
    _messageHistory.insert(0, MessageHistoryItem(
      message: message,
      type: type,
      convertedText: _convertedText,
      timestamp: DateTime.now(),
    ));
    
    if (_messageHistory.length > 50) {
      _messageHistory.removeLast();
    }
    
    notifyListeners();
  }

  void clearInput() {
    _inputText = '';
    _convertedText = '';
    _protocol = '';
    notifyListeners();
  }
}

enum TextType { korean, english, mixed }
enum Language { korean, english }

class TextSegment {
  final String text;
  final Language language;

  TextSegment({required this.text, required this.language});
}

class MessageHistoryItem {
  final String message;
  final TextType type;
  final String convertedText;
  final DateTime timestamp;

  MessageHistoryItem({
    required this.message,
    required this.type,
    required this.convertedText,
    required this.timestamp,
  });
}