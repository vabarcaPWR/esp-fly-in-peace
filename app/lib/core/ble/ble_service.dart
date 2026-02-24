import 'dart:async';

import 'package:flutter_blue_plus/flutter_blue_plus.dart';

import 'nus_protocol.dart';

enum BleConnectionStatus { disconnected, connecting, connected, disconnecting }

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

  final Map<String, BleScanDevice> _scanDevicesById = <String, BleScanDevice>{};

  StreamSubscription<List<ScanResult>>? _scanResultsSubscription;
  StreamSubscription<bool>? _isScanningSubscription;
  StreamSubscription<BluetoothConnectionState>? _deviceConnectionSubscription;

  BleConnectionStatus _status = BleConnectionStatus.disconnected;
  BluetoothDevice? _connectedDevice;
  BluetoothService? _nusService;
  BluetoothCharacteristic? _nusTxCharacteristic;
  BluetoothCharacteristic? _nusRxCharacteristic;

  BleConnectionStatus get status => _status;
  Stream<BleConnectionStatus> get statusStream => _statusController.stream;
  Stream<List<BleScanDevice>> get scanResults => _scanResultsController.stream;
  Stream<bool> get isScanning => _isScanningController.stream;
  BluetoothDevice? get connectedDevice => _connectedDevice;
  BluetoothService? get nusService => _nusService;
  BluetoothCharacteristic? get nusTxCharacteristic => _nusTxCharacteristic;
  BluetoothCharacteristic? get nusRxCharacteristic => _nusRxCharacteristic;

  Future<void> connect(BluetoothDevice device) async {
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
      _nusService = nusService;
      _nusTxCharacteristic = txCharacteristic;
      _nusRxCharacteristic = rxCharacteristic;
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

  Future<void> disconnect() async {
    if (_status == BleConnectionStatus.disconnected) {
      return;
    }

    _setStatus(BleConnectionStatus.disconnecting);
    final BluetoothDevice? device = _connectedDevice;

    await _deviceConnectionSubscription?.cancel();
    _deviceConnectionSubscription = null;

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

    await disconnect();
  }

  Future<void> _attachConnectionStatusListener(BluetoothDevice device) async {
    await _deviceConnectionSubscription?.cancel();
    _deviceConnectionSubscription = device.connectionState.listen((state) {
      switch (state) {
        case BluetoothConnectionState.disconnected:
          _clearConnectionReferences();
          _setStatus(BleConnectionStatus.disconnected);
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

    throw const BleServiceException('NUS service not found on connected device.');
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

  Future<void> _resetConnectionState({required BluetoothDevice device}) async {
    await _deviceConnectionSubscription?.cancel();
    _deviceConnectionSubscription = null;

    try {
      await device.disconnect();
    } catch (_) {}

    _clearConnectionReferences();
  }

  void _clearConnectionReferences() {
    _connectedDevice = null;
    _nusService = null;
    _nusTxCharacteristic = null;
    _nusRxCharacteristic = null;
  }

  void dispose() {
    _scanResultsSubscription?.cancel();
    _isScanningSubscription?.cancel();
    _deviceConnectionSubscription?.cancel();
    _statusController.close();
    _scanResultsController.close();
    _isScanningController.close();
  }
}
