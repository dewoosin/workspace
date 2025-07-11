import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:ghostype_ios/providers/ble_provider.dart';
import 'package:ghostype_ios/providers/text_conversion_provider.dart';
import 'package:intl/intl.dart';

class LogsScreen extends StatefulWidget {
  const LogsScreen({super.key});

  @override
  State<LogsScreen> createState() => _LogsScreenState();
}

class _LogsScreenState extends State<LogsScreen> with SingleTickerProviderStateMixin {
  late TabController _tabController;

  @override
  void initState() {
    super.initState();
    _tabController = TabController(length: 2, vsync: this);
  }

  @override
  void dispose() {
    _tabController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Logs & History'),
        elevation: 0,
        actions: [
          IconButton(
            icon: const Icon(Icons.clear_all_rounded),
            onPressed: () => _showClearDialog(context),
          ),
        ],
        bottom: TabBar(
          controller: _tabController,
          tabs: const [
            Tab(
              icon: Icon(Icons.bluetooth_rounded),
              text: 'BLE Logs',
            ),
            Tab(
              icon: Icon(Icons.history_rounded),
              text: 'Message History',
            ),
          ],
        ),
      ),
      body: TabBarView(
        controller: _tabController,
        children: [
          _buildBleLogsTab(),
          _buildMessageHistoryTab(),
        ],
      ),
    );
  }

  Widget _buildBleLogsTab() {
    return Consumer<BleProvider>(
      builder: (context, bleProvider, _) {
        if (bleProvider.logs.isEmpty) {
          return const Center(
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                Icon(
                  Icons.info_outline_rounded,
                  size: 64,
                  color: Colors.grey,
                ),
                SizedBox(height: 16),
                Text(
                  'No BLE logs yet',
                  style: TextStyle(
                    fontSize: 18,
                    color: Colors.grey,
                  ),
                ),
                SizedBox(height: 8),
                Text(
                  'Connect to a device to see logs',
                  style: TextStyle(
                    fontSize: 14,
                    color: Colors.grey,
                  ),
                ),
              ],
            ),
          );
        }

        return ListView.builder(
          padding: const EdgeInsets.all(16),
          itemCount: bleProvider.logs.length,
          itemBuilder: (context, index) {
            final log = bleProvider.logs[index];
            return _buildLogItem(log);
          },
        );
      },
    );
  }

  Widget _buildMessageHistoryTab() {
    return Consumer<TextConversionProvider>(
      builder: (context, textProvider, _) {
        if (textProvider.messageHistory.isEmpty) {
          return const Center(
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                Icon(
                  Icons.history_rounded,
                  size: 64,
                  color: Colors.grey,
                ),
                SizedBox(height: 16),
                Text(
                  'No message history yet',
                  style: TextStyle(
                    fontSize: 18,
                    color: Colors.grey,
                  ),
                ),
                SizedBox(height: 8),
                Text(
                  'Send some messages to see history',
                  style: TextStyle(
                    fontSize: 14,
                    color: Colors.grey,
                  ),
                ),
              ],
            ),
          );
        }

        return ListView.builder(
          padding: const EdgeInsets.all(16),
          itemCount: textProvider.messageHistory.length,
          itemBuilder: (context, index) {
            final message = textProvider.messageHistory[index];
            return _buildMessageHistoryItem(message);
          },
        );
      },
    );
  }

  Widget _buildLogItem(LogMessage log) {
    return Card(
      margin: const EdgeInsets.only(bottom: 8),
      child: ListTile(
        dense: true,
        leading: _getLogIcon(log.type),
        title: Text(
          log.message,
          style: const TextStyle(fontSize: 14),
        ),
        subtitle: Text(
          DateFormat('HH:mm:ss').format(log.timestamp),
          style: TextStyle(
            fontSize: 12,
            color: Colors.grey[600],
          ),
        ),
      ),
    );
  }

  Widget _buildMessageHistoryItem(MessageHistoryItem message) {
    return Card(
      margin: const EdgeInsets.only(bottom: 12),
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            // Header
            Row(
              children: [
                _getTypeIcon(message.type),
                const SizedBox(width: 8),
                Text(
                  _getTypeLabel(message.type),
                  style: const TextStyle(
                    fontSize: 12,
                    fontWeight: FontWeight.w600,
                  ),
                ),
                const Spacer(),
                Text(
                  DateFormat('MM/dd HH:mm').format(message.timestamp),
                  style: TextStyle(
                    fontSize: 12,
                    color: Colors.grey[600],
                  ),
                ),
              ],
            ),
            const SizedBox(height: 8),
            
            // Original Message
            Container(
              width: double.infinity,
              padding: const EdgeInsets.all(12),
              decoration: BoxDecoration(
                color: Colors.grey[50],
                borderRadius: BorderRadius.circular(8),
              ),
              child: Text(
                message.message,
                style: const TextStyle(fontSize: 14),
              ),
            ),
            
            // Converted Text (if different)
            if (message.convertedText != message.message) ...[
              const SizedBox(height: 8),
              Container(
                width: double.infinity,
                padding: const EdgeInsets.all(12),
                decoration: BoxDecoration(
                  color: Colors.blue[50],
                  borderRadius: BorderRadius.circular(8),
                ),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Text(
                      'QWERTY Keys:',
                      style: TextStyle(
                        fontSize: 12,
                        fontWeight: FontWeight.w600,
                        color: Colors.blue[700],
                      ),
                    ),
                    const SizedBox(height: 4),
                    Text(
                      message.convertedText,
                      style: const TextStyle(
                        fontSize: 14,
                        fontFamily: 'Courier',
                      ),
                    ),
                  ],
                ),
              ),
            ],
          ],
        ),
      ),
    );
  }

  Widget _getLogIcon(LogType type) {
    switch (type) {
      case LogType.info:
        return const Icon(Icons.info_outline_rounded, size: 20, color: Colors.blue);
      case LogType.success:
        return const Icon(Icons.check_circle_outline_rounded, size: 20, color: Colors.green);
      case LogType.warning:
        return const Icon(Icons.warning_amber_rounded, size: 20, color: Colors.orange);
      case LogType.error:
        return const Icon(Icons.error_outline_rounded, size: 20, color: Colors.red);
      case LogType.data:
        return const Icon(Icons.data_usage_rounded, size: 20, color: Colors.purple);
    }
  }

  Widget _getTypeIcon(TextType type) {
    switch (type) {
      case TextType.korean:
        return const Icon(Icons.translate_rounded, size: 16, color: Colors.blue);
      case TextType.english:
        return const Icon(Icons.abc_rounded, size: 16, color: Colors.green);
      case TextType.mixed:
        return const Icon(Icons.language_rounded, size: 16, color: Colors.orange);
    }
  }

  String _getTypeLabel(TextType type) {
    switch (type) {
      case TextType.korean:
        return 'KOREAN';
      case TextType.english:
        return 'ENGLISH';
      case TextType.mixed:
        return 'MIXED';
    }
  }

  void _showClearDialog(BuildContext context) {
    showDialog(
      context: context,
      builder: (context) => AlertDialog(
        title: const Text('Clear Logs'),
        content: const Text('Are you sure you want to clear all logs and history?'),
        actions: [
          TextButton(
            onPressed: () => Navigator.of(context).pop(),
            child: const Text('Cancel'),
          ),
          TextButton(
            onPressed: () {
              context.read<BleProvider>().clearLogs();
              // Note: TextConversionProvider doesn't have clearHistory method
              Navigator.of(context).pop();
            },
            child: const Text('Clear'),
          ),
        ],
      ),
    );
  }
}