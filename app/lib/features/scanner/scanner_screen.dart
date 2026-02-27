import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../../core/ble/ble_service.dart';
import '../../widgets/device_list_tile.dart';
import '../dashboard/dashboard_screen.dart';
import 'scanner_provider.dart';

class ScannerScreen extends ConsumerStatefulWidget {
  const ScannerScreen({super.key});

  @override
  ConsumerState<ScannerScreen> createState() => _ScannerScreenState();
}

class _ScannerScreenState extends ConsumerState<ScannerScreen> {
  bool _hasRequestedInitialScan = false;

  @override
  void didChangeDependencies() {
    super.didChangeDependencies();
    if (_hasRequestedInitialScan) {
      return;
    }

    _hasRequestedInitialScan = true;
    WidgetsBinding.instance.addPostFrameCallback((_) {
      if (!mounted) {
        return;
      }
      ref.read(scannerControllerProvider.notifier).startScan();
    });
  }

  @override
  Widget build(BuildContext context) {
    final ScannerState state = ref.watch(scannerControllerProvider);
    final visibleDevices = state.visibleDevices;

    ref.listen<ScannerState>(scannerControllerProvider, (previous, next) {
      final ScannerDialogRequest? request = next.dialogRequest;
      if (request == null || request == previous?.dialogRequest) {
        return;
      }

      WidgetsBinding.instance.addPostFrameCallback((_) async {
        if (!mounted) {
          return;
        }

        await showDialog<void>(
          context: context,
          builder: (dialogContext) {
            return AlertDialog(
              title: Text(request.title),
              content: Text(request.message),
              actions: [
                TextButton(
                  onPressed: () {
                    Navigator.of(dialogContext).pop();
                  },
                  child: const Text('Close'),
                ),
                if (request.action != ScannerDialogAction.none)
                  ElevatedButton(
                    onPressed: () async {
                      await ref
                          .read(scannerControllerProvider.notifier)
                          .handleDialogAction();
                      if (dialogContext.mounted) {
                        Navigator.of(dialogContext).pop();
                      }
                    },
                    child: Text(
                      request.action == ScannerDialogAction.turnOnBluetooth
                          ? 'Turn on Bluetooth'
                          : 'Open settings',
                    ),
                  ),
              ],
            );
          },
        );

        ref.read(scannerControllerProvider.notifier).clearDialog();
      });
    });

    return Scaffold(
      appBar: AppBar(title: const Text('Scanner')),
      body: Column(
        children: [
          Padding(
            padding: const EdgeInsets.fromLTRB(12, 8, 12, 0),
            child: Card(
              child: ListTile(
                leading: Icon(
                  state.isConnected
                      ? Icons.bluetooth_connected
                      : Icons.bluetooth_disabled,
                  color: state.isConnected
                      ? Theme.of(context).colorScheme.primary
                      : Theme.of(context).colorScheme.outline,
                ),
                title: Text(state.isConnected ? 'Connected' : 'Disconnected'),
                subtitle: state.connectedDeviceId == null
                    ? null
                    : Text('Device: ${state.connectedDeviceId}'),
                trailing: state.isConnected
                    ? FilledButton.icon(
                        onPressed:
                            state.connectionStatus ==
                                BleConnectionStatus.disconnecting
                            ? null
                            : () async {
                                await ref
                                    .read(scannerControllerProvider.notifier)
                                    .disconnectDevice();
                              },
                        icon: const Icon(Icons.link_off),
                        label: const Text('Disconnect'),
                      )
                    : null,
              ),
            ),
          ),
          Padding(
            padding: const EdgeInsets.fromLTRB(12, 8, 12, 0),
            child: Align(
              alignment: Alignment.centerLeft,
              child: SegmentedButton<ScannerDeviceFilter>(
                segments: const [
                  ButtonSegment<ScannerDeviceFilter>(
                    value: ScannerDeviceFilter.all,
                    label: Text('All'),
                  ),
                  ButtonSegment<ScannerDeviceFilter>(
                    value: ScannerDeviceFilter.compatible,
                    label: Text('Compatible'),
                  ),
                ],
                selected: <ScannerDeviceFilter>{state.deviceFilter},
                onSelectionChanged: (selection) {
                  final ScannerDeviceFilter selected = selection.first;
                  ref
                      .read(scannerControllerProvider.notifier)
                      .setDeviceFilter(selected);
                },
              ),
            ),
          ),
          if (state.isScanning)
            LinearProgressIndicator(value: state.scanProgress.clamp(0.0, 1.0)),
          if (state.isScanning)
            const ListTile(
              leading: SizedBox(
                width: 20,
                height: 20,
                child: CircularProgressIndicator(strokeWidth: 2),
              ),
              title: Text('Scanning for BLE devices...'),
            ),
          if (state.isConnecting)
            ListTile(
              leading: const SizedBox(
                width: 20,
                height: 20,
                child: CircularProgressIndicator(strokeWidth: 2),
              ),
              title: Text(
                state.connectingDeviceId == null
                    ? 'Connecting...'
                    : 'Connecting to ${state.connectingDeviceId}...',
              ),
            ),
          Expanded(
            child: RefreshIndicator(
              onRefresh: () async {
                await ref
                    .read(scannerControllerProvider.notifier)
                    .refreshScan();
              },
              child: visibleDevices.isEmpty
                  ? ListView(
                      physics: const AlwaysScrollableScrollPhysics(),
                      padding: const EdgeInsets.all(16),
                      children: [
                        const SizedBox(height: 80),
                        Text(
                          state.deviceFilter == ScannerDeviceFilter.all
                              ? 'No BLE devices found nearby.'
                              : 'No profiled devices (FlyInPeace / BlueFlyVario) found.',
                          textAlign: TextAlign.center,
                          style: Theme.of(context).textTheme.titleMedium,
                        ),
                        if (state.showScanAgain) ...[
                          const SizedBox(height: 20),
                          Center(
                            child: ElevatedButton.icon(
                              onPressed: () {
                                ref
                                    .read(scannerControllerProvider.notifier)
                                    .startScan();
                              },
                              icon: const Icon(Icons.refresh),
                              label: const Text('Scan again'),
                            ),
                          ),
                        ],
                      ],
                    )
                  : ListView.separated(
                      physics: const AlwaysScrollableScrollPhysics(),
                      padding: const EdgeInsets.all(12),
                      itemBuilder: (context, index) {
                        final device = visibleDevices[index];

                        void handleIncompatibleTap() {
                          ScaffoldMessenger.of(context).showSnackBar(
                            const SnackBar(
                              content: Text(
                                'This device is unsupported or non-profiled.',
                              ),
                            ),
                          );
                        }

                        Future<void> connectAndOpenDashboard() async {
                          if (state.isConnecting) {
                            return;
                          }

                          final bool connected = await ref
                              .read(scannerControllerProvider.notifier)
                              .connectToDevice(device);
                          if (!mounted || !connected) {
                            return;
                          }

                          await Navigator.of(this.context).push(
                            MaterialPageRoute<void>(
                              builder: (_) => const DashboardScreen(),
                            ),
                          );
                        }

                        return DeviceListTile(
                          device: device,
                          onTap: () async {
                            if (!device.isFlyInPeaceCompatible) {
                              handleIncompatibleTap();
                              return;
                            }

                            await connectAndOpenDashboard();
                          },
                          onConnect: () async {
                            if (!device.isFlyInPeaceCompatible) {
                              handleIncompatibleTap();
                              return;
                            }

                            await connectAndOpenDashboard();
                          },
                        );
                      },
                      separatorBuilder: (_, _) => const SizedBox(height: 8),
                      itemCount: visibleDevices.length,
                    ),
            ),
          ),
        ],
      ),
      floatingActionButton: FloatingActionButton.extended(
        onPressed: () async {
          final controller = ref.read(scannerControllerProvider.notifier);
          if (state.isScanning) {
            await controller.stopScan();
          } else {
            await controller.startScan();
          }
        },
        icon: Icon(state.isScanning ? Icons.stop : Icons.search),
        label: Text(state.isScanning ? 'Stop' : 'Scan'),
      ),
    );
  }
}
