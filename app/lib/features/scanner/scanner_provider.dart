import 'dart:async';

import 'package:flutter_riverpod/flutter_riverpod.dart';

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

class ScannerState {
  const ScannerState({
    required this.devices,
    required this.deviceFilter,
    required this.isScanning,
    required this.isConnecting,
    required this.connectingDeviceId,
    required this.scanProgress,
    required this.showScanAgain,
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
  final ScannerDialogRequest? dialogRequest;

  List<BleScanDevice> get visibleDevices {
    if (deviceFilter == ScannerDeviceFilter.all) {
      return devices;
    }

    return devices.where((d) => d.isFlyInPeaceCompatible).toList();
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
      dialogRequest: clearDialog ? null : (dialogRequest ?? this.dialogRequest),
    );
  }
}

final blePermissionsProvider = Provider<BlePermissions>((ref) {
  return const BlePermissions();
});

final bleServiceProvider = Provider<BleService>((ref) {
  final BleService service = BleService();
  ref.onDispose(service.dispose);
  return service;
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
    });
    _isScanningSubscription = _bleService.isScanning.listen(_handleScanning);
  }

  static const Duration defaultScanTimeout = Duration(seconds: 10);

  final BleService _bleService;
  final BlePermissions _blePermissions;

  StreamSubscription<List<BleScanDevice>>? _scanResultsSubscription;
  StreamSubscription<bool>? _isScanningSubscription;
  Timer? _progressTimer;
  DateTime? _scanStartedAt;
  Duration _activeScanTimeout = defaultScanTimeout;

  Future<void> startScan({Duration timeout = defaultScanTimeout}) async {
    _progressTimer?.cancel();
    _scanStartedAt = null;
    _activeScanTimeout = timeout;

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
    super.dispose();
  }
}
