import 'dart:async';

import 'package:flutter_test/flutter_test.dart';
import 'package:fly_in_peace/core/ble/ble_service.dart';
import 'package:fly_in_peace/core/utils/lk8ex1_parser.dart';
import 'package:fly_in_peace/features/dashboard/conductor/dashboard_conductor.dart';
import 'package:fly_in_peace/features/dashboard/hardware/dashboard_hardware.dart';
import 'package:fly_in_peace/features/dashboard/model/dashboard_model.dart';

void main() {
  group('DashboardConductor', () {
    test('updates dashboard data from valid telemetry line', () async {
      final _FakeDashboardHardware hardware = _FakeDashboardHardware();
      final DashboardConductor conductor = DashboardConductor(
        hardware: hardware,
        parser: const Lk8ex1Parser(),
      );
      addTearDown(() {
        conductor.dispose();
        hardware.dispose();
      });

      hardware.emitConnectionStatus(BleConnectionStatus.connected);
      hardware.emitTelemetryLine(
        _buildSentence('LK8EX1,101325,1200,120,215,4100,'),
      );
      await Future<void>.delayed(Duration.zero);

      final DashboardModel state = conductor.state;
      expect(state.isConnected, isTrue);
      expect(state.latestData, isNotNull);
      expect(state.latestData!.pressurePa, 101325);
      expect(state.latestData!.altitudeM, 1200);
      expect(state.latestData!.varioMs, 1.2);
      expect(state.isStale, isFalse);
    });

    test(
      'marks values as stale when disconnected after receiving data',
      () async {
        final _FakeDashboardHardware hardware = _FakeDashboardHardware();
        final DashboardConductor conductor = DashboardConductor(
          hardware: hardware,
          parser: const Lk8ex1Parser(),
        );
        addTearDown(() {
          conductor.dispose();
          hardware.dispose();
        });

        hardware.emitConnectionStatus(BleConnectionStatus.connected);
        hardware.emitTelemetryLine(
          _buildSentence('LK8EX1,101325,1300,20,210,4100,'),
        );
        await Future<void>.delayed(Duration.zero);

        hardware.emitConnectionStatus(BleConnectionStatus.disconnected);
        await Future<void>.delayed(Duration.zero);

        expect(conductor.state.latestData, isNotNull);
        expect(conductor.state.isStale, isTrue);
        expect(
          conductor.state.connectionStatus,
          BleConnectionStatus.disconnected,
        );
      },
    );

    test(
      'keeps running and counts parse warnings on invalid telemetry',
      () async {
        final _FakeDashboardHardware hardware = _FakeDashboardHardware();
        final DashboardConductor conductor = DashboardConductor(
          hardware: hardware,
          parser: const Lk8ex1Parser(),
        );
        addTearDown(() {
          conductor.dispose();
          hardware.dispose();
        });

        hardware.emitTelemetryLine('INVALID_FRAME');
        await Future<void>.delayed(Duration.zero);

        expect(conductor.state.parseWarningCount, 1);
        expect(conductor.state.latestData, isNull);
        expect(conductor.state.lastStreamError, isNotNull);

        hardware.emitTelemetryLine(
          _buildSentence('LK8EX1,100900,800,-30,180,4050,'),
        );
        await Future<void>.delayed(Duration.zero);

        expect(conductor.state.latestData, isNotNull);
        expect(conductor.state.lastStreamError, isNull);
      },
    );
  });

  group('DashboardFormatter', () {
    test('formats altitude placeholder as no data', () {
      final model = DashboardModel.initial(
        connectionStatus: BleConnectionStatus.disconnected,
        reconnectState: const BleReconnectState.idle(),
      );

      expect(DashboardFormatter.formatAltitudeMeters(model.latestData), '---');
    });

    test('converts pressure from Pa to hPa', () {
      final data = const Lk8ex1Parser().parseLk8ex1(
        _buildSentence('LK8EX1,101325,500,0,200,4100,'),
      );

      expect(DashboardFormatter.formatPressureHpa(data), '1013.25');
    });

    test('resolves vario trend with dead zone', () {
      expect(
        DashboardFormatter.resolveVarioTrend(0.0),
        DashboardVarioTrend.neutral,
      );
      expect(
        DashboardFormatter.resolveVarioTrend(0.2),
        DashboardVarioTrend.climb,
      );
      expect(
        DashboardFormatter.resolveVarioTrend(-0.2),
        DashboardVarioTrend.sink,
      );
    });
  });
}

class _FakeDashboardHardware implements DashboardHardware {
  final StreamController<String> _telemetryController =
      StreamController<String>.broadcast();
  final StreamController<BleConnectionStatus> _connectionController =
      StreamController<BleConnectionStatus>.broadcast();
  final StreamController<BleReconnectState> _reconnectController =
      StreamController<BleReconnectState>.broadcast();

  BleConnectionStatus _status = BleConnectionStatus.disconnected;
  BleReconnectState _reconnectState = const BleReconnectState.idle();
  String? _connectedDeviceName;

  @override
  Stream<String> get telemetryLines => _telemetryController.stream;

  @override
  Stream<BleConnectionStatus> get connectionStatusStream =>
      _connectionController.stream;

  @override
  Stream<BleReconnectState> get reconnectStateStream =>
      _reconnectController.stream;

  @override
  BleConnectionStatus get connectionStatus => _status;

  @override
  BleReconnectState get reconnectState => _reconnectState;

  @override
  String? get connectedDeviceName => _connectedDeviceName;

  void emitTelemetryLine(String line) {
    _telemetryController.add(line);
  }

  void emitConnectionStatus(BleConnectionStatus status, {String? deviceName}) {
    _status = status;
    _connectedDeviceName = deviceName;
    _connectionController.add(status);
  }

  void emitReconnectState(BleReconnectState reconnectState) {
    _reconnectState = reconnectState;
    _reconnectController.add(reconnectState);
  }

  void dispose() {
    _telemetryController.close();
    _connectionController.close();
    _reconnectController.close();
  }
}

String _buildSentence(String payload) {
  int checksum = 0;
  for (final int codeUnit in payload.codeUnits) {
    checksum ^= codeUnit;
  }
  final String checksumHex = checksum
      .toRadixString(16)
      .toUpperCase()
      .padLeft(2, '0');
  return '\$$payload*$checksumHex';
}
