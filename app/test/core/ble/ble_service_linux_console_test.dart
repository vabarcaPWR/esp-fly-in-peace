import 'package:flutter/foundation.dart';
import 'package:flutter_test/flutter_test.dart';

import 'package:fly_in_peace/core/ble/ble_service.dart';

void main() {
  group('BleService Linux telemetry mirror', () {
    test('enables telemetry mirror only for linux desktop runtime', () {
      expect(
        BleService.shouldMirrorTelemetryToLinuxConsole(
          isWeb: false,
          targetPlatform: TargetPlatform.linux,
        ),
        isTrue,
      );
      expect(
        BleService.shouldMirrorTelemetryToLinuxConsole(
          isWeb: false,
          targetPlatform: TargetPlatform.android,
        ),
        isFalse,
      );
      expect(
        BleService.shouldMirrorTelemetryToLinuxConsole(
          isWeb: true,
          targetPlatform: TargetPlatform.linux,
        ),
        isFalse,
      );
    });

    test('formats mirrored line with timestamp and source metadata', () {
      final DateTime timestamp = DateTime.utc(2026, 2, 28, 14, 13, 3, 530);
      final String formatted = BleService.formatLinuxTelemetryMirrorLine(
        timestamp: timestamp,
        sourceDeviceId: 'DC:DA:0C:81:52:26',
        sourceDeviceName: 'FlyInPeace',
        line: 'LK8EX1,101325,1234,15,24,999,*AB',
      );

      expect(
        formatted,
        '[2026-02-28T14:13:03.530Z] BLE RX DC:DA:0C:81:52:26 (FlyInPeace) -> LK8EX1,101325,1234,15,24,999,*AB',
      );
    });
  });
}
