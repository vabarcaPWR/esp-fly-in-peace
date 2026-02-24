import 'package:flutter_riverpod/flutter_riverpod.dart';

import 'ble_service.dart';

final bleServiceProvider = Provider<BleService>((ref) {
  final BleService service = BleService();
  ref.onDispose(service.dispose);
  return service;
});

final bleConnectionStatusProvider = StreamProvider<BleConnectionStatus>((ref) {
  final BleService service = ref.watch(bleServiceProvider);

  return Stream<BleConnectionStatus>.multi((controller) {
    controller.add(service.status);
    final subscription = service.statusStream.listen(
      controller.add,
      onError: controller.addError,
    );

    controller.onCancel = subscription.cancel;
  });
});

final bleReconnectStateProvider = StreamProvider<BleReconnectState>((ref) {
  final BleService service = ref.watch(bleServiceProvider);

  return Stream<BleReconnectState>.multi((controller) {
    controller.add(service.reconnectState);
    final subscription = service.reconnectStateStream.listen(
      controller.add,
      onError: controller.addError,
    );

    controller.onCancel = subscription.cancel;
  });
});
