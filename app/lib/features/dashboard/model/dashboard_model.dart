import '../../../core/ble/ble_service.dart';
import '../../../core/models/lk8ex1_data.dart';

enum DashboardVarioTrend { climb, sink, neutral }

class DashboardModel {
  const DashboardModel({
    required this.connectionStatus,
    required this.reconnectState,
    required this.connectedDeviceName,
    required this.latestData,
    required this.isStale,
    required this.lastUpdatedAt,
    required this.parseWarningCount,
    required this.lastStreamError,
  });

  const DashboardModel.initial({
    required BleConnectionStatus connectionStatus,
    required BleReconnectState reconnectState,
    String? connectedDeviceName,
  }) : this(
         connectionStatus: connectionStatus,
         reconnectState: reconnectState,
         connectedDeviceName: connectedDeviceName,
         latestData: null,
         isStale: false,
         lastUpdatedAt: null,
         parseWarningCount: 0,
         lastStreamError: null,
       );

  static const double defaultVarioDeadZoneMs = 0.1;

  final BleConnectionStatus connectionStatus;
  final BleReconnectState reconnectState;
  final String? connectedDeviceName;
  final Lk8ex1Data? latestData;
  final bool isStale;
  final DateTime? lastUpdatedAt;
  final int parseWarningCount;
  final String? lastStreamError;

  bool get hasData => latestData != null;

  bool get isConnected => connectionStatus == BleConnectionStatus.connected;

  DashboardVarioTrend get varioTrend {
    final Lk8ex1Data? data = latestData;
    if (data == null) {
      return DashboardVarioTrend.neutral;
    }
    return DashboardFormatter.resolveVarioTrend(
      data.varioMs,
      deadZoneMs: defaultVarioDeadZoneMs,
    );
  }

  String get connectionLabel {
    if (reconnectState.isReconnecting) {
      return 'Reconnecting...';
    }

    switch (connectionStatus) {
      case BleConnectionStatus.connected:
        final String normalizedName = connectedDeviceName?.trim() ?? '';
        return normalizedName.isEmpty ? 'Connected' : normalizedName;
      case BleConnectionStatus.connecting:
        return 'Connecting...';
      case BleConnectionStatus.disconnecting:
        return 'Disconnecting...';
      case BleConnectionStatus.disconnected:
        return 'Disconnected';
    }
  }

  DashboardModel copyWith({
    BleConnectionStatus? connectionStatus,
    BleReconnectState? reconnectState,
    Object? connectedDeviceName = _keepCurrentValue,
    Object? latestData = _keepCurrentValue,
    bool? isStale,
    Object? lastUpdatedAt = _keepCurrentValue,
    int? parseWarningCount,
    Object? lastStreamError = _keepCurrentValue,
  }) {
    return DashboardModel(
      connectionStatus: connectionStatus ?? this.connectionStatus,
      reconnectState: reconnectState ?? this.reconnectState,
      connectedDeviceName: identical(connectedDeviceName, _keepCurrentValue)
          ? this.connectedDeviceName
          : connectedDeviceName as String?,
      latestData: identical(latestData, _keepCurrentValue)
          ? this.latestData
          : latestData as Lk8ex1Data?,
      isStale: isStale ?? this.isStale,
      lastUpdatedAt: identical(lastUpdatedAt, _keepCurrentValue)
          ? this.lastUpdatedAt
          : lastUpdatedAt as DateTime?,
      parseWarningCount: parseWarningCount ?? this.parseWarningCount,
      lastStreamError: identical(lastStreamError, _keepCurrentValue)
          ? this.lastStreamError
          : lastStreamError as String?,
    );
  }
}

class DashboardFormatter {
  const DashboardFormatter._();

  static String formatAltitudeMeters(Lk8ex1Data? data) {
    if (data == null || data.altitudeM.isNaN) {
      return '---';
    }
    return data.altitudeM.toStringAsFixed(0);
  }

  static String formatVarioMs(Lk8ex1Data? data) {
    if (data == null) {
      return '--';
    }
    return data.varioMs.toStringAsFixed(2);
  }

  static String formatPressureHpa(Lk8ex1Data? data) {
    if (data == null) {
      return '--';
    }
    return (data.pressurePa / 100.0).toStringAsFixed(2);
  }

  static String formatTemperatureC(Lk8ex1Data? data) {
    if (data == null) {
      return '--';
    }
    return data.temperatureC.toStringAsFixed(1);
  }

  static DashboardVarioTrend resolveVarioTrend(
    double varioMs, {
    double deadZoneMs = DashboardModel.defaultVarioDeadZoneMs,
  }) {
    if (varioMs > deadZoneMs) {
      return DashboardVarioTrend.climb;
    }
    if (varioMs < -deadZoneMs) {
      return DashboardVarioTrend.sink;
    }
    return DashboardVarioTrend.neutral;
  }
}

const Object _keepCurrentValue = Object();
