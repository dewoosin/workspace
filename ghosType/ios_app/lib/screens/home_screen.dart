import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:ghostype_ios/providers/ble_provider.dart';
import 'package:ghostype_ios/providers/text_conversion_provider.dart';
import 'package:ghostype_ios/widgets/connection_status_card.dart';
import 'package:ghostype_ios/widgets/text_input_section.dart';
import 'package:ghostype_ios/widgets/preview_section.dart';
import 'package:ghostype_ios/widgets/control_buttons.dart';
import 'package:ghostype_ios/screens/device_scan_screen.dart';
import 'package:ghostype_ios/screens/logs_screen.dart';
import 'package:ghostype_ios/screens/settings_screen.dart';

class HomeScreen extends StatelessWidget {
  const HomeScreen({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: Colors.grey[50],
      appBar: AppBar(
        title: const Text(
          'GHOSTYPE',
          style: TextStyle(
            fontWeight: FontWeight.bold,
            letterSpacing: 1.2,
          ),
        ),
        centerTitle: true,
        elevation: 0,
        actions: [
          IconButton(
            icon: const Icon(Icons.history_rounded),
            onPressed: () => _showLogs(context),
          ),
          IconButton(
            icon: const Icon(Icons.settings_rounded),
            onPressed: () => _showSettings(context),
          ),
        ],
      ),
      body: SafeArea(
        child: SingleChildScrollView(
          padding: const EdgeInsets.all(16.0),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              // Connection Status
              Consumer<BleProvider>(
                builder: (context, bleProvider, _) {
                  return ConnectionStatusCard(
                    isConnected: bleProvider.isConnected,
                    deviceName: bleProvider.deviceName,
                    statusMessage: bleProvider.statusMessage,
                    onConnect: () => _showDeviceScan(context),
                    onDisconnect: () => bleProvider.disconnect(),
                  );
                },
              ),
              const SizedBox(height: 20),
              
              // Text Input Section
              Consumer<BleProvider>(
                builder: (context, bleProvider, _) {
                  if (!bleProvider.isConnected) {
                    return const Center(
                      child: Padding(
                        padding: EdgeInsets.all(40.0),
                        child: Text(
                          'Connect to a GHOSTYPE device to start typing',
                          style: TextStyle(
                            color: Colors.grey,
                            fontSize: 16,
                          ),
                          textAlign: TextAlign.center,
                        ),
                      ),
                    );
                  }
                  
                  return Column(
                    children: [
                      const TextInputSection(),
                      const SizedBox(height: 16),
                      const PreviewSection(),
                      const SizedBox(height: 20),
                      const ControlButtons(),
                    ],
                  );
                },
              ),
            ],
          ),
        ),
      ),
    );
  }

  void _showDeviceScan(BuildContext context) {
    Navigator.push(
      context,
      MaterialPageRoute(
        builder: (context) => const DeviceScanScreen(),
      ),
    );
  }

  void _showLogs(BuildContext context) {
    Navigator.push(
      context,
      MaterialPageRoute(
        builder: (context) => const LogsScreen(),
      ),
    );
  }

  void _showSettings(BuildContext context) {
    Navigator.push(
      context,
      MaterialPageRoute(
        builder: (context) => const SettingsScreen(),
      ),
    );
  }
}