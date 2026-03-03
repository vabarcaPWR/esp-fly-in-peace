import 'package:fly_in_peace/core/ble/ble_service.dart';
import 'package:fly_in_peace/core/models/lk8ex1_data.dart';
import 'package:fly_in_peace/features/dashboard/dashboard_provider.dart';
import 'package:fly_in_peace/features/dashboard/dashboard_screen.dart';
import 'package:fly_in_peace/features/dashboard/model/dashboard_model.dart';
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:flutter_test/flutter_test.dart';

void main() {
  group('DashboardScreen widget', () {
    testWidgets('renders placeholders when no data is available', (
      WidgetTester tester,
    ) async {
      const DashboardModel model = DashboardModel(
        connectionStatus: BleConnectionStatus.disconnected,
        reconnectState: BleReconnectState.idle(),
        connectedDeviceName: null,
        latestData: null,
        isStale: false,
        lastUpdatedAt: null,
        parseWarningCount: 0,
        lastStreamError: null,
      );

      await _pumpDashboard(tester, model);

      expect(find.text('Disconnected'), findsOneWidget);
      expect(find.text('---'), findsOneWidget);
      expect(find.text('--'), findsNWidgets(3));
      expect(find.textContaining('Showing stale data.'), findsNothing);
    });

    testWidgets('renders connected live values', (WidgetTester tester) async {
      final Lk8ex1Data data = Lk8ex1Data(
        pressurePa: 101325,
        altitudeM: 1234,
        varioMs: 1.23,
        temperatureC: 21.5,
        batteryMv: 4100,
        timestamp: DateTime.utc(2026, 3, 3, 12, 0, 0),
      );

      final DashboardModel model = DashboardModel(
        connectionStatus: BleConnectionStatus.connected,
        reconnectState: const BleReconnectState.idle(),
        connectedDeviceName: 'FlyInPeace',
        latestData: data,
        isStale: false,
        lastUpdatedAt: data.timestamp,
        parseWarningCount: 0,
        lastStreamError: null,
      );

      await _pumpDashboard(tester, model);

      expect(find.text('FlyInPeace'), findsOneWidget);
      expect(find.text('1234'), findsOneWidget);
      expect(find.text('1.23'), findsOneWidget);
      expect(find.text('1013.25'), findsOneWidget);
      expect(find.text('21.5'), findsOneWidget);
      expect(find.textContaining('Showing stale data.'), findsNothing);
    });

    testWidgets('renders stale banner and disconnected overlay when needed', (
      WidgetTester tester,
    ) async {
      final DateTime lastUpdate = DateTime.utc(2026, 3, 3, 12, 0, 0);
      final Lk8ex1Data data = Lk8ex1Data(
        pressurePa: 101000,
        altitudeM: 900,
        varioMs: -0.5,
        temperatureC: 18.0,
        batteryMv: 4000,
        timestamp: lastUpdate,
      );

      final DashboardModel model = DashboardModel(
        connectionStatus: BleConnectionStatus.disconnected,
        reconnectState: const BleReconnectState.idle(),
        connectedDeviceName: 'FlyInPeace',
        latestData: data,
        isStale: true,
        lastUpdatedAt: lastUpdate,
        parseWarningCount: 0,
        lastStreamError: null,
      );

      await _pumpDashboard(tester, model);

      expect(find.textContaining('Showing stale data.'), findsOneWidget);
      expect(find.text('Disconnected'), findsNWidgets(2));
    });
  });
}

Future<void> _pumpDashboard(WidgetTester tester, DashboardModel model) async {
  await tester.pumpWidget(
    ProviderScope(
      overrides: [dashboardModelProvider.overrideWithValue(model)],
      child: const MaterialApp(home: DashboardScreen()),
    ),
  );
  await tester.pumpAndSettle();
}
