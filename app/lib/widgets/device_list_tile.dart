import 'package:flutter/material.dart';

import '../core/ble/ble_service.dart';

class DeviceListTile extends StatelessWidget {
  const DeviceListTile({
    super.key,
    required this.device,
    required this.onTap,
    required this.onConnect,
  });

  final BleScanDevice device;
  final VoidCallback onTap;
  final VoidCallback onConnect;

  @override
  Widget build(BuildContext context) {
    final bool isCompatible = device.isFlyInPeaceCompatible;
    final ColorScheme colorScheme = Theme.of(context).colorScheme;
    final String compatibilityLabel = _profileLabel(
      device.compatibilityProfile,
    );

    return Card(
      child: ListTile(
        onTap: onTap,
        leading: Icon(
          _rssiIcon(device.rssi),
          color: isCompatible ? colorScheme.primary : colorScheme.outline,
        ),
        title: Text(device.name.isEmpty ? 'Unknown' : device.name),
        subtitle: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          mainAxisSize: MainAxisSize.min,
          children: [
            Text('${device.remoteId} • ${device.rssi} dBm'),
            const SizedBox(height: 4),
            Wrap(
              spacing: 6,
              children: [
                _CompatibilityChip(
                  label: compatibilityLabel,
                  isCompatible: isCompatible,
                ),
                _CompatibilityChip(
                  label: _protocolLabel(device),
                  isCompatible: isCompatible,
                ),
              ],
            ),
          ],
        ),
        trailing: FilledButton(
          onPressed: isCompatible ? onConnect : null,
          child: const Text('Connect'),
        ),
      ),
    );
  }

  String _profileLabel(BleCompatibilityProfile profile) {
    switch (profile) {
      case BleCompatibilityProfile.flyInPeace:
        return 'FlyInPeace profiled';
      case BleCompatibilityProfile.blueFlyVario:
        return 'BlueFlyVario profiled';
      case BleCompatibilityProfile.unsupported:
        return 'Unsupported / non-profiled';
    }
  }

  String _protocolLabel(BleScanDevice device) {
    if (device.hasNusService) {
      return 'Protocol: NUS';
    }
    if (device.hasBlueFlyService) {
      return 'Protocol: BlueFly UART';
    }
    return 'Protocol: Generic UART';
  }

  IconData _rssiIcon(int rssi) {
    if (rssi >= -60) {
      return Icons.signal_cellular_4_bar;
    }
    if (rssi >= -70) {
      return Icons.signal_cellular_alt;
    }
    if (rssi >= -80) {
      return Icons.signal_cellular_alt_2_bar;
    }
    return Icons.signal_cellular_0_bar;
  }
}

class _CompatibilityChip extends StatelessWidget {
  const _CompatibilityChip({required this.label, required this.isCompatible});

  final String label;
  final bool isCompatible;

  @override
  Widget build(BuildContext context) {
    final ColorScheme colorScheme = Theme.of(context).colorScheme;
    final Color foreground = isCompatible
        ? colorScheme.onSecondaryContainer
        : colorScheme.onSurfaceVariant;
    final Color background = isCompatible
        ? colorScheme.secondaryContainer
        : colorScheme.surfaceContainerHighest;

    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 2),
      decoration: BoxDecoration(
        color: background,
        borderRadius: BorderRadius.circular(12),
      ),
      child: Text(
        label,
        style: Theme.of(
          context,
        ).textTheme.labelSmall?.copyWith(color: foreground),
      ),
    );
  }
}
