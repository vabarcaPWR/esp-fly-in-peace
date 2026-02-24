import 'package:flutter/foundation.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:permission_handler/permission_handler.dart';

enum BleReadinessIssue {
  none,
  notSupported,
  permissionsDenied,
  permissionsPermanentlyDenied,
  bluetoothDisabled,
  locationServiceDisabled,
}

class BleReadiness {
  const BleReadiness({required this.issue, required this.message});

  final BleReadinessIssue issue;
  final String message;

  bool get isReady => issue == BleReadinessIssue.none;
}

class BlePermissions {
  const BlePermissions();

  Future<BleReadiness> ensureReadyForScan() async {
    if (kIsWeb) {
      return const BleReadiness(
        issue: BleReadinessIssue.none,
        message: 'Ready',
      );
    }

    final bool supported;
    try {
      supported = await FlutterBluePlus.isSupported;
    } on UnsupportedError {
      return const BleReadiness(
        issue: BleReadinessIssue.notSupported,
        message: 'Bluetooth scan is not supported on this platform.',
      );
    }

    if (!supported) {
      return const BleReadiness(
        issue: BleReadinessIssue.notSupported,
        message: 'Bluetooth LE is not supported on this device.',
      );
    }

    final Map<Permission, PermissionStatus> statuses = await <Permission>[
      Permission.bluetoothScan,
      Permission.bluetoothConnect,
      Permission.locationWhenInUse,
    ].request();

    final bool permanentlyDenied = statuses.values.any(
      (status) => status.isPermanentlyDenied,
    );
    if (permanentlyDenied) {
      return const BleReadiness(
        issue: BleReadinessIssue.permissionsPermanentlyDenied,
        message:
            'Bluetooth permissions are permanently denied. Open app settings to continue.',
      );
    }

    final bool denied = statuses.values.any(
      (status) => status.isDenied || status.isRestricted,
    );
    if (denied) {
      return const BleReadiness(
        issue: BleReadinessIssue.permissionsDenied,
        message:
            'Bluetooth permissions are required to scan for your vario device.',
      );
    }

    final BluetoothAdapterState adapterState =
        await FlutterBluePlus.adapterState.first;
    if (adapterState != BluetoothAdapterState.on) {
      return const BleReadiness(
        issue: BleReadinessIssue.bluetoothDisabled,
        message: 'Bluetooth is turned off. Please enable Bluetooth and retry.',
      );
    }

    final ServiceStatus locationServiceStatus =
        await Permission.locationWhenInUse.serviceStatus;
    if (locationServiceStatus != ServiceStatus.enabled) {
      return const BleReadiness(
        issue: BleReadinessIssue.locationServiceDisabled,
        message:
            'Location services are disabled. Enable location services for BLE scan.',
      );
    }

    return const BleReadiness(issue: BleReadinessIssue.none, message: 'Ready');
  }

  Future<bool> turnOnBluetooth() async {
    try {
      await FlutterBluePlus.turnOn(timeout: 20);
      final BluetoothAdapterState adapterState =
          await FlutterBluePlus.adapterState.first;
      return adapterState == BluetoothAdapterState.on;
    } catch (_) {
      return false;
    }
  }

  Future<bool> openAppPermissionsSettings() async {
    return openAppSettings();
  }
}
