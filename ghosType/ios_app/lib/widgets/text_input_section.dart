import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:ghostype_ios/providers/text_conversion_provider.dart';

class TextInputSection extends StatefulWidget {
  const TextInputSection({super.key});

  @override
  State<TextInputSection> createState() => _TextInputSectionState();
}

class _TextInputSectionState extends State<TextInputSection> {
  final TextEditingController _controller = TextEditingController();
  final FocusNode _focusNode = FocusNode();

  @override
  void initState() {
    super.initState();
    _controller.addListener(_onTextChanged);
  }

  @override
  void dispose() {
    _controller.removeListener(_onTextChanged);
    _controller.dispose();
    _focusNode.dispose();
    super.dispose();
  }

  void _onTextChanged() {
    context.read<TextConversionProvider>().updateInputText(_controller.text);
  }

  @override
  Widget build(BuildContext context) {
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
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            // Header
            Row(
              children: [
                Icon(
                  Icons.edit_rounded,
                  size: 20,
                  color: Theme.of(context).colorScheme.primary,
                ),
                const SizedBox(width: 8),
                const Text(
                  'Text Input',
                  style: TextStyle(
                    fontSize: 16,
                    fontWeight: FontWeight.w600,
                  ),
                ),
                const Spacer(),
                Consumer<TextConversionProvider>(
                  builder: (context, provider, _) {
                    return TextButton(
                      onPressed: provider.inputText.isNotEmpty
                          ? () {
                              _controller.clear();
                              provider.clearInput();
                            }
                          : null,
                      child: const Text('Clear'),
                    );
                  },
                ),
              ],
            ),
            const SizedBox(height: 16),
            
            // Text Input Field
            TextField(
              controller: _controller,
              focusNode: _focusNode,
              maxLines: 5,
              minLines: 3,
              decoration: InputDecoration(
                hintText: 'Enter Korean/English text to type...\n\nExample:\ntest 안녕하세요\nHello 세계',
                hintStyle: TextStyle(
                  color: Colors.grey[500],
                  fontSize: 14,
                ),
                filled: true,
                fillColor: Colors.grey[50],
                border: OutlineInputBorder(
                  borderRadius: BorderRadius.circular(12),
                  borderSide: BorderSide.none,
                ),
                contentPadding: const EdgeInsets.all(16),
              ),
              style: const TextStyle(
                fontSize: 16,
                height: 1.5,
              ),
            ),
            const SizedBox(height: 12),
            
            // Character Counter
            Consumer<TextConversionProvider>(
              builder: (context, provider, _) {
                return Row(
                  children: [
                    Text(
                      '${provider.inputText.length} characters',
                      style: TextStyle(
                        fontSize: 12,
                        color: Colors.grey[600],
                      ),
                    ),
                    const Spacer(),
                    if (provider.inputText.isNotEmpty)
                      Container(
                        padding: const EdgeInsets.symmetric(
                          horizontal: 8,
                          vertical: 4,
                        ),
                        decoration: BoxDecoration(
                          color: _getTypeColor(provider.inputText),
                          borderRadius: BorderRadius.circular(12),
                        ),
                        child: Text(
                          _getTypeLabel(provider.inputText),
                          style: const TextStyle(
                            fontSize: 10,
                            color: Colors.white,
                            fontWeight: FontWeight.w500,
                          ),
                        ),
                      ),
                  ],
                );
              },
            ),
          ],
        ),
      ),
    );
  }

  Color _getTypeColor(String text) {
    final provider = context.read<TextConversionProvider>();
    final type = provider.analyzeText(text);
    
    switch (type) {
      case TextType.korean:
        return Colors.blue;
      case TextType.english:
        return Colors.green;
      case TextType.mixed:
        return Colors.orange;
    }
  }

  String _getTypeLabel(String text) {
    final provider = context.read<TextConversionProvider>();
    final type = provider.analyzeText(text);
    
    switch (type) {
      case TextType.korean:
        return 'KOR';
      case TextType.english:
        return 'ENG';
      case TextType.mixed:
        return 'MIX';
    }
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