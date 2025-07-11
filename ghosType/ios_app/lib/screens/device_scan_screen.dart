import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:provider/provider.dart';
import 'package:ghostype_ios/providers/ble_provider.dart';
import 'package:flutter_spinkit/flutter_spinkit.dart';

class DeviceScanScreen extends StatefulWidget {
  const DeviceScanScreen({super.key});

  @override
  State<DeviceScanScreen> createState() => _DeviceScanScreenState();
}

class _DeviceScanScreenState extends State<DeviceScanScreen> {
  @override
  void initState() {
    super.initState();
    // Start scanning when screen opens
    WidgetsBinding.instance.addPostFrameCallback((_) {
      context.read<BleProvider>().startScan();
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Connect Device'),
        elevation: 0,
      ),
      body: Consumer<BleProvider>(
        builder: (context, bleProvider, _) {
          return Column(
            children: [
              // Scanning indicator
              if (bleProvider.isScanning)
                Container(
                  padding: const EdgeInsets.all(20),
                  child: Column(
                    children: [
                      SpinKitPulse(
                        color: Theme.of(context).colorScheme.primary,
                        size: 50.0,
                      ),
                      const SizedBox(height: 16),
                      const Text(
                        'Searching for GHOSTYPE devices...',
                        style: TextStyle(fontSize: 16),
                      ),
                    ],
                  ),
                ),
              
              // Device list
              Expanded(
                child: bleProvider.scanResults.isEmpty && !bleProvider.isScanning
                    ? Center(
                        child: Column(
                          mainAxisAlignment: MainAxisAlignment.center,
                          children: [
                            Icon(
                              Icons.bluetooth_disabled_rounded,
                              size: 64,
                              color: Colors.grey[400],
                            ),
                            const SizedBox(height: 16),
                            Text(
                              'No GHOSTYPE devices found',
                              style: TextStyle(
                                fontSize: 18,
                                color: Colors.grey[600],
                              ),
                            ),
                            const SizedBox(height: 8),
                            Text(
                              'Make sure your device is powered on',
                              style: TextStyle(
                                fontSize: 14,
                                color: Colors.grey[500],
                              ),
                            ),
                            const SizedBox(height: 24),
                            ElevatedButton.icon(
                              onPressed: () => bleProvider.startScan(),
                              icon: const Icon(Icons.refresh),
                              label: const Text('Scan Again'),
                            ),
                          ],
                        ),
                      )
                    : ListView.builder(
                        itemCount: bleProvider.scanResults.length,
                        padding: const EdgeInsets.all(16),
                        itemBuilder: (context, index) {
                          final result = bleProvider.scanResults[index];
                          return Card(
                            margin: const EdgeInsets.only(bottom: 12),
                            child: ListTile(
                              leading: CircleAvatar(
                                backgroundColor: Theme.of(context).colorScheme.primaryContainer,
                                child: Icon(
                                  Icons.keyboard_rounded,
                                  color: Theme.of(context).colorScheme.primary,
                                ),
                              ),
                              title: Text(
                                result.device.platformName.isNotEmpty
                                    ? result.device.platformName
                                    : 'Unknown Device',
                                style: const TextStyle(
                                  fontWeight: FontWeight.w600,
                                ),
                              ),
                              subtitle: Text(
                                'Signal: ${result.rssi} dBm',
                                style: TextStyle(
                                  color: Colors.grey[600],
                                  fontSize: 13,
                                ),
                              ),
                              trailing: ElevatedButton(
                                onPressed: () => _connectToDevice(context, result.device),
                                child: const Text('Connect'),
                              ),
                            ),
                          );
                        },
                      ),
              ),
              
              // Control buttons
              if (!bleProvider.isScanning)
                Padding(
                  padding: const EdgeInsets.all(16.0),
                  child: Row(
                    children: [
                      Expanded(
                        child: OutlinedButton.icon(
                          onPressed: () => Navigator.pop(context),
                          icon: const Icon(Icons.close),
                          label: const Text('Cancel'),
                        ),
                      ),
                      const SizedBox(width: 12),
                      Expanded(
                        child: ElevatedButton.icon(
                          onPressed: () => bleProvider.startScan(),
                          icon: const Icon(Icons.refresh),
                          label: const Text('Refresh'),
                        ),
                      ),
                    ],
                  ),
                ),
            ],
          );
        },
      ),
    );
  }

  Future<void> _connectToDevice(BuildContext context, BluetoothDevice device) async {
    final bleProvider = context.read<BleProvider>();
    
    // Show connecting dialog
    showDialog(
      context: context,
      barrierDismissible: false,
      builder: (context) => AlertDialog(
        content: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            SpinKitDoubleBounce(
              color: Theme.of(context).colorScheme.primary,
              size: 50.0,
            ),
            const SizedBox(height: 20),
            Text('Connecting to ${device.platformName}...'),
          ],
        ),
      ),
    );

    // Attempt connection
    await bleProvider.connectToDevice(device);
    
    // Close dialog
    if (context.mounted) {
      Navigator.pop(context);
      
      // If connected, go back to home screen
      if (bleProvider.isConnected) {
        Navigator.pop(context);
      }
    }
  }
}