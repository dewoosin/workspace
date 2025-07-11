import 'package:flutter/material.dart';

class ConnectionStatusCard extends StatelessWidget {
  final bool isConnected;
  final String deviceName;
  final String statusMessage;
  final VoidCallback onConnect;
  final VoidCallback onDisconnect;

  const ConnectionStatusCard({
    super.key,
    required this.isConnected,
    required this.deviceName,
    required this.statusMessage,
    required this.onConnect,
    required this.onDisconnect,
  });

  @override
  Widget build(BuildContext context) {
    return Card(
      elevation: 0,
      shape: RoundedRectangleBorder(
        borderRadius: BorderRadius.circular(16),
        side: BorderSide(
          color: isConnected
              ? Theme.of(context).colorScheme.primary.withOpacity(0.3)
              : Colors.grey.withOpacity(0.3),
          width: 1,
        ),
      ),
      child: Padding(
        padding: const EdgeInsets.all(20.0),
        child: Row(
          children: [
            // Status Icon
            Container(
              width: 48,
              height: 48,
              decoration: BoxDecoration(
                shape: BoxShape.circle,
                color: isConnected
                    ? Theme.of(context).colorScheme.primaryContainer
                    : Colors.grey[200],
              ),
              child: Icon(
                isConnected
                    ? Icons.bluetooth_connected_rounded
                    : Icons.bluetooth_disabled_rounded,
                color: isConnected
                    ? Theme.of(context).colorScheme.primary
                    : Colors.grey[600],
                size: 24,
              ),
            ),
            const SizedBox(width: 16),
            
            // Status Text
            Expanded(
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Text(
                    isConnected ? deviceName : 'Not Connected',
                    style: const TextStyle(
                      fontSize: 16,
                      fontWeight: FontWeight.w600,
                    ),
                  ),
                  const SizedBox(height: 4),
                  Text(
                    statusMessage,
                    style: TextStyle(
                      fontSize: 14,
                      color: Colors.grey[600],
                    ),
                  ),
                ],
              ),
            ),
            
            // Action Button
            FilledButton.tonal(
              onPressed: isConnected ? onDisconnect : onConnect,
              child: Text(isConnected ? 'Disconnect' : 'Connect'),
            ),
          ],
        ),
      ),
    );
  }
}