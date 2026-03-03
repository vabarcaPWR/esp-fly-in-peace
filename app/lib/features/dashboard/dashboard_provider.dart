import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../../core/ble/ble_providers.dart';
import '../../core/ble/ble_service.dart';
import '../../core/utils/lk8ex1_parser.dart';
import 'conductor/dashboard_conductor.dart';
import 'hardware/dashboard_hardware.dart';
import 'model/dashboard_model.dart';

final dashboardHardwareProvider = Provider<DashboardHardware>((ref) {
  return BleDashboardHardware(bleService: ref.watch(bleServiceProvider));
});

final dashboardConductorProvider =
    StateNotifierProvider<DashboardConductor, DashboardModel>((ref) {
      return DashboardConductor(
        hardware: ref.watch(dashboardHardwareProvider),
        parser: const Lk8ex1Parser(),
      );
    });

final dashboardModelProvider = Provider<DashboardModel>((ref) {
  return ref.watch(dashboardConductorProvider);
});

final dashboardConnectionStatusProvider = Provider<BleConnectionStatus>((ref) {
  return ref.watch(dashboardModelProvider).connectionStatus;
});

final dashboardConnectedProvider = Provider<bool>((ref) {
  return ref.watch(dashboardModelProvider).isConnected;
});

final dashboardReconnectStateProvider = Provider<BleReconnectState>((ref) {
  return ref.watch(dashboardModelProvider).reconnectState;
});

final dashboardShowDisconnectedOverlayProvider = Provider<bool>((ref) {
  final DashboardModel dashboardModel = ref.watch(dashboardModelProvider);
  return dashboardModel.connectionStatus == BleConnectionStatus.disconnected &&
      !dashboardModel.reconnectState.isReconnecting &&
      dashboardModel.hasData;
});
