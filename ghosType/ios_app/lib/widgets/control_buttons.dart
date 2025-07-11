import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:ghostype_ios/providers/ble_provider.dart';
import 'package:ghostype_ios/providers/text_conversion_provider.dart';

class ControlButtons extends StatefulWidget {
  const ControlButtons({super.key});

  @override
  State<ControlButtons> createState() => _ControlButtonsState();
}

class _ControlButtonsState extends State<ControlButtons> {
  bool _isTyping = false;
  bool _isCountingDown = false;
  int _countdownValue = 0;

  @override
  Widget build(BuildContext context) {
    return Consumer2<BleProvider, TextConversionProvider>(
      builder: (context, bleProvider, textProvider, _) {
        final canType = bleProvider.isConnected && 
                       textProvider.inputText.isNotEmpty && 
                       !_isTyping && 
                       !_isCountingDown;

        return Card(
          elevation: 0,
          shape: RoundedRectangleBorder(
            borderRadius: BorderRadius.circular(16),
            side: BorderSide(
              color: Colors.grey.withOpacity(0.3),
              width: 1,
            ),
          ),
          child: Padding(
            padding: const EdgeInsets.all(20.0),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.stretch,
              children: [
                // Header
                Row(
                  children: [
                    Icon(
                      Icons.keyboard_rounded,
                      size: 20,
                      color: Theme.of(context).colorScheme.primary,
                    ),
                    const SizedBox(width: 8),
                    const Text(
                      'Control',
                      style: TextStyle(
                        fontSize: 16,
                        fontWeight: FontWeight.w600,
                      ),
                    ),
                  ],
                ),
                const SizedBox(height: 16),
                
                // Main Type Button
                SizedBox(
                  height: 56,
                  child: FilledButton(
                    onPressed: canType ? () => _startTyping(context) : null,
                    style: FilledButton.styleFrom(
                      backgroundColor: _isCountingDown 
                          ? Colors.orange 
                          : _isTyping 
                              ? Colors.red 
                              : null,
                      shape: RoundedRectangleBorder(
                        borderRadius: BorderRadius.circular(12),
                      ),
                    ),
                    child: _buildButtonContent(bleProvider, textProvider),
                  ),
                ),
                const SizedBox(height: 12),
                
                // Secondary Actions
                Row(
                  children: [
                    // Speed Control
                    Expanded(
                      child: OutlinedButton.icon(
                        onPressed: () => _showSpeedDialog(context),
                        icon: const Icon(Icons.speed_rounded, size: 18),
                        label: Text('${textProvider.typingSpeed} ms'),
                        style: OutlinedButton.styleFrom(
                          shape: RoundedRectangleBorder(
                            borderRadius: BorderRadius.circular(12),
                          ),
                        ),
                      ),
                    ),
                    const SizedBox(width: 12),
                    
                    // Countdown Setting
                    Expanded(
                      child: OutlinedButton.icon(
                        onPressed: () => _showCountdownDialog(context),
                        icon: const Icon(Icons.timer_rounded, size: 18),
                        label: Text('${textProvider.countdownSeconds}s'),
                        style: OutlinedButton.styleFrom(
                          shape: RoundedRectangleBorder(
                            borderRadius: BorderRadius.circular(12),
                          ),
                        ),
                      ),
                    ),
                  ],
                ),
              ],
            ),
          ),
        );
      },
    );
  }

  Widget _buildButtonContent(BleProvider bleProvider, TextConversionProvider textProvider) {
    if (!bleProvider.isConnected) {
      return const Row(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          Icon(Icons.bluetooth_disabled_rounded, size: 20),
          SizedBox(width: 8),
          Text('Connect Device First'),
        ],
      );
    }

    if (textProvider.inputText.isEmpty) {
      return const Row(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          Icon(Icons.edit_rounded, size: 20),
          SizedBox(width: 8),
          Text('Enter Text First'),
        ],
      );
    }

    if (_isCountingDown) {
      return Row(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          SizedBox(
            width: 16,
            height: 16,
            child: CircularProgressIndicator(
              strokeWidth: 2,
              valueColor: AlwaysStoppedAnimation<Color>(
                Theme.of(context).colorScheme.onPrimary,
              ),
            ),
          ),
          const SizedBox(width: 8),
          Text('Starting in $_countdownValue...'),
        ],
      );
    }

    if (_isTyping) {
      return const Row(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          SizedBox(
            width: 16,
            height: 16,
            child: CircularProgressIndicator(
              strokeWidth: 2,
              valueColor: AlwaysStoppedAnimation<Color>(Colors.white),
            ),
          ),
          SizedBox(width: 8),
          Text('Typing...'),
        ],
      );
    }

    return const Row(
      mainAxisAlignment: MainAxisAlignment.center,
      children: [
        Icon(Icons.send_rounded, size: 20),
        SizedBox(width: 8),
        Text('Start Typing'),
      ],
    );
  }

  Future<void> _startTyping(BuildContext context) async {
    final bleProvider = context.read<BleProvider>();
    final textProvider = context.read<TextConversionProvider>();

    // Start countdown
    setState(() {
      _isCountingDown = true;
      _countdownValue = textProvider.countdownSeconds;
    });

    // Countdown
    for (int i = textProvider.countdownSeconds; i > 0; i--) {
      if (!mounted) return;
      setState(() => _countdownValue = i);
      await Future.delayed(const Duration(seconds: 1));
    }

    if (!mounted) return;

    setState(() {
      _isCountingDown = false;
      _isTyping = true;
    });

    // Send the protocol to ESP32
    final success = await bleProvider.sendData(textProvider.protocol);
    
    if (success) {
      textProvider.addToHistory(
        textProvider.inputText,
        textProvider.analyzeText(textProvider.inputText),
      );
      
      // Show success message
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          const SnackBar(
            content: Text('✅ Typing commands sent successfully!'),
            backgroundColor: Colors.green,
          ),
        );
      }
    } else {
      // Show error message
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          const SnackBar(
            content: Text('❌ Failed to send typing commands'),
            backgroundColor: Colors.red,
          ),
        );
      }
    }

    if (mounted) {
      setState(() => _isTyping = false);
    }
  }

  void _showSpeedDialog(BuildContext context) {
    showDialog(
      context: context,
      builder: (context) {
        return Consumer<TextConversionProvider>(
          builder: (context, provider, _) {
            return AlertDialog(
              title: const Text('Typing Speed'),
              content: Column(
                mainAxisSize: MainAxisSize.min,
                children: [
                  const Text('Adjust the delay between keystrokes:'),
                  const SizedBox(height: 20),
                  Slider(
                    value: provider.typingSpeed.toDouble(),
                    min: 1,
                    max: 100,
                    divisions: 99,
                    label: '${provider.typingSpeed} ms',
                    onChanged: (value) {
                      provider.setTypingSpeed(value.toInt());
                    },
                  ),
                  Text('${provider.typingSpeed} ms per keystroke'),
                ],
              ),
              actions: [
                TextButton(
                  onPressed: () => Navigator.of(context).pop(),
                  child: const Text('Close'),
                ),
              ],
            );
          },
        );
      },
    );
  }

  void _showCountdownDialog(BuildContext context) {
    showDialog(
      context: context,
      builder: (context) {
        return Consumer<TextConversionProvider>(
          builder: (context, provider, _) {
            return AlertDialog(
              title: const Text('Countdown Timer'),
              content: Column(
                mainAxisSize: MainAxisSize.min,
                children: [
                  const Text('Set countdown before typing starts:'),
                  const SizedBox(height: 20),
                  Slider(
                    value: provider.countdownSeconds.toDouble(),
                    min: 0,
                    max: 10,
                    divisions: 10,
                    label: '${provider.countdownSeconds}s',
                    onChanged: (value) {
                      provider.setCountdownSeconds(value.toInt());
                    },
                  ),
                  Text('${provider.countdownSeconds} seconds'),
                ],
              ),
              actions: [
                TextButton(
                  onPressed: () => Navigator.of(context).pop(),
                  child: const Text('Close'),
                ),
              ],
            );
          },
        );
      },
    );
  }
}

extension TextAnalysis on TextConversionProvider {
  TextType analyzeText(String text) {
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
}