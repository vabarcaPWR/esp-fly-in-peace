import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../../core/ble/ble_providers.dart';
import '../../core/ble/ble_service.dart';

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
  final ScrollController _scrollController = ScrollController();
  final List<RawDataEntry> _entries = <RawDataEntry>[];
  StreamSubscription<String>? _receivedLinesSubscription;
  String? _sendError;

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

          WidgetsBinding.instance.addPostFrameCallback((_) {
            if (!_scrollController.hasClients) {
              return;
            }

            _scrollController.animateTo(
              _scrollController.position.maxScrollExtent,
              duration: const Duration(milliseconds: 180),
              curve: Curves.easeOut,
            );
          });
        });
  }

  @override
  void dispose() {
    _receivedLinesSubscription?.cancel();
    _sendController.dispose();
    _scrollController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final BleConnectionStatus connectionStatus =
        ref.watch(bleConnectionStatusProvider).valueOrNull ??
        ref.read(bleServiceProvider).status;
    final bool isConnected = connectionStatus == BleConnectionStatus.connected;

    return Scaffold(
      appBar: AppBar(
        title: const Text('Raw BLE Debug'),
        actions: [
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
}
