import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../../core/ble/ble_providers.dart';
import '../../core/ble/ble_service.dart';
import '../../widgets/connection_indicator.dart';
import '../../widgets/value_display.dart';
import 'dashboard_provider.dart';

class DashboardScreen extends ConsumerStatefulWidget {
  const DashboardScreen({super.key});

  @override
  ConsumerState<DashboardScreen> createState() => _DashboardScreenState();
}

class _DashboardScreenState extends ConsumerState<DashboardScreen> {
  bool _manualDisconnectPending = false;

  @override
  Widget build(BuildContext context) {
    final connected = ref.watch(dashboardConnectedProvider);
    final BleReconnectState reconnectState = ref.watch(
      dashboardReconnectStateProvider,
    );
    final bool showDisconnectedOverlay = ref.watch(
      dashboardShowDisconnectedOverlayProvider,
    );

    ref.listen<BleConnectionStatus>(dashboardConnectionStatusProvider, (
      previous,
      next,
    ) {
      if (previous == BleConnectionStatus.connected &&
          next == BleConnectionStatus.disconnected &&
          !_manualDisconnectPending) {
        ScaffoldMessenger.of(
          context,
        ).showSnackBar(const SnackBar(content: Text('Connection lost.')));
      }

      if (_manualDisconnectPending &&
          next == BleConnectionStatus.disconnected) {
        _manualDisconnectPending = false;
      }
    });

    return Scaffold(
      appBar: AppBar(title: const Text('Dashboard')),
      body: Stack(
        children: [
          ListView(
            padding: const EdgeInsets.all(16),
            children: [
              ConnectionIndicator(connected: connected),
              const SizedBox(height: 16),
              const ValueDisplay(label: 'Altitude', value: '--', unit: 'm'),
              const SizedBox(height: 8),
              const ValueDisplay(label: 'Vario', value: '--', unit: 'm/s'),
              const SizedBox(height: 8),
              const ValueDisplay(label: 'Pressure', value: '--', unit: 'Pa'),
              const SizedBox(height: 16),
              if (connected)
                FilledButton.icon(
                  onPressed: () async {
                    final navigator = Navigator.of(context);
                    final scaffoldMessenger = ScaffoldMessenger.of(context);

                    _manualDisconnectPending = true;
                    await ref.read(bleServiceProvider).disconnect();
                    if (!mounted) {
                      return;
                    }

                    scaffoldMessenger.showSnackBar(
                      const SnackBar(content: Text('Disconnected.')),
                    );
                    navigator.maybePop();
                  },
                  icon: const Icon(Icons.link_off),
                  label: const Text('Disconnect'),
                ),
            ],
          ),
          if (reconnectState.isReconnecting)
            _StatusOverlay(
              label:
                  'Reconnecting... (${reconnectState.attempt}/${reconnectState.maxAttempts})',
            ),
          if (showDisconnectedOverlay)
            const _StatusOverlay(label: 'Disconnected'),
        ],
      ),
    );
  }
}

class _StatusOverlay extends StatelessWidget {
  const _StatusOverlay({required this.label});

  final String label;

  @override
  Widget build(BuildContext context) {
    return ColoredBox(
      color: Colors.black54,
      child: Center(
        child: Container(
          padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 10),
          decoration: BoxDecoration(
            color: Theme.of(context).colorScheme.surface,
            borderRadius: BorderRadius.circular(12),
          ),
          child: Text(label, style: Theme.of(context).textTheme.titleMedium),
        ),
      ),
    );
  }
}
