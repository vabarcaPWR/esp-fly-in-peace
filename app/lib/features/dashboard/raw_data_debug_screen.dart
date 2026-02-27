import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:flutter/scheduler.dart';
import 'package:flutter/services.dart';
import 'package:file_selector/file_selector.dart';
import 'package:url_launcher/url_launcher.dart';

import '../../core/ble/ble_providers.dart';
import '../../core/ble/ble_service.dart';
import '../../core/ble/ble_stream_recorder.dart';

class RawDataEntry {
  const RawDataEntry({required this.timestamp, required this.line});

  final DateTime timestamp;
  final String line;
}

class RawDataDebugScreen extends ConsumerStatefulWidget {
  const RawDataDebugScreen({super.key});

  @override
  ConsumerState<RawDataDebugScreen> createState() => _RawDataDebugScreenState();
}

class _RawDataDebugScreenState extends ConsumerState<RawDataDebugScreen> {
  final TextEditingController _sendController = TextEditingController();
  final TextEditingController _filePrefixController = TextEditingController(
    text: 'ble_session',
  );
  final ScrollController _scrollController = ScrollController();
  final List<RawDataEntry> _entries = <RawDataEntry>[];
  StreamSubscription<String>? _receivedLinesSubscription;
  String? _sendError;
  String? _recordingError;
  String? _activeRecordingPath;
  String? _activeCsvRecordingPath;
  String? _lastSavedRecordingPath;
  String? _lastSavedCsvRecordingPath;
  String? _selectedOutputDirectory;
  int? _autoscrollFrameCallbackId;

  @override
  void initState() {
    super.initState();

    _receivedLinesSubscription = ref
        .read(bleServiceProvider)
        .receivedLines
        .listen((line) {
          setState(() {
            _entries.add(RawDataEntry(timestamp: DateTime.now(), line: line));
          });

          _scheduleAutoscrollIfNeeded();
        });
  }

  @override
  void dispose() {
    _receivedLinesSubscription?.cancel();
    _cancelScheduledAutoscroll();
    _sendController.dispose();
    _filePrefixController.dispose();
    _scrollController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final BleConnectionStatus connectionStatus =
        ref.watch(bleConnectionStatusProvider).valueOrNull ??
        ref.read(bleServiceProvider).status;
    final bool isConnected = connectionStatus == BleConnectionStatus.connected;
    final BleStreamRecorder recorder = ref.read(bleStreamRecorderProvider);
    final bool isRecording = recorder.isRecording;

    if (isRecording && _activeRecordingPath == null) {
      _activeRecordingPath = recorder.currentFilePath;
      _activeCsvRecordingPath = recorder.currentCsvFilePath;
    }

    if (!isRecording && _lastSavedRecordingPath == null) {
      _lastSavedRecordingPath = recorder.lastSavedFilePath;
      _lastSavedCsvRecordingPath = recorder.lastSavedCsvFilePath;
    }

    return Scaffold(
      appBar: AppBar(
        title: const Text('Raw BLE Debug'),
        actions: [
          Row(
            mainAxisSize: MainAxisSize.min,
            children: [
              const Text('Auto'),
              Switch(
                value: ref.watch(rawBleDebugAutoscrollEnabledProvider),
                onChanged: (bool enabled) {
                  if (!enabled) {
                    _cancelScheduledAutoscroll();
                  }
                  ref
                          .read(rawBleDebugAutoscrollEnabledProvider.notifier)
                          .state =
                      enabled;
                },
              ),
            ],
          ),
          IconButton(
            onPressed: _entries.isEmpty
                ? null
                : () {
                    setState(() {
                      _entries.clear();
                    });
                  },
            icon: const Icon(Icons.clear_all),
            tooltip: 'Clear',
          ),
        ],
      ),
      body: Column(
        children: [
          Padding(
            padding: const EdgeInsets.fromLTRB(16, 12, 16, 8),
            child: Row(
              children: [
                Icon(
                  isConnected
                      ? Icons.bluetooth_connected
                      : Icons.bluetooth_disabled,
                  color: isConnected
                      ? Theme.of(context).colorScheme.primary
                      : Theme.of(context).colorScheme.error,
                ),
                const SizedBox(width: 8),
                Text(isConnected ? 'Connected' : 'Disconnected'),
              ],
            ),
          ),
          Padding(
            padding: const EdgeInsets.fromLTRB(16, 0, 16, 8),
            child: TextField(
              controller: _filePrefixController,
              enabled: !isRecording,
              decoration: const InputDecoration(
                labelText: 'File name prefix',
                hintText: 'ble_session',
              ),
            ),
          ),
          Padding(
            padding: const EdgeInsets.fromLTRB(16, 0, 16, 8),
            child: Row(
              children: [
                Expanded(
                  child: Text(
                    _selectedOutputDirectory == null
                        ? 'Output folder: default app documents/ble_logs'
                        : 'Output folder: $_selectedOutputDirectory',
                  ),
                ),
                TextButton.icon(
                  onPressed: isRecording
                      ? null
                      : () async {
                          await _pickOutputDirectory();
                        },
                  icon: const Icon(Icons.folder_open),
                  label: const Text('Select folder'),
                ),
              ],
            ),
          ),
          Padding(
            padding: const EdgeInsets.fromLTRB(16, 0, 16, 8),
            child: Row(
              children: [
                FilledButton.icon(
                  onPressed: (isConnected && !isRecording)
                      ? () async {
                          await _startRecording();
                        }
                      : null,
                  icon: const Icon(Icons.fiber_manual_record),
                  label: const Text('Start recording'),
                ),
                const SizedBox(width: 8),
                FilledButton.tonalIcon(
                  onPressed: isRecording
                      ? () async {
                          await _stopRecording();
                        }
                      : null,
                  icon: const Icon(Icons.stop),
                  label: const Text('Stop'),
                ),
              ],
            ),
          ),
          if (isRecording)
            Padding(
              padding: const EdgeInsets.fromLTRB(16, 0, 16, 8),
              child: Align(
                alignment: Alignment.centerLeft,
                child: Text(
                  _activeRecordingPath == null
                      ? 'Recording started'
                      : 'Recording LOG: $_activeRecordingPath\nRecording CSV: ${_activeCsvRecordingPath ?? '-'}',
                ),
              ),
            ),
          if (!isRecording && _lastSavedRecordingPath != null)
            Padding(
              padding: const EdgeInsets.fromLTRB(16, 0, 16, 8),
              child: Row(
                children: [
                  Expanded(
                    child: Text(
                      'Saved LOG: $_lastSavedRecordingPath\nSaved CSV: ${_lastSavedCsvRecordingPath ?? '-'}',
                    ),
                  ),
                  IconButton(
                    onPressed: _lastSavedRecordingPath == null
                        ? null
                        : () async {
                            await _copySavedPathToClipboard();
                          },
                    icon: const Icon(Icons.copy),
                    tooltip: 'Copy path',
                  ),
                  IconButton(
                    onPressed: _lastSavedRecordingPath == null
                        ? null
                        : () async {
                            await _openSavedFile(_lastSavedRecordingPath);
                          },
                    icon: const Icon(Icons.description),
                    tooltip: 'Open LOG',
                  ),
                  IconButton(
                    onPressed: _lastSavedCsvRecordingPath == null
                        ? null
                        : () async {
                            await _openSavedFile(_lastSavedCsvRecordingPath);
                          },
                    icon: const Icon(Icons.table_chart),
                    tooltip: 'Open CSV',
                  ),
                  IconButton(
                    onPressed: _lastSavedRecordingPath == null
                        ? null
                        : () async {
                            await _openSavedFolder();
                          },
                    icon: const Icon(Icons.folder_open),
                    tooltip: 'Open folder',
                  ),
                ],
              ),
            ),
          if (_recordingError != null)
            Padding(
              padding: const EdgeInsets.fromLTRB(16, 0, 16, 8),
              child: Align(
                alignment: Alignment.centerLeft,
                child: Text(
                  _recordingError!,
                  style: TextStyle(color: Theme.of(context).colorScheme.error),
                ),
              ),
            ),
          Expanded(
            child: _entries.isEmpty
                ? const Center(child: Text('No raw data received yet.'))
                : ListView.separated(
                    controller: _scrollController,
                    padding: const EdgeInsets.symmetric(
                      horizontal: 12,
                      vertical: 8,
                    ),
                    itemBuilder: (context, index) {
                      final entry = _entries[index];
                      final String timestamp =
                          '${entry.timestamp.hour.toString().padLeft(2, '0')}:'
                          '${entry.timestamp.minute.toString().padLeft(2, '0')}:'
                          '${entry.timestamp.second.toString().padLeft(2, '0')}.'
                          '${entry.timestamp.millisecond.toString().padLeft(3, '0')}';

                      return ListTile(
                        dense: true,
                        title: Text(
                          entry.line,
                          style: Theme.of(context).textTheme.bodyMedium,
                        ),
                        subtitle: Text(timestamp),
                      );
                    },
                    separatorBuilder: (_, _) => const Divider(height: 1),
                    itemCount: _entries.length,
                  ),
          ),
          Padding(
            padding: const EdgeInsets.fromLTRB(12, 8, 12, 12),
            child: Row(
              children: [
                Expanded(
                  child: TextField(
                    controller: _sendController,
                    enabled: isConnected,
                    decoration: const InputDecoration(
                      labelText: 'Send command',
                      hintText: 'Type raw text to send over NUS RX',
                    ),
                    onSubmitted: (_) => _handleSend(),
                  ),
                ),
                const SizedBox(width: 8),
                FilledButton(
                  onPressed: isConnected ? _handleSend : null,
                  child: const Text('Send'),
                ),
              ],
            ),
          ),
          if (_sendError != null)
            Padding(
              padding: const EdgeInsets.fromLTRB(12, 0, 12, 12),
              child: Align(
                alignment: Alignment.centerLeft,
                child: Text(
                  _sendError!,
                  style: TextStyle(color: Theme.of(context).colorScheme.error),
                ),
              ),
            ),
        ],
      ),
    );
  }

  Future<void> _handleSend() async {
    final String text = _sendController.text.trim();
    if (text.isEmpty) {
      return;
    }

    setState(() {
      _sendError = null;
    });

    try {
      await ref.read(bleServiceProvider).sendCommand(text);
      _sendController.clear();
    } catch (error) {
      setState(() {
        _sendError = '$error';
      });
    }
  }

  Future<void> _startRecording() async {
    setState(() {
      _recordingError = null;
    });

    try {
      final String path = await ref
          .read(bleStreamRecorderProvider)
          .startRecording(
            outputDirectoryPath: _selectedOutputDirectory,
            filePrefix: _filePrefixController.text,
          );
      if (!mounted) {
        return;
      }

      setState(() {
        _activeRecordingPath = path;
        _activeCsvRecordingPath = ref
            .read(bleStreamRecorderProvider)
            .currentCsvFilePath;
        _lastSavedRecordingPath = null;
        _lastSavedCsvRecordingPath = null;
      });
    } catch (error) {
      if (!mounted) {
        return;
      }

      setState(() {
        _recordingError = '$error';
      });
    }
  }

  Future<void> _stopRecording() async {
    setState(() {
      _recordingError = null;
    });

    try {
      final String? savedPath = await ref
          .read(bleStreamRecorderProvider)
          .stopRecording();
      if (!mounted) {
        return;
      }

      setState(() {
        _activeRecordingPath = null;
        _activeCsvRecordingPath = null;
        _lastSavedRecordingPath = savedPath;
        _lastSavedCsvRecordingPath = ref
            .read(bleStreamRecorderProvider)
            .lastSavedCsvFilePath;
      });
    } catch (error) {
      if (!mounted) {
        return;
      }

      setState(() {
        _recordingError = '$error';
      });
    }
  }

  Future<void> _pickOutputDirectory() async {
    setState(() {
      _recordingError = null;
    });

    try {
      final String? selected = await getDirectoryPath();
      if (!mounted || selected == null || selected.trim().isEmpty) {
        return;
      }

      setState(() {
        _selectedOutputDirectory = selected;
      });
    } catch (error) {
      if (!mounted) {
        return;
      }

      setState(() {
        _recordingError = '$error';
      });
    }
  }

  Future<void> _copySavedPathToClipboard() async {
    final String? savedPath = _lastSavedRecordingPath;
    final String? savedCsvPath = _lastSavedCsvRecordingPath;
    if (savedPath == null || savedPath.isEmpty) {
      return;
    }

    final String textToCopy = 'log=$savedPath\ncsv=${savedCsvPath ?? ''}'
        .trimRight();
    await Clipboard.setData(ClipboardData(text: textToCopy));
    if (!mounted) {
      return;
    }

    ScaffoldMessenger.of(
      context,
    ).showSnackBar(const SnackBar(content: Text('Saved path copied')));
  }

  Future<void> _openSavedFolder() async {
    final String? savedPath = _lastSavedRecordingPath;
    if (savedPath == null || savedPath.isEmpty) {
      return;
    }

    final String normalizedPath = savedPath.replaceAll('\\', '/');
    final int separatorIndex = normalizedPath.lastIndexOf('/');
    final String folderPath = separatorIndex > 0
        ? normalizedPath.substring(0, separatorIndex)
        : normalizedPath;

    final Uri folderUri = Uri.file(folderPath);
    final bool opened = await launchUrl(
      folderUri,
      mode: LaunchMode.externalApplication,
    );

    if (!mounted) {
      return;
    }

    if (!opened) {
      setState(() {
        _recordingError = 'Could not open folder: $folderPath';
      });
      return;
    }

    ScaffoldMessenger.of(
      context,
    ).showSnackBar(const SnackBar(content: Text('Folder opened')));
  }

  Future<void> _openSavedFile(String? filePath) async {
    if (filePath == null || filePath.isEmpty) {
      return;
    }

    final Uri fileUri = Uri.file(filePath);
    final bool opened = await launchUrl(
      fileUri,
      mode: LaunchMode.externalApplication,
    );

    if (!mounted) {
      return;
    }

    if (!opened) {
      setState(() {
        _recordingError = 'Could not open file: $filePath';
      });
      return;
    }

    ScaffoldMessenger.of(
      context,
    ).showSnackBar(const SnackBar(content: Text('File opened')));
  }

  void _scheduleAutoscrollIfNeeded() {
    if (!ref.read(rawBleDebugAutoscrollEnabledProvider)) {
      return;
    }
    if (_autoscrollFrameCallbackId != null) {
      return;
    }

    _autoscrollFrameCallbackId = SchedulerBinding.instance
        .scheduleFrameCallback((_) {
          _autoscrollFrameCallbackId = null;
          if (!mounted || !ref.read(rawBleDebugAutoscrollEnabledProvider)) {
            return;
          }
          if (!_scrollController.hasClients) {
            return;
          }

          _scrollController.animateTo(
            _scrollController.position.maxScrollExtent,
            duration: const Duration(milliseconds: 180),
            curve: Curves.easeOut,
          );
        });
  }

  void _cancelScheduledAutoscroll() {
    final int? callbackId = _autoscrollFrameCallbackId;
    if (callbackId != null) {
      SchedulerBinding.instance.cancelFrameCallbackWithId(callbackId);
      _autoscrollFrameCallbackId = null;
    }

    if (_scrollController.hasClients) {
      _scrollController.jumpTo(_scrollController.position.pixels);
    }
  }
}
