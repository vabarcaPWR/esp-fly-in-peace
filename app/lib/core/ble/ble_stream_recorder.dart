import 'dart:async';
import 'dart:io';

import 'package:path_provider/path_provider.dart';
import 'package:package_info_plus/package_info_plus.dart';

import 'ble_service.dart';

class BleRecordingPaths {
  const BleRecordingPaths({required this.logPath, required this.csvPath});

  final String logPath;
  final String csvPath;
}

class BleStreamRecorder {
  BleStreamRecorder({required BleService bleService})
    : _bleService = bleService;

  final BleService _bleService;

  StreamSubscription<String>? _receivedLinesSubscription;
  StreamSubscription<BleConnectionStatus>? _connectionStatusSubscription;
  IOSink? _logSink;
  IOSink? _csvSink;
  BleRecordingPaths? _currentPaths;
  BleRecordingPaths? _lastSavedPaths;

  bool get isRecording => _logSink != null && _csvSink != null;
  String? get currentFilePath => _currentPaths?.logPath;
  String? get currentCsvFilePath => _currentPaths?.csvPath;
  String? get lastSavedFilePath => _lastSavedPaths?.logPath;
  String? get lastSavedCsvFilePath => _lastSavedPaths?.csvPath;

  Future<String> startRecording({
    String? outputDirectoryPath,
    String? filePrefix,
  }) async {
    if (isRecording) {
      return _currentPaths!.logPath;
    }

    if (_bleService.status != BleConnectionStatus.connected) {
      throw const BleServiceException(
        'Cannot start recording: BLE device is not connected.',
      );
    }

    final Directory logsDirectory = await _resolveLogsDirectory(
      outputDirectoryPath,
    );
    final String resolvedPrefix = _sanitizeFilePrefix(filePrefix);
    final String timestamp = DateTime.now()
        .toUtc()
        .toIso8601String()
        .replaceAll(':', '-');
    final File logFile = File(
      '${logsDirectory.path}/${resolvedPrefix}_$timestamp.log',
    );
    final File csvFile = File(
      '${logsDirectory.path}/${resolvedPrefix}_$timestamp.csv',
    );

    _logSink = logFile.openWrite(mode: FileMode.writeOnlyAppend);
    _csvSink = csvFile.openWrite(mode: FileMode.writeOnlyAppend);
    _currentPaths = BleRecordingPaths(
      logPath: logFile.path,
      csvPath: csvFile.path,
    );
    _lastSavedPaths = null;

    final String startedAtUtc = DateTime.now().toUtc().toIso8601String();
    final String deviceId =
        _bleService.connectedDevice?.remoteId.str ?? 'unknown';
    final String deviceName =
        _bleService.connectedDevice?.platformName.trim() ?? 'unknown';
    final String compatibilityProfile = _resolveCompatibilityProfile(
      deviceName,
    );
    final PackageInfo packageInfo = await PackageInfo.fromPlatform();
    final String appBuild =
        '${packageInfo.appName} ${packageInfo.version}+${packageInfo.buildNumber}';

    _logSink!.writeln('# BLE Session Recording');
    _logSink!.writeln('# started_at_utc=$startedAtUtc');
    _logSink!.writeln('# device_id=$deviceId');
    _logSink!.writeln('# device_name=$deviceName');
    _logSink!.writeln('# profile=$compatibilityProfile');
    _logSink!.writeln('# app_build=$appBuild');
    _logSink!.writeln('# format=utc_iso8601|raw_line');

    _csvSink!.writeln('# BLE Session Recording CSV');
    _csvSink!.writeln('# started_at_utc,$startedAtUtc');
    _csvSink!.writeln('# device_id,$deviceId');
    _csvSink!.writeln('# device_name,"${_escapeCsv(deviceName)}"');
    _csvSink!.writeln('# profile,$compatibilityProfile');
    _csvSink!.writeln('# app_build,"${_escapeCsv(appBuild)}"');
    _csvSink!.writeln('timestamp_utc,raw_line');

    _receivedLinesSubscription = _bleService.receivedLines.listen((line) {
      final String nowUtc = DateTime.now().toUtc().toIso8601String();
      _logSink?.writeln('$nowUtc|$line');
      _csvSink?.writeln('$nowUtc,"${_escapeCsv(line)}"');
    });

    _connectionStatusSubscription = _bleService.statusStream.listen((status) {
      if (status == BleConnectionStatus.disconnected ||
          status == BleConnectionStatus.disconnecting) {
        unawaited(stopRecording());
      }
    });

    return logFile.path;
  }

  Future<String?> stopRecording() async {
    final BleRecordingPaths? currentPaths = _currentPaths;

    await _receivedLinesSubscription?.cancel();
    _receivedLinesSubscription = null;
    await _connectionStatusSubscription?.cancel();
    _connectionStatusSubscription = null;

    if (_logSink != null) {
      final String stoppedAtUtc = DateTime.now().toUtc().toIso8601String();
      _logSink!.writeln('# stopped_at_utc=$stoppedAtUtc');
      await _logSink!.flush();
      await _logSink!.close();
    }

    if (_csvSink != null) {
      final String stoppedAtUtc = DateTime.now().toUtc().toIso8601String();
      _csvSink!.writeln('# stopped_at_utc,$stoppedAtUtc');
      await _csvSink!.flush();
      await _csvSink!.close();
    }

    _logSink = null;
    _csvSink = null;
    _currentPaths = null;
    _lastSavedPaths = currentPaths;
    return currentPaths?.logPath;
  }

  Future<void> dispose() async {
    await stopRecording();
  }

  Future<Directory> _resolveLogsDirectory(String? outputDirectoryPath) async {
    final String normalizedOutputDirectory = outputDirectoryPath?.trim() ?? '';
    if (normalizedOutputDirectory.isNotEmpty) {
      final Directory selectedDirectory = Directory(normalizedOutputDirectory);
      await selectedDirectory.create(recursive: true);
      return selectedDirectory;
    }

    final Directory appDocumentsDirectory =
        await getApplicationDocumentsDirectory();
    final Directory logsDirectory = Directory(
      '${appDocumentsDirectory.path}/ble_logs',
    );
    await logsDirectory.create(recursive: true);
    return logsDirectory;
  }

  String _sanitizeFilePrefix(String? prefix) {
    final String normalized = prefix?.trim() ?? '';
    if (normalized.isEmpty) {
      return 'ble_session';
    }

    final String sanitized = normalized.replaceAll(
      RegExp(r'[^a-zA-Z0-9_-]'),
      '_',
    );
    return sanitized.isEmpty ? 'ble_session' : sanitized;
  }

  String _resolveCompatibilityProfile(String deviceName) {
    final BleCompatibilityProfile profile =
        BleService.detectCompatibilityProfile(deviceName);
    switch (profile) {
      case BleCompatibilityProfile.flyInPeace:
        return 'flyInPeace';
      case BleCompatibilityProfile.blueFlyVario:
        return 'blueFlyVario';
      case BleCompatibilityProfile.unsupported:
        return 'unsupported';
    }
  }

  String _escapeCsv(String value) {
    return value.replaceAll('"', '""');
  }
}
