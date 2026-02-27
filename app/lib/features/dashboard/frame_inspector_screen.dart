import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter/scheduler.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../../core/ble/ble_providers.dart';
import '../../core/ble/ble_service.dart';
import '../../core/utils/lk8ex1_frame_verdict.dart';
import '../../core/utils/lk8ex1_parser.dart';

class InspectedFrame {
  const InspectedFrame({
    required this.timestamp,
    required this.sourceName,
    required this.sourceId,
    required this.sourceProfile,
    required this.parseResult,
    required this.verdict,
  });

  final DateTime timestamp;
  final String sourceName;
  final String sourceId;
  final BleCompatibilityProfile sourceProfile;
  final Lk8ex1ParseResult parseResult;
  final Lk8ex1FrameVerdictResult verdict;
}

class FrameInspectorScreen extends ConsumerStatefulWidget {
  const FrameInspectorScreen({super.key});

  @override
  ConsumerState<FrameInspectorScreen> createState() =>
      _FrameInspectorScreenState();
}

class _FrameInspectorScreenState extends ConsumerState<FrameInspectorScreen> {
  static const int maxStoredFrames = 500;

  final Lk8ex1Parser _parser = const Lk8ex1Parser();
  final Lk8ex1FrameVerdictEngine _verdictEngine =
      const Lk8ex1FrameVerdictEngine();
  final ScrollController _historyScrollController = ScrollController();

  final List<InspectedFrame> _frames = <InspectedFrame>[];
  StreamSubscription<String>? _receivedLinesSubscription;
  int? _selectedFrameIndex;
  int? _autoscrollFrameCallbackId;

  @override
  void initState() {
    super.initState();

    _receivedLinesSubscription = ref
        .read(bleServiceProvider)
        .receivedLines
        .listen(_handleLineReceived);
  }

  @override
  void dispose() {
    _receivedLinesSubscription?.cancel();
    _cancelScheduledAutoscroll();
    _historyScrollController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final BleService bleService = ref.watch(bleServiceProvider);
    final BleConnectionStatus status =
        ref.watch(bleConnectionStatusProvider).valueOrNull ?? bleService.status;
    final bool isConnected = status == BleConnectionStatus.connected;

    final BluetoothDevice? connectedDevice = bleService.connectedDevice;
    final String sourceName =
        connectedDevice?.platformName.trim().isNotEmpty == true
        ? connectedDevice!.platformName.trim()
        : 'Unknown';
    final String sourceId = connectedDevice?.remoteId.str ?? 'N/A';
    final BleCompatibilityProfile sourceProfile =
        BleService.detectCompatibilityProfile(sourceName);

    final InspectedFrame? selectedFrame =
        (_selectedFrameIndex != null && _selectedFrameIndex! < _frames.length)
        ? _frames[_selectedFrameIndex!]
        : null;

    return Scaffold(
      appBar: AppBar(
        title: const Text('Frame Inspector'),
        actions: [
          Row(
            mainAxisSize: MainAxisSize.min,
            children: [
              const Text('Auto'),
              Switch(
                value: ref.watch(frameInspectorAutoscrollEnabledProvider),
                onChanged: (bool enabled) {
                  if (!enabled) {
                    _cancelScheduledAutoscroll();
                  }
                  ref
                          .read(
                            frameInspectorAutoscrollEnabledProvider.notifier,
                          )
                          .state =
                      enabled;
                },
              ),
            ],
          ),
          IconButton(
            onPressed: _frames.isEmpty ? null : _clearFrames,
            icon: const Icon(Icons.clear_all),
            tooltip: 'Clear history',
          ),
        ],
      ),
      body: Column(
        children: [
          _SourceMetadataCard(
            isConnected: isConnected,
            sourceName: sourceName,
            sourceId: sourceId,
            sourceProfile: sourceProfile,
          ),
          Expanded(
            child: _frames.isEmpty
                ? const Center(child: Text('No frames received yet.'))
                : ListView.separated(
                    controller: _historyScrollController,
                    padding: const EdgeInsets.symmetric(
                      horizontal: 12,
                      vertical: 8,
                    ),
                    itemBuilder: (context, index) {
                      final InspectedFrame frame = _frames[index];
                      final bool isSelected = _selectedFrameIndex == index;

                      return ListTile(
                        selected: isSelected,
                        onTap: () {
                          setState(() {
                            _selectedFrameIndex = index;
                          });
                        },
                        title: Text(
                          _frameTitle(frame),
                          maxLines: 1,
                          overflow: TextOverflow.ellipsis,
                        ),
                        subtitle: Text(_timestampLabel(frame.timestamp)),
                        trailing: _VerdictChip(verdict: frame.verdict.verdict),
                      );
                    },
                    separatorBuilder: (_, _) => const Divider(height: 1),
                    itemCount: _frames.length,
                  ),
          ),
          _FrameDetailsCard(frame: selectedFrame),
        ],
      ),
    );
  }

  void _handleLineReceived(String line) {
    final BleService bleService = ref.read(bleServiceProvider);
    final BluetoothDevice? connectedDevice = bleService.connectedDevice;
    final String sourceName =
        connectedDevice?.platformName.trim().isNotEmpty == true
        ? connectedDevice!.platformName.trim()
        : 'Unknown';
    final String sourceId = connectedDevice?.remoteId.str ?? 'N/A';
    final BleCompatibilityProfile sourceProfile =
        BleService.detectCompatibilityProfile(sourceName);

    final Lk8ex1ParseResult parseResult = _parser.parseLine(line);
    final Lk8ex1FrameVerdictResult verdict = _verdictEngine.evaluate(
      parseResult,
    );

    setState(() {
      _frames.add(
        InspectedFrame(
          timestamp: DateTime.now(),
          sourceName: sourceName,
          sourceId: sourceId,
          sourceProfile: sourceProfile,
          parseResult: parseResult,
          verdict: verdict,
        ),
      );

      if (_frames.length > maxStoredFrames) {
        _frames.removeAt(0);
        if (_selectedFrameIndex != null) {
          _selectedFrameIndex = (_selectedFrameIndex! - 1).clamp(
            0,
            _frames.length - 1,
          );
        }
      }

      _selectedFrameIndex ??= _frames.length - 1;
    });

    _scheduleAutoscrollIfNeeded();
  }

  void _clearFrames() {
    setState(() {
      _frames.clear();
      _selectedFrameIndex = null;
    });
  }

  void _scheduleAutoscrollIfNeeded() {
    if (!ref.read(frameInspectorAutoscrollEnabledProvider)) {
      return;
    }
    if (_autoscrollFrameCallbackId != null) {
      return;
    }

    _autoscrollFrameCallbackId = SchedulerBinding.instance
        .scheduleFrameCallback((_) {
          _autoscrollFrameCallbackId = null;
          if (!mounted || !ref.read(frameInspectorAutoscrollEnabledProvider)) {
            return;
          }
          if (!_historyScrollController.hasClients) {
            return;
          }

          _historyScrollController.animateTo(
            _historyScrollController.position.maxScrollExtent,
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

    if (_historyScrollController.hasClients) {
      _historyScrollController.jumpTo(_historyScrollController.position.pixels);
    }
  }

  String _frameTitle(InspectedFrame frame) {
    final data = frame.parseResult.data;
    if (data == null) {
      return frame.parseResult.rawLine;
    }

    return 'P ${data.pressurePa} • Alt ${data.altitudeM} • V ${data.varioCms} • T ${data.temperatureDc} • B ${data.battery}';
  }

  String _timestampLabel(DateTime timestamp) {
    return '${timestamp.hour.toString().padLeft(2, '0')}:'
        '${timestamp.minute.toString().padLeft(2, '0')}:'
        '${timestamp.second.toString().padLeft(2, '0')}.'
        '${timestamp.millisecond.toString().padLeft(3, '0')}';
  }
}

class _SourceMetadataCard extends StatelessWidget {
  const _SourceMetadataCard({
    required this.isConnected,
    required this.sourceName,
    required this.sourceId,
    required this.sourceProfile,
  });

  final bool isConnected;
  final String sourceName;
  final String sourceId;
  final BleCompatibilityProfile sourceProfile;

  @override
  Widget build(BuildContext context) {
    return Card(
      margin: const EdgeInsets.fromLTRB(12, 10, 12, 8),
      child: Padding(
        padding: const EdgeInsets.all(12),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Row(
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
            const SizedBox(height: 8),
            Text('Source name: $sourceName'),
            Text('Source id: $sourceId'),
            Text('Source profile: ${_profileLabel(sourceProfile)}'),
          ],
        ),
      ),
    );
  }

  String _profileLabel(BleCompatibilityProfile profile) {
    switch (profile) {
      case BleCompatibilityProfile.flyInPeace:
        return 'FlyInPeace';
      case BleCompatibilityProfile.blueFlyVario:
        return 'BlueFlyVario';
      case BleCompatibilityProfile.unsupported:
        return 'Unsupported';
    }
  }
}

class _FrameDetailsCard extends StatelessWidget {
  const _FrameDetailsCard({required this.frame});

  final InspectedFrame? frame;

  @override
  Widget build(BuildContext context) {
    if (frame == null) {
      return const Padding(
        padding: EdgeInsets.fromLTRB(12, 0, 12, 12),
        child: Card(
          child: Padding(
            padding: EdgeInsets.all(12),
            child: Text('Select one frame to inspect details.'),
          ),
        ),
      );
    }

    final data = frame!.parseResult.data;
    final String checksumState = frame!.parseResult.hasChecksum
        ? (frame!.parseResult.checksumValid ? 'valid' : 'invalid')
        : 'missing';

    return Padding(
      padding: const EdgeInsets.fromLTRB(12, 0, 12, 12),
      child: Card(
        child: Padding(
          padding: const EdgeInsets.all(12),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            mainAxisSize: MainAxisSize.min,
            children: [
              Row(
                children: [
                  _VerdictChip(verdict: frame!.verdict.verdict),
                  const SizedBox(width: 8),
                  Expanded(child: Text(frame!.verdict.reason)),
                ],
              ),
              const SizedBox(height: 8),
              Text('Timestamp: ${_timestampLabel(frame!.timestamp)}'),
              Text('Checksum: $checksumState'),
              if (data != null) ...[
                Text('Pressure: ${data.pressurePa} Pa'),
                Text('Altitude: ${data.altitudeM} m'),
                Text('Vario: ${data.varioCms} cm/s'),
                Text('Temperature: ${data.temperatureDc} (°C×10)'),
                Text('Battery: ${data.battery}'),
              ],
              const SizedBox(height: 6),
              Text(
                'Raw: ${frame!.parseResult.rawLine}',
                maxLines: 3,
                overflow: TextOverflow.ellipsis,
              ),
            ],
          ),
        ),
      ),
    );
  }

  String _timestampLabel(DateTime timestamp) {
    return '${timestamp.hour.toString().padLeft(2, '0')}:'
        '${timestamp.minute.toString().padLeft(2, '0')}:'
        '${timestamp.second.toString().padLeft(2, '0')}.'
        '${timestamp.millisecond.toString().padLeft(3, '0')}';
  }
}

class _VerdictChip extends StatelessWidget {
  const _VerdictChip({required this.verdict});

  final Lk8ex1FrameVerdict verdict;

  @override
  Widget build(BuildContext context) {
    final ColorScheme colorScheme = Theme.of(context).colorScheme;
    late final Color background;
    late final Color foreground;
    late final String label;

    switch (verdict) {
      case Lk8ex1FrameVerdict.valid:
        background = colorScheme.secondaryContainer;
        foreground = colorScheme.onSecondaryContainer;
        label = 'valid';
        break;
      case Lk8ex1FrameVerdict.warning:
        background = colorScheme.tertiaryContainer;
        foreground = colorScheme.onTertiaryContainer;
        label = 'warning';
        break;
      case Lk8ex1FrameVerdict.error:
        background = colorScheme.errorContainer;
        foreground = colorScheme.onErrorContainer;
        label = 'error';
        break;
    }

    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 2),
      decoration: BoxDecoration(
        color: background,
        borderRadius: BorderRadius.circular(12),
      ),
      child: Text(
        label,
        style: Theme.of(context).textTheme.labelSmall?.copyWith(
          color: foreground,
          fontWeight: FontWeight.w600,
        ),
      ),
    );
  }
}
