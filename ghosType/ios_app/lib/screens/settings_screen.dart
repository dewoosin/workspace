import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:ghostype_ios/providers/text_conversion_provider.dart';
import 'package:ghostype_ios/providers/ble_provider.dart';
import 'package:package_info_plus/package_info_plus.dart';

class SettingsScreen extends StatefulWidget {
  const SettingsScreen({super.key});

  @override
  State<SettingsScreen> createState() => _SettingsScreenState();
}

class _SettingsScreenState extends State<SettingsScreen> {
  PackageInfo? _packageInfo;

  @override
  void initState() {
    super.initState();
    _loadPackageInfo();
  }

  Future<void> _loadPackageInfo() async {
    final info = await PackageInfo.fromPlatform();
    setState(() => _packageInfo = info);
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Settings'),
        elevation: 0,
      ),
      body: ListView(
        padding: const EdgeInsets.all(16),
        children: [
          // Typing Settings
          _buildSection(
            'Typing Settings',
            Icons.keyboard_rounded,
            [
              Consumer<TextConversionProvider>(
                builder: (context, provider, _) {
                  return ListTile(
                    leading: const Icon(Icons.speed_rounded),
                    title: const Text('Typing Speed'),
                    subtitle: Text('${provider.typingSpeed} ms delay between keystrokes'),
                    trailing: const Icon(Icons.chevron_right_rounded),
                    onTap: () => _showSpeedDialog(context),
                  );
                },
              ),
              Consumer<TextConversionProvider>(
                builder: (context, provider, _) {
                  return ListTile(
                    leading: const Icon(Icons.timer_rounded),
                    title: const Text('Countdown Timer'),
                    subtitle: Text('${provider.countdownSeconds} seconds before typing starts'),
                    trailing: const Icon(Icons.chevron_right_rounded),
                    onTap: () => _showCountdownDialog(context),
                  );
                },
              ),
            ],
          ),

          const SizedBox(height: 20),

          // BLE Settings
          _buildSection(
            'Bluetooth Settings',
            Icons.bluetooth_rounded,
            [
              Consumer<BleProvider>(
                builder: (context, bleProvider, _) {
                  return ListTile(
                    leading: const Icon(Icons.bluetooth_searching_rounded),
                    title: const Text('Auto-connect'),
                    subtitle: const Text('Automatically connect to known devices'),
                    trailing: Switch(
                      value: false, // TODO: Implement auto-connect setting
                      onChanged: (value) {
                        // TODO: Implement auto-connect toggle
                      },
                    ),
                  );
                },
              ),
              ListTile(
                leading: const Icon(Icons.device_hub_rounded),
                title: const Text('Device Information'),
                subtitle: const Text('View connected device details'),
                trailing: const Icon(Icons.chevron_right_rounded),
                onTap: () => _showDeviceInfo(context),
              ),
              ListTile(
                leading: const Icon(Icons.refresh_rounded),
                title: const Text('Reset Connection'),
                subtitle: const Text('Clear connection cache and restart'),
                trailing: const Icon(Icons.chevron_right_rounded),
                onTap: () => _showResetDialog(context),
              ),
            ],
          ),

          const SizedBox(height: 20),

          // Korean Input Settings
          _buildSection(
            'Korean Input',
            Icons.language_rounded,
            [
              ListTile(
                leading: const Icon(Icons.keyboard_alt_rounded),
                title: const Text('QWERTY Mapping'),
                subtitle: const Text('View Korean to QWERTY key mappings'),
                trailing: const Icon(Icons.chevron_right_rounded),
                onTap: () => _showQwertyMapping(context),
              ),
              ListTile(
                leading: const Icon(Icons.help_outline_rounded),
                title: const Text('Input Guide'),
                subtitle: const Text('Learn how to use mixed Korean/English input'),
                trailing: const Icon(Icons.chevron_right_rounded),
                onTap: () => _showInputGuide(context),
              ),
            ],
          ),

          const SizedBox(height: 20),

          // About
          _buildSection(
            'About',
            Icons.info_rounded,
            [
              ListTile(
                leading: const Icon(Icons.apps_rounded),
                title: const Text('App Version'),
                subtitle: Text(_packageInfo?.version ?? 'Loading...'),
              ),
              ListTile(
                leading: const Icon(Icons.code_rounded),
                title: const Text('Build Number'),
                subtitle: Text(_packageInfo?.buildNumber ?? 'Loading...'),
              ),
              ListTile(
                leading: const Icon(Icons.description_rounded),
                title: const Text('Open Source'),
                subtitle: const Text('View source code and contribute'),
                trailing: const Icon(Icons.open_in_new_rounded),
                onTap: () {
                  // TODO: Open GitHub repository
                },
              ),
            ],
          ),
        ],
      ),
    );
  }

  Widget _buildSection(String title, IconData icon, List<Widget> children) {
    return Card(
      elevation: 0,
      shape: RoundedRectangleBorder(
        borderRadius: BorderRadius.circular(16),
        side: BorderSide(
          color: Colors.grey.withOpacity(0.3),
          width: 1,
        ),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Padding(
            padding: const EdgeInsets.all(16),
            child: Row(
              children: [
                Icon(icon, size: 20, color: Theme.of(context).colorScheme.primary),
                const SizedBox(width: 8),
                Text(
                  title,
                  style: const TextStyle(
                    fontSize: 16,
                    fontWeight: FontWeight.w600,
                  ),
                ),
              ],
            ),
          ),
          ...children,
        ],
      ),
    );
  }

  void _showSpeedDialog(BuildContext context) {
    showDialog(
      context: context,
      builder: (context) => Consumer<TextConversionProvider>(
        builder: (context, provider, _) => AlertDialog(
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
                onChanged: (value) => provider.setTypingSpeed(value.toInt()),
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
        ),
      ),
    );
  }

  void _showCountdownDialog(BuildContext context) {
    showDialog(
      context: context,
      builder: (context) => Consumer<TextConversionProvider>(
        builder: (context, provider, _) => AlertDialog(
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
                onChanged: (value) => provider.setCountdownSeconds(value.toInt()),
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
        ),
      ),
    );
  }

  void _showDeviceInfo(BuildContext context) {
    showDialog(
      context: context,
      builder: (context) => Consumer<BleProvider>(
        builder: (context, bleProvider, _) => AlertDialog(
          title: const Text('Device Information'),
          content: Column(
            mainAxisSize: MainAxisSize.min,
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              _buildInfoRow('Status', bleProvider.isConnected ? 'Connected' : 'Disconnected'),
              _buildInfoRow('Device Name', bleProvider.deviceName.isNotEmpty ? bleProvider.deviceName : 'None'),
              _buildInfoRow('Service UUID', '6e400001-b5a3-f393-e0a9-e50e24dcca9e'),
              _buildInfoRow('Message', bleProvider.statusMessage),
            ],
          ),
          actions: [
            TextButton(
              onPressed: () => Navigator.of(context).pop(),
              child: const Text('Close'),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildInfoRow(String label, String value) {
    return Padding(
      padding: const EdgeInsets.only(bottom: 8),
      child: Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          SizedBox(
            width: 80,
            child: Text(
              '$label:',
              style: const TextStyle(fontWeight: FontWeight.w600),
            ),
          ),
          Expanded(
            child: Text(
              value,
              style: const TextStyle(fontFamily: 'Courier'),
            ),
          ),
        ],
      ),
    );
  }

  void _showResetDialog(BuildContext context) {
    showDialog(
      context: context,
      builder: (context) => AlertDialog(
        title: const Text('Reset Connection'),
        content: const Text('This will disconnect the current device and clear the connection cache. Continue?'),
        actions: [
          TextButton(
            onPressed: () => Navigator.of(context).pop(),
            child: const Text('Cancel'),
          ),
          TextButton(
            onPressed: () {
              context.read<BleProvider>().disconnect();
              Navigator.of(context).pop();
            },
            child: const Text('Reset'),
          ),
        ],
      ),
    );
  }

  void _showQwertyMapping(BuildContext context) {
    showDialog(
      context: context,
      builder: (context) => AlertDialog(
        title: const Text('QWERTY Mapping'),
        content: SizedBox(
          width: double.maxFinite,
          child: Column(
            mainAxisSize: MainAxisSize.min,
            children: [
              const Text('Korean characters mapped to QWERTY keys:'),
              const SizedBox(height: 16),
              Flexible(
                child: SingleChildScrollView(
                  child: Table(
                    columnWidths: const {
                      0: FlexColumnWidth(1),
                      1: FlexColumnWidth(1),
                      2: FlexColumnWidth(1),
                      3: FlexColumnWidth(1),
                    },
                    children: [
                      const TableRow(
                        children: [
                          TableCell(child: Text('Key', style: TextStyle(fontWeight: FontWeight.bold))),
                          TableCell(child: Text('Korean', style: TextStyle(fontWeight: FontWeight.bold))),
                          TableCell(child: Text('Key', style: TextStyle(fontWeight: FontWeight.bold))),
                          TableCell(child: Text('Korean', style: TextStyle(fontWeight: FontWeight.bold))),
                        ],
                      ),
                      ...['q:ㅂ', 'w:ㅈ', 'e:ㄷ', 'r:ㄱ', 't:ㅅ', 'y:ㅛ', 'u:ㅕ', 'i:ㅑ', 'o:ㅐ', 'p:ㅔ']
                          .asMap()
                          .entries
                          .map((entry) {
                        final pairs = ['q:ㅂ', 'w:ㅈ', 'e:ㄷ', 'r:ㄱ', 't:ㅅ', 'y:ㅛ', 'u:ㅕ', 'i:ㅑ', 'o:ㅐ', 'p:ㅔ',
                                       'a:ㅁ', 's:ㄴ', 'd:ㅇ', 'f:ㄹ', 'g:ㅎ', 'h:ㅗ', 'j:ㅓ', 'k:ㅏ', 'l:ㅣ', 'z:ㅋ'];
                        final index = entry.key;
                        if (index < pairs.length ~/ 2) {
                          final left = pairs[index].split(':');
                          final right = pairs[index + 10].split(':');
                          return TableRow(
                            children: [
                              TableCell(child: Text(left[0], style: const TextStyle(fontFamily: 'Courier'))),
                              TableCell(child: Text(left[1])),
                              TableCell(child: Text(right[0], style: const TextStyle(fontFamily: 'Courier'))),
                              TableCell(child: Text(right[1])),
                            ],
                          );
                        }
                        return const TableRow(children: []);
                      }).where((row) => row.children.isNotEmpty).toList(),
                    ],
                  ),
                ),
              ),
            ],
          ),
        ),
        actions: [
          TextButton(
            onPressed: () => Navigator.of(context).pop(),
            child: const Text('Close'),
          ),
        ],
      ),
    );
  }

  void _showInputGuide(BuildContext context) {
    showDialog(
      context: context,
      builder: (context) => AlertDialog(
        title: const Text('Input Guide'),
        content: const SingleChildScrollView(
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            mainAxisSize: MainAxisSize.min,
            children: [
              Text('How to use GHOSTYPE:', style: TextStyle(fontWeight: FontWeight.bold)),
              SizedBox(height: 8),
              Text('1. Connect to your ESP32 device'),
              Text('2. Enter mixed Korean/English text'),
              Text('3. The app will automatically detect language segments'),
              Text('4. Tap "Start Typing" to send commands'),
              SizedBox(height: 16),
              Text('Examples:', style: TextStyle(fontWeight: FontWeight.bold)),
              Text('• "Hello 안녕하세요" → Mixed input'),
              Text('• "test\\ntest" → Multi-line input'),
              Text('• "안녕" → Korean only'),
              Text('• "Hello" → English only'),
              SizedBox(height: 16),
              Text('Tips:', style: TextStyle(fontWeight: FontWeight.bold)),
              Text('• Use Enter key for new lines'),
              Text('• Adjust typing speed if needed'),
              Text('• Check logs for troubleshooting'),
            ],
          ),
        ),
        actions: [
          TextButton(
            onPressed: () => Navigator.of(context).pop(),
            child: const Text('Close'),
          ),
        ],
      ),
    );
  }
}