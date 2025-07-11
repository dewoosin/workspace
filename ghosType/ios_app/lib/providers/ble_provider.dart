import 'dart:async';
import 'dart:typed_data';
import 'package:flutter/foundation.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:permission_handler/permission_handler.dart';

class BleProvider extends ChangeNotifier {
  // BLE UUIDs - matching the ESP32 configuration
  static const String serviceUuid = '6e400001-b5a3-f393-e0a9-e50e24dcca9e';
  static const String rxCharUuid = '6e400002-b5a3-f393-e0a9-e50e24dcca9e';
  static const String txCharUuid = '6e400003-b5a3-f393-e0a9-e50e24dcca9e';

  // Connection state
  BluetoothDevice? _device;
  BluetoothCharacteristic? _rxCharacteristic;
  BluetoothCharacteristic? _txCharacteristic;
  StreamSubscription<BluetoothConnectionState>? _connectionStateSubscription;
  StreamSubscription<List<int>>? _notificationSubscription;

  // Scanning state
  bool _isScanning = false;
  final List<ScanResult> _scanResults = [];
  StreamSubscription<List<ScanResult>>? _scanSubscription;

  // Connection status
  bool _isConnected = false;
  String _statusMessage = 'Disconnected';
  String _deviceName = '';

  // Log messages
  final List<LogMessage> _logs = [];

  // Getters
  bool get isScanning => _isScanning;
  bool get isConnected => _isConnected;
  String get statusMessage => _statusMessage;
  String get deviceName => _deviceName;
  List<ScanResult> get scanResults => _scanResults;
  List<LogMessage> get logs => _logs;

  // Initialize
  BleProvider() {
    _checkPermissions();
  }

  Future<void> _checkPermissions() async {
    final permissions = [
      Permission.bluetooth,
      Permission.bluetoothScan,
      Permission.bluetoothConnect,
    ];

    for (final permission in permissions) {
      final status = await permission.status;
      if (!status.isGranted) {
        await permission.request();
      }
    }
  }

  void _addLog(String message, LogType type) {
    _logs.insert(0, LogMessage(
      message: message,
      type: type,
      timestamp: DateTime.now(),
    ));
    if (_logs.length > 100) {
      _logs.removeLast();
    }
    notifyListeners();
  }

  Future<void> startScan() async {
    if (_isScanning) return;

    _addLog('🔍 Searching for GHOSTYPE devices...', LogType.info);
    _updateStatus('Scanning...', false);
    
    _scanResults.clear();
    _isScanning = true;
    notifyListeners();

    try {
      // Start scanning
      await FlutterBluePlus.startScan(
        timeout: const Duration(seconds: 10),
        withServices: [Guid(serviceUuid)],
      );

      // Listen to scan results
      _scanSubscription = FlutterBluePlus.scanResults.listen((results) {
        _scanResults.clear();
        for (final result in results) {
          if (result.device.platformName.contains('GHOSTYPE')) {
            _scanResults.add(result);
          }
        }
        notifyListeners();
      });

      // Wait for scan to complete
      await Future.delayed(const Duration(seconds: 10));
      stopScan();
      
    } catch (e) {
      _addLog('❌ Scan failed: $e', LogType.error);
      stopScan();
    }
  }

  void stopScan() {
    if (!_isScanning) return;
    
    FlutterBluePlus.stopScan();
    _scanSubscription?.cancel();
    _isScanning = false;
    
    if (_scanResults.isEmpty) {
      _addLog('No devices found', LogType.warning);
    } else {
      _addLog('Found ${_scanResults.length} device(s)', LogType.success);
    }
    
    notifyListeners();
  }

  Future<void> connectToDevice(BluetoothDevice device) async {
    try {
      _device = device;
      _deviceName = device.platformName;
      
      _addLog('📱 Connecting to ${device.platformName}...', LogType.info);
      _updateStatus('Connecting...', false);

      // Connect to device
      await device.connect(
        timeout: const Duration(seconds: 10),
        autoConnect: false,
      );

      // Listen to connection state
      _connectionStateSubscription = device.connectionState.listen((state) {
        if (state == BluetoothConnectionState.disconnected) {
          _onDisconnected();
        }
      });

      // Discover services
      _addLog('🔍 Discovering services...', LogType.info);
      final services = await device.discoverServices();

      // Find our service
      for (final service in services) {
        if (service.uuid.toString() == serviceUuid) {
          // Find characteristics
          for (final char in service.characteristics) {
            if (char.uuid.toString() == rxCharUuid) {
              _rxCharacteristic = char;
            } else if (char.uuid.toString() == txCharUuid) {
              _txCharacteristic = char;
              
              // Subscribe to notifications
              await char.setNotifyValue(true);
              _notificationSubscription = char.lastValueStream.listen(_onDataReceived);
            }
          }
        }
      }

      if (_rxCharacteristic != null && _txCharacteristic != null) {
        _isConnected = true;
        _updateStatus('Connected to $_deviceName', true);
        _addLog('🎉 Connected! Ready for Korean/English input', LogType.success);
      } else {
        throw Exception('Required characteristics not found');
      }

    } catch (e) {
      _addLog('❌ Connection failed: $e', LogType.error);
      _updateStatus('Connection failed', false);
      await disconnect();
    }
    
    notifyListeners();
  }

  Future<void> disconnect() async {
    if (_device != null) {
      await _device!.disconnect();
    }
    _onDisconnected();
  }

  void _onDisconnected() {
    _connectionStateSubscription?.cancel();
    _notificationSubscription?.cancel();
    
    _device = null;
    _rxCharacteristic = null;
    _txCharacteristic = null;
    _isConnected = false;
    _deviceName = '';
    
    _updateStatus('Disconnected', false);
    _addLog('👋 Device disconnected', LogType.info);
    
    notifyListeners();
  }

  void _onDataReceived(List<int> data) {
    final message = String.fromCharCodes(data);
    _addLog('📨 ESP32: "$message"', LogType.data);
  }

  Future<bool> sendData(String message) async {
    if (_rxCharacteristic == null || !_isConnected) {
      _addLog('❌ Not connected', LogType.error);
      return false;
    }

    try {
      final bytes = Uint8List.fromList(message.codeUnits);
      await _rxCharacteristic!.write(bytes);
      _addLog('📤 Sent: "$message"', LogType.success);
      return true;
    } catch (e) {
      _addLog('❌ Send failed: $e', LogType.error);
      return false;
    }
  }

  void _updateStatus(String message, bool connected) {
    _statusMessage = message;
    _isConnected = connected;
    notifyListeners();
  }

  void clearLogs() {
    _logs.clear();
    notifyListeners();
  }

  @override
  void dispose() {
    disconnect();
    super.dispose();
  }
}

enum LogType { info, success, warning, error, data }

class LogMessage {
  final String message;
  final LogType type;
  final DateTime timestamp;

  LogMessage({
    required this.message,
    required this.type,
    required this.timestamp,
  });
}