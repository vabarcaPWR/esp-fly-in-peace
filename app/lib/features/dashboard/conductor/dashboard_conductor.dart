import 'dart:async';

import 'package:flutter/foundation.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../../../core/ble/ble_service.dart';
import '../../../core/utils/lk8ex1_parser.dart';
import '../hardware/dashboard_hardware.dart';
import '../model/dashboard_model.dart';

class DashboardConductor extends StateNotifier<DashboardModel> {
  DashboardConductor({
    required DashboardHardware hardware,
    required Lk8ex1Parser parser,
  }) : _hardware = hardware,
       _parser = parser,
       super(
         DashboardModel.initial(
           connectionStatus: hardware.connectionStatus,
           reconnectState: hardware.reconnectState,
           connectedDeviceName: hardware.connectedDeviceName,
         ),
       ) {
    _telemetrySubscription = _hardware.telemetryLines.listen(
      _handleTelemetryLine,
      onError: _handleTelemetryError,
    );
    _connectionStatusSubscription = _hardware.connectionStatusStream.listen(
      _handleConnectionStatus,
      onError: _handleConnectionStatusError,
    );
    _reconnectStateSubscription = _hardware.reconnectStateStream.listen(
      _handleReconnectState,
      onError: _handleReconnectError,
    );
  }

  final DashboardHardware _hardware;
  final Lk8ex1Parser _parser;

  StreamSubscription<String>? _telemetrySubscription;
  StreamSubscription<BleConnectionStatus>? _connectionStatusSubscription;
  StreamSubscription<BleReconnectState>? _reconnectStateSubscription;

  String? _latestPendingLine;
  bool _isProcessingLine = false;

  void _handleTelemetryLine(String line) {
    _latestPendingLine = line;
    if (_isProcessingLine) {
      return;
    }

    _drainPendingLines();
  }

  void _drainPendingLines() {
    _isProcessingLine = true;
    while (_latestPendingLine != null) {
      final String lineToParse = _latestPendingLine!;
      _latestPendingLine = null;
      _parseAndApply(lineToParse);
    }
    _isProcessingLine = false;
  }

  void _parseAndApply(String line) {
    final Lk8ex1ParseResult parseResult = _parser.parseLine(line);
    if (parseResult.data == null) {
      debugPrint('Dashboard parse warning: ${parseResult.errorReason}');
      state = state.copyWith(
        parseWarningCount: state.parseWarningCount + 1,
        lastStreamError: parseResult.errorReason,
      );
      return;
    }

    state = state.copyWith(
      latestData: parseResult.data,
      isStale: false,
      lastUpdatedAt: DateTime.now().toUtc(),
      lastStreamError: null,
    );
  }

  void _handleTelemetryError(Object error, StackTrace stackTrace) {
    debugPrint('Dashboard telemetry stream error: $error');
    state = state.copyWith(lastStreamError: '$error');
  }

  void _handleConnectionStatus(BleConnectionStatus status) {
    final bool shouldMarkStale =
        status == BleConnectionStatus.disconnected && state.latestData != null;
    state = state.copyWith(
      connectionStatus: status,
      connectedDeviceName: _hardware.connectedDeviceName,
      isStale: shouldMarkStale,
    );
  }

  void _handleConnectionStatusError(Object error, StackTrace stackTrace) {
    debugPrint('Dashboard connection stream error: $error');
    state = state.copyWith(lastStreamError: '$error');
  }

  void _handleReconnectState(BleReconnectState reconnectState) {
    state = state.copyWith(
      reconnectState: reconnectState,
      connectedDeviceName: _hardware.connectedDeviceName,
    );
  }

  void _handleReconnectError(Object error, StackTrace stackTrace) {
    debugPrint('Dashboard reconnect stream error: $error');
    state = state.copyWith(lastStreamError: '$error');
  }

  @override
  void dispose() {
    _telemetrySubscription?.cancel();
    _connectionStatusSubscription?.cancel();
    _reconnectStateSubscription?.cancel();
    super.dispose();
  }
}
