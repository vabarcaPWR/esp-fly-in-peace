import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../../core/ble/ble_providers.dart';
import '../../core/ble/ble_service.dart';

final dashboardConnectionStatusProvider = Provider<BleConnectionStatus>((ref) {
  final BleService service = ref.watch(bleServiceProvider);
  final AsyncValue<BleConnectionStatus> statusAsync = ref.watch(
    bleConnectionStatusProvider,
  );

  return statusAsync.value ?? service.status;
});

final dashboardConnectedProvider = Provider<bool>((ref) {
  final BleConnectionStatus status = ref.watch(
    dashboardConnectionStatusProvider,
  );
  return status == BleConnectionStatus.connected;
});

final dashboardReconnectStateProvider = Provider<BleReconnectState>((ref) {
  final BleService service = ref.watch(bleServiceProvider);
  final AsyncValue<BleReconnectState> reconnectAsync = ref.watch(
    bleReconnectStateProvider,
  );

  return reconnectAsync.value ?? service.reconnectState;
});

final dashboardShowDisconnectedOverlayProvider = Provider<bool>((ref) {
  final BleService service = ref.watch(bleServiceProvider);
  final BleConnectionStatus status = ref.watch(
    dashboardConnectionStatusProvider,
  );
  final BleReconnectState reconnectState = ref.watch(
    dashboardReconnectStateProvider,
  );

  return service.hasConnectedSession &&
      status == BleConnectionStatus.disconnected &&
      !reconnectState.isReconnecting;
});
