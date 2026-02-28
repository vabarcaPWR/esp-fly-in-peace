import 'dart:async';
import 'dart:convert';

import 'package:flutter/foundation.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';

import 'nus_protocol.dart';

enum BleConnectionStatus { disconnected, connecting, connected, disconnecting }

enum BleCompatibilityProfile { flyInPeace, blueFlyVario, unsupported }

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
    required this.hasBlueFlyName,
    required this.hasBlueFlyService,
    required this.compatibilityProfile,
  });

  final BluetoothDevice device;
  final String remoteId;
  final String name;
  final int rssi;
  final bool hasNusService;
  final bool hasFlyInPeaceName;
  final bool hasBlueFlyName;
  final bool hasBlueFlyService;
  final BleCompatibilityProfile compatibilityProfile;

  bool get isFlyInPeaceCompatible {
    return compatibilityProfile != BleCompatibilityProfile.unsupported;
  }
}

class _BleTelemetryPipe {
  const _BleTelemetryPipe({
    required this.service,
    required this.txCharacteristic,
    required this.rxCharacteristic,
    required this.usesNus,
  });

  final BluetoothService service;
  final BluetoothCharacteristic txCharacteristic;
  final BluetoothCharacteristic rxCharacteristic;
  final bool usesNus;
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
    _legacyScanResultsSubscription = FlutterBluePlus.scanResults.listen(
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
  StreamSubscription<List<ScanResult>>? _legacyScanResultsSubscription;
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

  static BleCompatibilityProfile detectCompatibilityProfile(
    String name, {
    bool hasNusService = false,
    bool hasBlueFlyService = false,
  }) {
    final String normalizedName = name.toLowerCase();
    final bool hasFlyInPeaceName =
        normalizedName.contains('flyinpeace') ||
        normalizedName.contains('fly in peace');
    if (hasFlyInPeaceName) {
      return BleCompatibilityProfile.flyInPeace;
    }

    final bool hasBlueFlyName =
        normalizedName.contains('blueflyvario') ||
        normalizedName.contains('bluefly');
    if (hasBlueFlyName) {
      return BleCompatibilityProfile.blueFlyVario;
    }

    if (hasBlueFlyService) {
      return BleCompatibilityProfile.blueFlyVario;
    }

    if (hasNusService) {
      return BleCompatibilityProfile.flyInPeace;
    }

    return BleCompatibilityProfile.unsupported;
  }

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
      final BleCompatibilityProfile profile = _resolveProfileForDevice(device);
      final _BleTelemetryPipe telemetryPipe = _findTelemetryPipe(
        services: services,
        profile: profile,
      );

      _connectedDevice = device;
      _hasConnectedSession = true;
      _nusService = telemetryPipe.service;
      _nusTxCharacteristic = telemetryPipe.txCharacteristic;
      _nusRxCharacteristic = telemetryPipe.rxCharacteristic;
      await _subscribeToTxNotifications(telemetryPipe.txCharacteristic);
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
    try {
      await FlutterBluePlus.stopScan();
    } catch (_) {}

    if (_scanDevicesById.isNotEmpty) {
      final List<BleScanDevice> existingDevices =
          _scanDevicesById.values.toList()
            ..sort((a, b) => b.rssi.compareTo(a.rssi));
      _scanResultsController.add(existingDevices);
    }

    final bool isLinuxDesktop =
        !kIsWeb && defaultTargetPlatform == TargetPlatform.linux;
    if (isLinuxDesktop) {
      await FlutterBluePlus.startScan(timeout: timeout);
      return;
    }

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
      final bool hasBlueFlyService = result.advertisementData.serviceUuids.any((
        Guid guid,
      ) {
        final String uuid = guid.toString().toUpperCase();
        return NusProtocol.blueFlyServiceUuids.any(
          (String blueFlyUuid) => blueFlyUuid.toUpperCase() == uuid,
        );
      });

      final String normalizedName = chosenName.toLowerCase();
      final bool hasFlyInPeaceName =
          normalizedName.contains('flyinpeace') ||
          normalizedName.contains('fly in peace');
      final bool hasBlueFlyName =
          normalizedName.contains('blueflyvario') ||
          normalizedName.contains('bluefly');
      final BleCompatibilityProfile compatibilityProfile =
          detectCompatibilityProfile(
            chosenName,
            hasNusService: hasNusService,
            hasBlueFlyService: hasBlueFlyService,
          );

      final BleScanDevice? existing = _scanDevicesById[remoteId];
      _scanDevicesById[remoteId] = BleScanDevice(
        device: result.device,
        remoteId: remoteId,
        name: chosenName,
        rssi: result.rssi,
        hasNusService: existing?.hasNusService == true || hasNusService,
        hasFlyInPeaceName:
            existing?.hasFlyInPeaceName == true || hasFlyInPeaceName,
        hasBlueFlyName: existing?.hasBlueFlyName == true || hasBlueFlyName,
        hasBlueFlyService:
            existing?.hasBlueFlyService == true || hasBlueFlyService,
        compatibilityProfile: compatibilityProfile,
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
        'Failed to enable notifications on telemetry characteristic: $error',
      );
    }

    await _txNotificationSubscription?.cancel();
    _txNotificationSubscription = txCharacteristic.onValueReceived.listen(
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

  BleCompatibilityProfile _resolveProfileForDevice(BluetoothDevice device) {
    final BleScanDevice? scannedDevice = _scanDevicesById[device.remoteId.str];
    if (scannedDevice != null) {
      return scannedDevice.compatibilityProfile;
    }

    return detectCompatibilityProfile(device.platformName.trim());
  }

  _BleTelemetryPipe _findTelemetryPipe({
    required List<BluetoothService> services,
    required BleCompatibilityProfile profile,
  }) {
    if (services.isEmpty) {
      throw const BleServiceException(
        'No GATT services found on connected device.',
      );
    }

    final BluetoothService? nusService = _tryFindNusService(services);
    if (nusService != null) {
      return _buildNusTelemetryPipe(nusService);
    }

    if (profile == BleCompatibilityProfile.flyInPeace) {
      throw const BleServiceException(
        'NUS service not found on connected FlyInPeace device.',
      );
    }

    final _BleTelemetryPipe? genericPipe = _tryFindGenericTelemetryPipe(
      services,
    );
    if (genericPipe != null) {
      return genericPipe;
    }

    throw const BleServiceException(
      'No compatible BLE telemetry service found (NUS/generic UART).',
    );
  }

  BluetoothService? _tryFindNusService(List<BluetoothService> services) {
    for (final BluetoothService service in services) {
      if (_uuidEquals(service.uuid, NusProtocol.serviceUuid)) {
        return service;
      }
    }

    return null;
  }

  _BleTelemetryPipe _buildNusTelemetryPipe(BluetoothService service) {
    final BluetoothCharacteristic txCharacteristic = _findCharacteristic(
      service: service,
      uuid: NusProtocol.txCharacteristicUuid,
      roleName: 'TX (notify)',
    );
    final BluetoothCharacteristic rxCharacteristic = _findCharacteristic(
      service: service,
      uuid: NusProtocol.rxCharacteristicUuid,
      roleName: 'RX (write)',
    );

    return _BleTelemetryPipe(
      service: service,
      txCharacteristic: txCharacteristic,
      rxCharacteristic: rxCharacteristic,
      usesNus: true,
    );
  }

  _BleTelemetryPipe? _tryFindGenericTelemetryPipe(
    List<BluetoothService> services,
  ) {
    for (final BluetoothService service in services) {
      BluetoothCharacteristic? notifyCharacteristic;
      BluetoothCharacteristic? writeCharacteristic;

      for (final BluetoothCharacteristic characteristic
          in service.characteristics) {
        final CharacteristicProperties props = characteristic.properties;

        if (notifyCharacteristic == null && (props.notify || props.indicate)) {
          notifyCharacteristic = characteristic;
        }

        if (writeCharacteristic == null &&
            (props.write || props.writeWithoutResponse)) {
          writeCharacteristic = characteristic;
        }
      }

      if (notifyCharacteristic != null && writeCharacteristic != null) {
        return _BleTelemetryPipe(
          service: service,
          txCharacteristic: notifyCharacteristic,
          rxCharacteristic: writeCharacteristic,
          usesNus: false,
        );
      }
    }

    return null;
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
    _legacyScanResultsSubscription?.cancel();
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
