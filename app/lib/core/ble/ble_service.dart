import 'dart:async';
import 'dart:convert';

import 'package:flutter/foundation.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';

import 'nus_protocol.dart';

enum BleConnectionStatus { disconnected, connecting, connected, disconnecting }

class BleReconnectState {
  const BleReconnectState({
    required this.isReconnecting,
    required this.attempt,
    required this.maxAttempts,
    required this.nextRetryDelay,
  });

  const BleReconnectState.idle()
    : isReconnecting = false,
      attempt = 0,
      maxAttempts = 0,
      nextRetryDelay = Duration.zero;

  final bool isReconnecting;
  final int attempt;
  final int maxAttempts;
  final Duration nextRetryDelay;
}

class BleServiceException implements Exception {
  const BleServiceException(this.message);

  final String message;

  @override
  String toString() => message;
}

class BleScanDevice {
  const BleScanDevice({
    required this.device,
    required this.remoteId,
    required this.name,
    required this.rssi,
    required this.hasNusService,
    required this.hasFlyInPeaceName,
    required this.isFlyInPeaceCompatible,
  });

  final BluetoothDevice device;
  final String remoteId;
  final String name;
  final int rssi;
  final bool hasNusService;
  final bool hasFlyInPeaceName;
  final bool isFlyInPeaceCompatible;
}

class BleService {
  static const Duration connectionTimeout = Duration(seconds: 10);
  static const List<Duration> reconnectBackoffDelays = <Duration>[
    Duration(seconds: 2),
    Duration(seconds: 4),
    Duration(seconds: 8),
    Duration(seconds: 16),
    Duration(seconds: 30),
  ];

  BleService() {
    _scanResultsSubscription = FlutterBluePlus.onScanResults.listen(
      _handleScanResults,
      onError: (Object error, StackTrace stackTrace) {
        _scanResultsController.addError(error, stackTrace);
      },
    );
    _isScanningSubscription = FlutterBluePlus.isScanning.listen(
      _isScanningController.add,
    );
  }

  final StreamController<BleConnectionStatus> _statusController =
      StreamController<BleConnectionStatus>.broadcast();
  final StreamController<List<BleScanDevice>> _scanResultsController =
      StreamController<List<BleScanDevice>>.broadcast();
  final StreamController<bool> _isScanningController =
      StreamController<bool>.broadcast();
  final StreamController<String> _receivedLinesController =
      StreamController<String>.broadcast();
  final StreamController<BleReconnectState> _reconnectStateController =
      StreamController<BleReconnectState>.broadcast();

  final Map<String, BleScanDevice> _scanDevicesById = <String, BleScanDevice>{};

  StreamSubscription<List<ScanResult>>? _scanResultsSubscription;
  StreamSubscription<bool>? _isScanningSubscription;
  StreamSubscription<BluetoothConnectionState>? _deviceConnectionSubscription;
  StreamSubscription<List<int>>? _txNotificationSubscription;

  BleConnectionStatus _status = BleConnectionStatus.disconnected;
  BluetoothDevice? _connectedDevice;
  BluetoothService? _nusService;
  BluetoothCharacteristic? _nusTxCharacteristic;
  BluetoothCharacteristic? _nusRxCharacteristic;
  final List<int> _txRxBuffer = <int>[];
  BleReconnectState _reconnectState = const BleReconnectState.idle();
  bool _manualDisconnectRequested = false;
  bool _isAutoReconnectInProgress = false;
  int _reconnectSessionId = 0;
  BluetoothDevice? _reconnectTargetDevice;
  bool _hasConnectedSession = false;

  BleConnectionStatus get status => _status;
  Stream<BleConnectionStatus> get statusStream => _statusController.stream;
  Stream<List<BleScanDevice>> get scanResults => _scanResultsController.stream;
  Stream<bool> get isScanning => _isScanningController.stream;
  Stream<String> get receivedLines => _receivedLinesController.stream;
  Stream<BleReconnectState> get reconnectStateStream =>
      _reconnectStateController.stream;
  BluetoothDevice? get connectedDevice => _connectedDevice;
  BleReconnectState get reconnectState => _reconnectState;
  bool get hasConnectedSession => _hasConnectedSession;
  BluetoothService? get nusService => _nusService;
  BluetoothCharacteristic? get nusTxCharacteristic => _nusTxCharacteristic;
  BluetoothCharacteristic? get nusRxCharacteristic => _nusRxCharacteristic;

  Future<void> connect(BluetoothDevice device) async {
    _manualDisconnectRequested = false;
    _reconnectTargetDevice = device;

    if (_connectedDevice?.remoteId == device.remoteId &&
        _status == BleConnectionStatus.connected &&
        _nusService != null &&
        _nusTxCharacteristic != null &&
        _nusRxCharacteristic != null) {
      return;
    }

    _setStatus(BleConnectionStatus.connecting);

    try {
      await _disconnectCurrentDeviceIfDifferent(targetDevice: device);

      final BluetoothConnectionState currentState =
          await device.connectionState.first;
      if (currentState != BluetoothConnectionState.connected) {
        await device.connect(timeout: connectionTimeout);
      }

      await _attachConnectionStatusListener(device);
      await _negotiateMtu(device);

      final List<BluetoothService> services = await device.discoverServices();
      final BluetoothService nusService = _findNusService(services);
      final BluetoothCharacteristic txCharacteristic = _findCharacteristic(
        service: nusService,
        uuid: NusProtocol.txCharacteristicUuid,
        roleName: 'TX (notify)',
      );
      final BluetoothCharacteristic rxCharacteristic = _findCharacteristic(
        service: nusService,
        uuid: NusProtocol.rxCharacteristicUuid,
        roleName: 'RX (write)',
      );

      _connectedDevice = device;
      _hasConnectedSession = true;
      _nusService = nusService;
      _nusTxCharacteristic = txCharacteristic;
      _nusRxCharacteristic = rxCharacteristic;
      await _subscribeToTxNotifications(txCharacteristic);
      _setReconnectState(const BleReconnectState.idle());
      _setStatus(BleConnectionStatus.connected);
    } catch (error) {
      await _resetConnectionState(device: device);
      _setStatus(BleConnectionStatus.disconnected);

      if (error is BleServiceException) {
        rethrow;
      }
      throw BleServiceException('Failed to connect to device: $error');
    }
  }

  Future<void> disconnect({bool manual = true}) async {
    if (manual) {
      _manualDisconnectRequested = true;
      _reconnectTargetDevice = null;
      _cancelAutoReconnect();
      _setReconnectState(const BleReconnectState.idle());
    }

    if (_status == BleConnectionStatus.disconnected) {
      return;
    }

    _setStatus(BleConnectionStatus.disconnecting);
    final BluetoothDevice? device = _connectedDevice;

    await _deviceConnectionSubscription?.cancel();
    _deviceConnectionSubscription = null;
    await _txNotificationSubscription?.cancel();
    _txNotificationSubscription = null;

    if (device != null) {
      try {
        await device.disconnect();
      } catch (_) {}
    }

    _clearConnectionReferences();
    _setStatus(BleConnectionStatus.disconnected);
  }

  Future<void> startScan({
    Duration timeout = const Duration(seconds: 10),
  }) async {
    _scanDevicesById.clear();
    _scanResultsController.add(const <BleScanDevice>[]);

    await FlutterBluePlus.startScan(
      timeout: timeout,
      continuousUpdates: true,
      continuousDivisor: 1,
      androidUsesFineLocation: true,
      androidCheckLocationServices: true,
    );
  }

  Future<void> stopScan() async {
    await FlutterBluePlus.stopScan();
  }

  Future<void> sendCommand(String command) async {
    final BluetoothCharacteristic? rxCharacteristic = _nusRxCharacteristic;
    final BluetoothDevice? device = _connectedDevice;

    if (device == null ||
        _status != BleConnectionStatus.connected ||
        rxCharacteristic == null) {
      throw const BleServiceException(
        'Cannot send command: device is not connected.',
      );
    }

    final String normalizedCommand = command.endsWith('\n')
        ? command
        : '$command\n';
    final List<int> commandBytes = utf8.encode(normalizedCommand);
    final int mtu = device.mtuNow;
    final int chunkSize = _calculateWriteChunkSize(mtu);
    final bool withoutResponse =
        rxCharacteristic.properties.writeWithoutResponse;

    for (int offset = 0; offset < commandBytes.length; offset += chunkSize) {
      final int end = (offset + chunkSize > commandBytes.length)
          ? commandBytes.length
          : offset + chunkSize;

      await rxCharacteristic.write(
        commandBytes.sublist(offset, end),
        withoutResponse: withoutResponse,
      );
    }

    debugPrint('BLE RX -> ${normalizedCommand.trimRight()}');
  }

  void _handleScanResults(List<ScanResult> results) {
    for (final ScanResult result in results) {
      final String remoteId = result.device.remoteId.str;

      final String platformName = result.device.platformName.trim();
      final String advertisementName = result.advertisementData.advName.trim();
      final String chosenName = platformName.isNotEmpty
          ? platformName
          : (advertisementName.isNotEmpty ? advertisementName : 'Unknown');

      final bool hasNusService = result.advertisementData.serviceUuids.any(
        (Guid guid) => guid.toString().toUpperCase() == NusProtocol.serviceUuid,
      );

      final String normalizedName = chosenName.toLowerCase();
      final bool hasFlyInPeaceName =
          normalizedName.contains('flyinpeace') ||
          normalizedName.contains('fly in peace');
      final bool isFlyInPeaceCompatible = hasNusService || hasFlyInPeaceName;

      _scanDevicesById[remoteId] = BleScanDevice(
        device: result.device,
        remoteId: remoteId,
        name: chosenName,
        rssi: result.rssi,
        hasNusService: hasNusService,
        hasFlyInPeaceName: hasFlyInPeaceName,
        isFlyInPeaceCompatible: isFlyInPeaceCompatible,
      );
    }

    final List<BleScanDevice> devices = _scanDevicesById.values.toList()
      ..sort((a, b) => b.rssi.compareTo(a.rssi));
    _scanResultsController.add(devices);
  }

  void _setStatus(BleConnectionStatus status) {
    _status = status;
    _statusController.add(_status);
  }

  Future<void> _disconnectCurrentDeviceIfDifferent({
    required BluetoothDevice targetDevice,
  }) async {
    if (_connectedDevice == null) {
      return;
    }

    if (_connectedDevice!.remoteId == targetDevice.remoteId) {
      return;
    }

    await disconnect(manual: false);
  }

  Future<void> _attachConnectionStatusListener(BluetoothDevice device) async {
    await _deviceConnectionSubscription?.cancel();
    _deviceConnectionSubscription = device.connectionState.listen((state) {
      switch (state) {
        case BluetoothConnectionState.disconnected:
          final bool manualDisconnect =
              _manualDisconnectRequested ||
              _status == BleConnectionStatus.disconnecting;
          final BluetoothDevice reconnectDevice =
              _reconnectTargetDevice ?? _connectedDevice ?? device;

          _clearConnectionReferences();
          _setStatus(BleConnectionStatus.disconnected);

          if (!manualDisconnect) {
            _startAutoReconnect(reconnectDevice);
          }
          break;
        case BluetoothConnectionState.connected:
          if (_status != BleConnectionStatus.connecting) {
            _setStatus(BleConnectionStatus.connected);
          }
          break;
        default:
          break;
      }
    });
  }

  Future<void> _subscribeToTxNotifications(
    BluetoothCharacteristic txCharacteristic,
  ) async {
    try {
      await txCharacteristic.setNotifyValue(true);
    } catch (error) {
      throw BleServiceException(
        'Failed to enable TX notifications on NUS characteristic: $error',
      );
    }

    await _txNotificationSubscription?.cancel();
    _txNotificationSubscription = txCharacteristic.lastValueStream.listen(
      _handleTxNotificationValue,
      onError: (Object error, StackTrace stackTrace) {
        _receivedLinesController.addError(
          BleServiceException('Failed to process TX notifications: $error'),
          stackTrace,
        );
      },
    );
  }

  void _handleTxNotificationValue(List<int> value) {
    if (value.isEmpty) {
      return;
    }

    _txRxBuffer.addAll(value);
    _emitCompletedLinesFromBuffer();
  }

  void _emitCompletedLinesFromBuffer() {
    const int carriageReturn = 13;
    const int lineFeed = 10;

    while (true) {
      int delimiterIndex = -1;
      for (int index = 0; index < _txRxBuffer.length - 1; index++) {
        if (_txRxBuffer[index] == carriageReturn &&
            _txRxBuffer[index + 1] == lineFeed) {
          delimiterIndex = index;
          break;
        }
      }

      if (delimiterIndex < 0) {
        return;
      }

      final List<int> lineBytes = _txRxBuffer.sublist(0, delimiterIndex);
      _txRxBuffer.removeRange(0, delimiterIndex + 2);

      final String line = utf8.decode(lineBytes, allowMalformed: true);
      if (line.isNotEmpty) {
        _receivedLinesController.add(line);
      }
    }
  }

  Future<void> _negotiateMtu(BluetoothDevice device) async {
    try {
      await device.requestMtu(512);
    } catch (_) {}
  }

  BluetoothService _findNusService(List<BluetoothService> services) {
    for (final BluetoothService service in services) {
      if (_uuidEquals(service.uuid, NusProtocol.serviceUuid)) {
        return service;
      }
    }

    throw const BleServiceException(
      'NUS service not found on connected device.',
    );
  }

  BluetoothCharacteristic _findCharacteristic({
    required BluetoothService service,
    required String uuid,
    required String roleName,
  }) {
    for (final BluetoothCharacteristic characteristic
        in service.characteristics) {
      if (_uuidEquals(characteristic.uuid, uuid)) {
        return characteristic;
      }
    }

    throw BleServiceException(
      'NUS characteristic $roleName not found on connected device.',
    );
  }

  bool _uuidEquals(Guid guid, String expectedUuid) {
    return guid.toString().toUpperCase() == expectedUuid.toUpperCase();
  }

  int _calculateWriteChunkSize(int mtu) {
    final int payloadSize = mtu - 3;
    if (payloadSize <= 0) {
      return 20;
    }

    return payloadSize;
  }

  Future<void> _resetConnectionState({required BluetoothDevice device}) async {
    await _deviceConnectionSubscription?.cancel();
    _deviceConnectionSubscription = null;
    await _txNotificationSubscription?.cancel();
    _txNotificationSubscription = null;

    try {
      await device.disconnect();
    } catch (_) {}

    _clearConnectionReferences();
  }

  void _setReconnectState(BleReconnectState reconnectState) {
    _reconnectState = reconnectState;
    _reconnectStateController.add(reconnectState);
  }

  void _cancelAutoReconnect() {
    _reconnectSessionId++;
    _isAutoReconnectInProgress = false;
  }

  void _startAutoReconnect(BluetoothDevice device) {
    if (_isAutoReconnectInProgress) {
      return;
    }

    _isAutoReconnectInProgress = true;
    final int sessionId = ++_reconnectSessionId;

    unawaited(_runAutoReconnectLoop(sessionId: sessionId, device: device));
  }

  Future<void> _runAutoReconnectLoop({
    required int sessionId,
    required BluetoothDevice device,
  }) async {
    final int maxAttempts = reconnectBackoffDelays.length;

    for (int index = 0; index < maxAttempts; index++) {
      if (_shouldStopReconnect(sessionId)) {
        _isAutoReconnectInProgress = false;
        return;
      }

      final Duration retryDelay = reconnectBackoffDelays[index];
      _setReconnectState(
        BleReconnectState(
          isReconnecting: true,
          attempt: index + 1,
          maxAttempts: maxAttempts,
          nextRetryDelay: retryDelay,
        ),
      );

      await Future<void>.delayed(retryDelay);

      if (_shouldStopReconnect(sessionId)) {
        _isAutoReconnectInProgress = false;
        return;
      }

      try {
        await connect(device);
        _isAutoReconnectInProgress = false;
        _setReconnectState(const BleReconnectState.idle());
        return;
      } catch (_) {}
    }

    _isAutoReconnectInProgress = false;
    _setReconnectState(const BleReconnectState.idle());
  }

  bool _shouldStopReconnect(int sessionId) {
    return _manualDisconnectRequested || sessionId != _reconnectSessionId;
  }

  void _clearConnectionReferences() {
    _connectedDevice = null;
    _nusService = null;
    _nusTxCharacteristic = null;
    _nusRxCharacteristic = null;
    _txRxBuffer.clear();
  }

  void dispose() {
    _scanResultsSubscription?.cancel();
    _isScanningSubscription?.cancel();
    _deviceConnectionSubscription?.cancel();
    _txNotificationSubscription?.cancel();
    _statusController.close();
    _scanResultsController.close();
    _isScanningController.close();
    _receivedLinesController.close();
    _reconnectStateController.close();
  }
}
