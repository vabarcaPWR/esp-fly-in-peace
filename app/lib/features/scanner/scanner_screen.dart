import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../../widgets/connection_indicator.dart';
import 'scanner_provider.dart';

class ScannerScreen extends ConsumerWidget {
  const ScannerScreen({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final refreshCount = ref.watch(scannerRefreshCountProvider);

    return Scaffold(
      appBar: AppBar(title: const Text('Scanner')),
      body: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            const ConnectionIndicator(connected: false),
            const SizedBox(height: 16),
            Text(
              'Phase 0 scanner placeholder',
              style: Theme.of(context).textTheme.titleLarge,
            ),
            const SizedBox(height: 8),
            Text('Refresh count: $refreshCount'),
            const SizedBox(height: 16),
            ElevatedButton.icon(
              onPressed: () {
                ref.read(scannerRefreshCountProvider.notifier).state++;
              },
              icon: const Icon(Icons.refresh),
              label: const Text('Refresh placeholder state'),
            ),
          ],
        ),
      ),
    );
  }
}
