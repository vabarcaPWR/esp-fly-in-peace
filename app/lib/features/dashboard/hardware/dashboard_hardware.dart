import '../../../core/ble/ble_service.dart';

abstract class DashboardHardware {
  Stream<String> get telemetryLines;
  Stream<BleConnectionStatus> get connectionStatusStream;
  Stream<BleReconnectState> get reconnectStateStream;
  BleConnectionStatus get connectionStatus;
  BleReconnectState get reconnectState;
  String? get connectedDeviceName;
}

class BleDashboardHardware implements DashboardHardware {
  const BleDashboardHardware({required BleService bleService})
    : _bleService = bleService;

  final BleService _bleService;

  @override
  Stream<String> get telemetryLines => _bleService.receivedLines;

  @override
  Stream<BleConnectionStatus> get connectionStatusStream =>
      _bleService.statusStream;

  @override
  Stream<BleReconnectState> get reconnectStateStream =>
      _bleService.reconnectStateStream;

  @override
  BleConnectionStatus get connectionStatus => _bleService.status;

  @override
  BleReconnectState get reconnectState => _bleService.reconnectState;

  @override
  String? get connectedDeviceName => _bleService.connectedDevice?.platformName;
}
