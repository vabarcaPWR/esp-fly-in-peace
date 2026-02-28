import 'dart:async';

import 'package:flutter/foundation.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../../core/ble/ble_providers.dart';
import '../../core/ble/ble_permissions.dart';
import '../../core/ble/ble_service.dart';

enum ScannerDialogAction { none, openAppSettings, turnOnBluetooth }

enum ScannerDeviceFilter { all, compatible }

class ScannerDialogRequest {
  const ScannerDialogRequest({
    required this.title,
    required this.message,
    required this.action,
  });

  final String title;
  final String message;
  final ScannerDialogAction action;
}

class ScannerAutoConnectCandidate {
  const ScannerAutoConnectCandidate({
    required this.remoteId,
    required this.hasFlyInPeaceName,
    required this.profile,
  });

  final String remoteId;
  final bool hasFlyInPeaceName;
  final BleCompatibilityProfile profile;
}

class ScannerState {
  const ScannerState({
    required this.devices,
    required this.deviceFilter,
    required this.isScanning,
    required this.isConnecting,
    required this.connectingDeviceId,
    required this.scanProgress,
    required this.showScanAgain,
    required this.connectionStatus,
    required this.connectedDeviceId,
    required this.dialogRequest,
  });

  factory ScannerState.initial() {
    return const ScannerState(
      devices: <BleScanDevice>[],
      deviceFilter: ScannerDeviceFilter.all,
      isScanning: false,
      isConnecting: false,
      connectingDeviceId: null,
      scanProgress: 0,
      showScanAgain: false,
      connectionStatus: BleConnectionStatus.disconnected,
      connectedDeviceId: null,
      dialogRequest: null,
    );
  }

  final List<BleScanDevice> devices;
  final ScannerDeviceFilter deviceFilter;
  final bool isScanning;
  final bool isConnecting;
  final String? connectingDeviceId;
  final double scanProgress;
  final bool showScanAgain;
  final BleConnectionStatus connectionStatus;
  final String? connectedDeviceId;
  final ScannerDialogRequest? dialogRequest;

  bool get isConnected => connectionStatus == BleConnectionStatus.connected;

  List<BleScanDevice> get visibleDevices {
    final List<BleScanDevice> sourceDevices =
        deviceFilter == ScannerDeviceFilter.all
        ? devices
        : devices.where((d) => d.isFlyInPeaceCompatible).toList();
    return _deduplicateCompatibleDevices(sourceDevices);
  }

  List<BleScanDevice> _deduplicateCompatibleDevices(
    List<BleScanDevice> source,
  ) {
    final Map<String, BleScanDevice> dedupedByKey = <String, BleScanDevice>{};
    for (final BleScanDevice device in source) {
      if (!device.isFlyInPeaceCompatible) {
        dedupedByKey['raw:${device.remoteId}'] = device;
        continue;
      }

      final String normalizedName = device.name.trim().toLowerCase();
      final String mergeNameKey =
          normalizedName.isEmpty || normalizedName == 'unknown'
          ? device.remoteId
          : normalizedName;
      final String protocolKey =
          '${device.compatibilityProfile.name}|nus:${device.hasNusService}|bluefly:${device.hasBlueFlyService}';
      final String mergedKey = 'compat:$mergeNameKey:$protocolKey';
      final BleScanDevice? existing = dedupedByKey[mergedKey];
      if (existing == null || device.rssi > existing.rssi) {
        dedupedByKey[mergedKey] = device;
      }
    }

    final List<BleScanDevice> deduped = dedupedByKey.values.toList()
      ..sort((a, b) => b.rssi.compareTo(a.rssi));
    return deduped;
  }

  List<BleScanDevice> get compatibleDevices {
    if (deviceFilter == ScannerDeviceFilter.all) {
      return visibleDevices.where((d) => d.isFlyInPeaceCompatible).toList();
    }
    return visibleDevices;
  }

  ScannerState copyWith({
    List<BleScanDevice>? devices,
    ScannerDeviceFilter? deviceFilter,
    bool? isScanning,
    bool? isConnecting,
    String? connectingDeviceId,
    bool clearConnectingDeviceId = false,
    double? scanProgress,
    bool? showScanAgain,
    BleConnectionStatus? connectionStatus,
    String? connectedDeviceId,
    bool clearConnectedDeviceId = false,
    ScannerDialogRequest? dialogRequest,
    bool clearDialog = false,
  }) {
    return ScannerState(
      devices: devices ?? this.devices,
      deviceFilter: deviceFilter ?? this.deviceFilter,
      isScanning: isScanning ?? this.isScanning,
      isConnecting: isConnecting ?? this.isConnecting,
      connectingDeviceId: clearConnectingDeviceId
          ? null
          : (connectingDeviceId ?? this.connectingDeviceId),
      scanProgress: scanProgress ?? this.scanProgress,
      showScanAgain: showScanAgain ?? this.showScanAgain,
      connectionStatus: connectionStatus ?? this.connectionStatus,
      connectedDeviceId: clearConnectedDeviceId
          ? null
          : (connectedDeviceId ?? this.connectedDeviceId),
      dialogRequest: clearDialog ? null : (dialogRequest ?? this.dialogRequest),
    );
  }
}

final blePermissionsProvider = Provider<BlePermissions>((ref) {
  return const BlePermissions();
});

final scannerControllerProvider =
    StateNotifierProvider.autoDispose<ScannerController, ScannerState>((ref) {
      return ScannerController(
        bleService: ref.watch(bleServiceProvider),
        blePermissions: ref.watch(blePermissionsProvider),
      );
    });

class ScannerController extends StateNotifier<ScannerState> {
  ScannerController({
    required BleService bleService,
    required BlePermissions blePermissions,
  }) : _bleService = bleService,
       _blePermissions = blePermissions,
       super(ScannerState.initial()) {
    _scanResultsSubscription = _bleService.scanResults.listen((devices) {
      state = state.copyWith(devices: devices);
      _maybeAutoConnectToFlyInPeace(devices);
    });
    _isScanningSubscription = _bleService.isScanning.listen(_handleScanning);
    _connectionStatusSubscription = _bleService.statusStream.listen(
      _handleConnectionStatus,
    );

    _handleConnectionStatus(_bleService.status);
  }

  static const Duration defaultScanTimeout = Duration(seconds: 10);

  final BleService _bleService;
  final BlePermissions _blePermissions;

  StreamSubscription<List<BleScanDevice>>? _scanResultsSubscription;
  StreamSubscription<bool>? _isScanningSubscription;
  StreamSubscription<BleConnectionStatus>? _connectionStatusSubscription;
  Timer? _progressTimer;
  DateTime? _scanStartedAt;
  Duration _activeScanTimeout = defaultScanTimeout;
  final Set<String> _autoConnectAttemptedDeviceIds = <String>{};

  Future<void> startScan({Duration timeout = defaultScanTimeout}) async {
    _progressTimer?.cancel();
    _scanStartedAt = null;
    _activeScanTimeout = timeout;
    _autoConnectAttemptedDeviceIds.clear();

    final BleReadiness readiness = await _blePermissions.ensureReadyForScan();
    if (!readiness.isReady) {
      final ScannerDialogRequest dialogRequest = _dialogForReadiness(readiness);
      state = state.copyWith(showScanAgain: true, dialogRequest: dialogRequest);
      return;
    }

    state = state.copyWith(
      devices: const <BleScanDevice>[],
      scanProgress: 0,
      showScanAgain: false,
      clearDialog: true,
    );

    _scanStartedAt = DateTime.now();
    _progressTimer = Timer.periodic(const Duration(milliseconds: 250), (_) {
      if (_scanStartedAt == null || !state.isScanning) {
        return;
      }

      final int elapsedMs = DateTime.now()
          .difference(_scanStartedAt!)
          .inMilliseconds;
      final int timeoutMs = _activeScanTimeout.inMilliseconds;
      final double progress = timeoutMs <= 0
          ? 0
          : (elapsedMs / timeoutMs).clamp(0.0, 1.0);
      state = state.copyWith(scanProgress: progress);
    });

    try {
      await _bleService.startScan(timeout: timeout);
    } catch (error) {
      _progressTimer?.cancel();
      state = state.copyWith(
        showScanAgain: true,
        dialogRequest: ScannerDialogRequest(
          title: 'Scan error',
          message: 'Failed to start BLE scan: $error',
          action: ScannerDialogAction.none,
        ),
      );
    }
  }

  Future<void> stopScan() async {
    await _bleService.stopScan();
  }

  Future<void> refreshScan() async {
    await startScan(timeout: defaultScanTimeout);
  }

  Future<bool> connectToDevice(BleScanDevice scanDevice) async {
    state = state.copyWith(
      isConnecting: true,
      connectingDeviceId: scanDevice.remoteId,
      clearDialog: true,
    );

    try {
      await _bleService.connect(scanDevice.device);
      state = state.copyWith(
        isConnecting: false,
        clearConnectingDeviceId: true,
        connectedDeviceId: scanDevice.remoteId,
      );
      return true;
    } catch (error) {
      state = state.copyWith(
        isConnecting: false,
        clearConnectingDeviceId: true,
        dialogRequest: ScannerDialogRequest(
          title: 'Connection error',
          message: '$error',
          action: ScannerDialogAction.none,
        ),
      );
      return false;
    }
  }

  static String? pickAutoConnectCandidateId({
    required List<ScannerAutoConnectCandidate> candidates,
    required Set<String> attemptedDeviceIds,
  }) {
    for (final ScannerAutoConnectCandidate candidate in candidates) {
      final bool isFlyInPeaceCandidate =
          candidate.hasFlyInPeaceName ||
          candidate.profile == BleCompatibilityProfile.flyInPeace;
      if (!isFlyInPeaceCandidate) {
        continue;
      }

      if (attemptedDeviceIds.contains(candidate.remoteId)) {
        continue;
      }

      return candidate.remoteId;
    }

    return null;
  }

  void setDeviceFilter(ScannerDeviceFilter filter) {
    state = state.copyWith(deviceFilter: filter);
  }

  Future<void> handleDialogAction() async {
    final ScannerDialogRequest? dialogRequest = state.dialogRequest;
    if (dialogRequest == null) {
      return;
    }

    switch (dialogRequest.action) {
      case ScannerDialogAction.none:
        break;
      case ScannerDialogAction.openAppSettings:
        await _blePermissions.openAppPermissionsSettings();
        break;
      case ScannerDialogAction.turnOnBluetooth:
        await _blePermissions.turnOnBluetooth();
        break;
    }
  }

  void clearDialog() {
    state = state.copyWith(clearDialog: true);
  }

  void _handleScanning(bool isScanning) {
    if (!isScanning) {
      _progressTimer?.cancel();
      state = state.copyWith(
        isScanning: false,
        scanProgress: 1,
        showScanAgain: true,
      );
      return;
    }

    state = state.copyWith(isScanning: true, showScanAgain: false);
  }

  void _maybeAutoConnectToFlyInPeace(List<BleScanDevice> devices) {
    if (state.isConnecting || state.isConnected) {
      return;
    }

    final List<ScannerAutoConnectCandidate> candidates = devices
        .map(
          (BleScanDevice device) => ScannerAutoConnectCandidate(
            remoteId: device.remoteId,
            hasFlyInPeaceName: device.hasFlyInPeaceName,
            profile: device.compatibilityProfile,
          ),
        )
        .toList();
    final String? candidateId = pickAutoConnectCandidateId(
      candidates: candidates,
      attemptedDeviceIds: _autoConnectAttemptedDeviceIds,
    );
    if (candidateId == null) {
      return;
    }
    debugPrint('BLE auto-connect candidate: $candidateId');

    BleScanDevice? selectedDevice;
    for (final BleScanDevice device in devices) {
      if (device.remoteId == candidateId) {
        selectedDevice = device;
        break;
      }
    }
    if (selectedDevice == null) {
      return;
    }

    _autoConnectAttemptedDeviceIds.add(candidateId);
    debugPrint('BLE auto-connect attempt: $candidateId');
    unawaited(connectToDevice(selectedDevice));
  }

  void _handleConnectionStatus(BleConnectionStatus connectionStatus) {
    final String? connectedDeviceId =
        connectionStatus == BleConnectionStatus.connected
        ? _bleService.connectedDevice?.remoteId.str
        : null;

    state = state.copyWith(
      connectionStatus: connectionStatus,
      connectedDeviceId: connectedDeviceId,
      clearConnectedDeviceId: connectionStatus != BleConnectionStatus.connected,
    );
  }

  ScannerDialogRequest _dialogForReadiness(BleReadiness readiness) {
    switch (readiness.issue) {
      case BleReadinessIssue.permissionsPermanentlyDenied:
        return ScannerDialogRequest(
          title: 'Permissions required',
          message: readiness.message,
          action: ScannerDialogAction.openAppSettings,
        );
      case BleReadinessIssue.bluetoothDisabled:
        return ScannerDialogRequest(
          title: 'Bluetooth disabled',
          message: readiness.message,
          action: ScannerDialogAction.turnOnBluetooth,
        );
      case BleReadinessIssue.locationServiceDisabled:
        return ScannerDialogRequest(
          title: 'Location services disabled',
          message: readiness.message,
          action: ScannerDialogAction.openAppSettings,
        );
      case BleReadinessIssue.permissionsDenied:
      case BleReadinessIssue.notSupported:
        return ScannerDialogRequest(
          title: 'Scan unavailable',
          message: readiness.message,
          action: ScannerDialogAction.none,
        );
      case BleReadinessIssue.none:
        return const ScannerDialogRequest(
          title: 'Ready',
          message: 'Ready',
          action: ScannerDialogAction.none,
        );
    }
  }

  @override
  void dispose() {
    _progressTimer?.cancel();
    _scanResultsSubscription?.cancel();
    _isScanningSubscription?.cancel();
    _connectionStatusSubscription?.cancel();
    super.dispose();
  }

  Future<void> disconnectDevice() async {
    await _bleService.disconnect();
  }
}
