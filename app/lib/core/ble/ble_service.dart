import 'dart:async';

enum BleConnectionStatus { disconnected, connecting, connected, disconnecting }

class BleService {
  BleService();

  final StreamController<BleConnectionStatus> _statusController =
      StreamController<BleConnectionStatus>.broadcast();

  BleConnectionStatus _status = BleConnectionStatus.disconnected;

  BleConnectionStatus get status => _status;
  Stream<BleConnectionStatus> get statusStream => _statusController.stream;

  Future<void> connect() async {
    _setStatus(BleConnectionStatus.connecting);
    _setStatus(BleConnectionStatus.connected);
  }

  Future<void> disconnect() async {
    _setStatus(BleConnectionStatus.disconnecting);
    _setStatus(BleConnectionStatus.disconnected);
  }

  void _setStatus(BleConnectionStatus status) {
    _status = status;
    _statusController.add(_status);
  }

  void dispose() {
    _statusController.close();
  }
}
