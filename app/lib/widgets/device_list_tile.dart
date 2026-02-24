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
    return Card(
      child: ListTile(
        onTap: onTap,
        leading: Icon(
          _rssiIcon(device.rssi),
          color: Theme.of(context).colorScheme.primary,
        ),
        title: Text(device.name.isEmpty ? 'Unknown' : device.name),
        subtitle: Text('${device.remoteId} • ${device.rssi} dBm'),
        trailing: FilledButton(
          onPressed: onConnect,
          child: const Text('Connect'),
        ),
      ),
    );
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
