import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import 'config_provider.dart';

class ConfigScreen extends ConsumerWidget {
  const ConfigScreen({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final bleName = ref.watch(configBleNameProvider);

    return Scaffold(
      appBar: AppBar(title: const Text('Config')),
      body: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text(
              'Device BLE Name',
              style: Theme.of(context).textTheme.titleLarge,
            ),
            const SizedBox(height: 8),
            Text(bleName),
            const SizedBox(height: 16),
            ElevatedButton(
              onPressed: () {
                ref.read(configBleNameProvider.notifier).state =
                    'FlyInPeace-MVP';
              },
              child: const Text('Apply placeholder update'),
            ),
          ],
        ),
      ),
    );
  }
}
