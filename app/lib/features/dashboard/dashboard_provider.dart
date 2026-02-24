import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../../core/ble/ble_providers.dart';
import '../../core/ble/ble_service.dart';

final dashboardConnectedProvider = Provider<bool>((ref) {
	final BleService service = ref.watch(bleServiceProvider);
	final AsyncValue<BleConnectionStatus> statusAsync = ref.watch(
		bleConnectionStatusProvider,
	);

	final BleConnectionStatus status = statusAsync.value ?? service.status;
	return status == BleConnectionStatus.connected;
});
