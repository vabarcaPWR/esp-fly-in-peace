import 'dart:async';

import 'package:flutter_blue_plus/flutter_blue_plus.dart';

import 'nus_protocol.dart';

enum BleConnectionStatus { disconnected, connecting, connected, disconnecting }

class BleScanDevice {
  const BleScanDevice({
    required this.device,
    required this.remoteId,
    required this.name,
    required this.rssi,
    required this.hasNusService,
  });

  final BluetoothDevice device;
  final String remoteId;
  final String name;
  final int rssi;
  final bool hasNusService;
}

class BleService {
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

  BleConnectionStatus _status = BleConnectionStatus.disconnected;

  BleConnectionStatus get status => _status;
  Stream<BleConnectionStatus> get statusStream => _statusController.stream;
  Stream<List<BleScanDevice>> get scanResults => _scanResultsController.stream;
  Stream<bool> get isScanning => _isScanningController.stream;

  Future<void> connect() async {
    _setStatus(BleConnectionStatus.connecting);
    _setStatus(BleConnectionStatus.connected);
  }

  Future<void> disconnect() async {
    _setStatus(BleConnectionStatus.disconnecting);
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

      if (!hasNusService && !hasFlyInPeaceName) {
        continue;
      }

      _scanDevicesById[remoteId] = BleScanDevice(
        device: result.device,
        remoteId: remoteId,
        name: chosenName,
        rssi: result.rssi,
        hasNusService: hasNusService,
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

  void dispose() {
    _scanResultsSubscription?.cancel();
    _isScanningSubscription?.cancel();
    _statusController.close();
    _scanResultsController.close();
    _isScanningController.close();
  }
}
