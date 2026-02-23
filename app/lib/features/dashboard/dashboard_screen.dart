import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../../widgets/connection_indicator.dart';
import '../../widgets/value_display.dart';
import 'dashboard_provider.dart';

class DashboardScreen extends ConsumerWidget {
  const DashboardScreen({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final connected = ref.watch(dashboardConnectedProvider);

    return Scaffold(
      appBar: AppBar(title: const Text('Dashboard')),
      body: ListView(
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
          OutlinedButton.icon(
            onPressed: () {
              ref.read(dashboardConnectedProvider.notifier).state = !connected;
            },
            icon: const Icon(Icons.sync),
            label: const Text('Toggle connection placeholder'),
          ),
        ],
      ),
    );
  }
}
