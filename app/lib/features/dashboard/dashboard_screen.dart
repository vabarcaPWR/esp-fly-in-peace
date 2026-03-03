import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../../core/ble/ble_providers.dart';
import '../../core/ble/ble_service.dart';
import '../../core/models/lk8ex1_data.dart';
import 'dashboard_provider.dart';
import 'model/dashboard_model.dart';

class DashboardScreen extends ConsumerStatefulWidget {
  const DashboardScreen({super.key});

  @override
  ConsumerState<DashboardScreen> createState() => _DashboardScreenState();
}

class _DashboardScreenState extends ConsumerState<DashboardScreen> {
  bool _manualDisconnectPending = false;
  bool _darkBackgroundEnabled = true;

  @override
  Widget build(BuildContext context) {
    final DashboardModel model = ref.watch(dashboardModelProvider);
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
      appBar: AppBar(
        title: const Text('Dashboard'),
        actions: [
          IconButton(
            tooltip: _darkBackgroundEnabled
                ? 'Use default background'
                : 'Use dark background',
            onPressed: () {
              setState(() {
                _darkBackgroundEnabled = !_darkBackgroundEnabled;
              });
            },
            icon: Icon(
              _darkBackgroundEnabled ? Icons.dark_mode : Icons.light_mode,
            ),
          ),
          TextButton.icon(
            onPressed: () async {
              final scaffoldMessenger = ScaffoldMessenger.of(context);
              _manualDisconnectPending = true;
              await ref.read(bleServiceProvider).disconnect();
              if (!mounted) {
                return;
              }
              scaffoldMessenger.showSnackBar(
                const SnackBar(content: Text('Disconnected.')),
              );
            },
            icon: const Icon(Icons.link_off),
            label: const Text('Disconnect'),
          ),
        ],
      ),
      body: Stack(
        children: [
          ColoredBox(
            color: _darkBackgroundEnabled
                ? Colors.black
                : Theme.of(context).colorScheme.surface,
            child: SafeArea(
              bottom: false,
              child: LayoutBuilder(
                builder: (context, constraints) {
                  final bool isTablet = constraints.maxWidth >= 700;
                  final double spacing = isTablet ? 16 : 12;

                  return SingleChildScrollView(
                    padding: EdgeInsets.all(spacing),
                    child: Column(
                      crossAxisAlignment: CrossAxisAlignment.stretch,
                      children: [
                        _ConnectionStatusBar(
                          model: model,
                          onTap: () {
                            _showConnectionDetails(context, model);
                          },
                        ),
                        if (model.isStale) ...[
                          SizedBox(height: spacing),
                          _StaleDataBanner(lastUpdatedAt: model.lastUpdatedAt),
                        ],
                        SizedBox(height: spacing),
                        isTablet
                            ? Row(
                                crossAxisAlignment: CrossAxisAlignment.start,
                                children: [
                                  Expanded(
                                    child: _AltitudeCard(
                                      data: model.latestData,
                                    ),
                                  ),
                                  SizedBox(width: spacing),
                                  Expanded(
                                    child: _VarioCard(
                                      data: model.latestData,
                                      trend: model.varioTrend,
                                    ),
                                  ),
                                ],
                              )
                            : Column(
                                children: [
                                  _AltitudeCard(data: model.latestData),
                                  SizedBox(height: spacing),
                                  _VarioCard(
                                    data: model.latestData,
                                    trend: model.varioTrend,
                                  ),
                                ],
                              ),
                        SizedBox(height: spacing),
                        GridView.count(
                          shrinkWrap: true,
                          physics: const NeverScrollableScrollPhysics(),
                          crossAxisCount: isTablet ? 2 : 1,
                          mainAxisSpacing: spacing,
                          crossAxisSpacing: spacing,
                          childAspectRatio: isTablet ? 2.8 : 3.0,
                          children: [
                            _SecondaryValueCard(
                              label: 'Pressure',
                              value: DashboardFormatter.formatPressureHpa(
                                model.latestData,
                              ),
                              unit: 'hPa',
                            ),
                            _SecondaryValueCard(
                              label: 'Temperature',
                              value: DashboardFormatter.formatTemperatureC(
                                model.latestData,
                              ),
                              unit: '°C',
                            ),
                          ],
                        ),
                        if (model.lastStreamError != null) ...[
                          SizedBox(height: spacing),
                          Card(
                            child: Padding(
                              padding: const EdgeInsets.all(12),
                              child: Text(
                                'Stream warning: ${model.lastStreamError}',
                                style: Theme.of(context).textTheme.bodyMedium,
                              ),
                            ),
                          ),
                        ],
                      ],
                    ),
                  );
                },
              ),
            ),
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

void _showConnectionDetails(BuildContext context, DashboardModel model) {
  showModalBottomSheet<void>(
    context: context,
    builder: (context) {
      return Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          mainAxisSize: MainAxisSize.min,
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text(
              'Connection details',
              style: Theme.of(context).textTheme.titleLarge,
            ),
            const SizedBox(height: 8),
            Text('State: ${model.connectionLabel}'),
            Text('Status: ${model.connectionStatus.name}'),
            if (model.reconnectState.isReconnecting)
              Text(
                'Reconnect attempt ${model.reconnectState.attempt}/${model.reconnectState.maxAttempts}',
              ),
          ],
        ),
      );
    },
  );
}

class _ConnectionStatusBar extends StatelessWidget {
  const _ConnectionStatusBar({required this.model, required this.onTap});

  final DashboardModel model;
  final VoidCallback onTap;

  @override
  Widget build(BuildContext context) {
    final Color indicatorColor;
    if (model.reconnectState.isReconnecting) {
      indicatorColor = Colors.orange;
    } else if (model.connectionStatus == BleConnectionStatus.connected) {
      indicatorColor = Colors.green;
    } else {
      indicatorColor = Colors.red;
    }

    return Card(
      child: InkWell(
        borderRadius: BorderRadius.circular(12),
        onTap: onTap,
        child: Padding(
          padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 14),
          child: Row(
            children: [
              Container(
                width: 12,
                height: 12,
                decoration: BoxDecoration(
                  color: indicatorColor,
                  shape: BoxShape.circle,
                ),
              ),
              const SizedBox(width: 10),
              Expanded(
                child: Text(
                  model.connectionLabel,
                  style: Theme.of(context).textTheme.titleMedium,
                ),
              ),
              const Icon(Icons.chevron_right),
            ],
          ),
        ),
      ),
    );
  }
}

class _StaleDataBanner extends StatelessWidget {
  const _StaleDataBanner({required this.lastUpdatedAt});

  final DateTime? lastUpdatedAt;

  @override
  Widget build(BuildContext context) {
    final String lastUpdatedText = lastUpdatedAt == null
        ? 'unknown'
        : lastUpdatedAt!.toLocal().toIso8601String();

    return Card(
      child: Padding(
        padding: const EdgeInsets.all(12),
        child: Row(
          children: [
            const Icon(Icons.warning_amber_rounded),
            const SizedBox(width: 8),
            Expanded(
              child: Text('Showing stale data. Last update: $lastUpdatedText'),
            ),
          ],
        ),
      ),
    );
  }
}

class _AltitudeCard extends StatelessWidget {
  const _AltitudeCard({required this.data});

  final Lk8ex1Data? data;

  @override
  Widget build(BuildContext context) {
    return Card(
      child: Padding(
        padding: const EdgeInsets.all(18),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text('Altitude', style: Theme.of(context).textTheme.titleMedium),
            const SizedBox(height: 12),
            Row(
              crossAxisAlignment: CrossAxisAlignment.end,
              children: [
                Expanded(
                  child: Text(
                    DashboardFormatter.formatAltitudeMeters(data),
                    maxLines: 1,
                    overflow: TextOverflow.ellipsis,
                    style: Theme.of(context).textTheme.displayMedium,
                  ),
                ),
                const SizedBox(width: 8),
                Padding(
                  padding: const EdgeInsets.only(bottom: 10),
                  child: Text(
                    'm',
                    style: Theme.of(context).textTheme.titleLarge,
                  ),
                ),
              ],
            ),
          ],
        ),
      ),
    );
  }
}

class _VarioCard extends StatelessWidget {
  const _VarioCard({required this.data, required this.trend});

  final Lk8ex1Data? data;
  final DashboardVarioTrend trend;

  @override
  Widget build(BuildContext context) {
    final Color trendColor;
    final IconData trendIcon;

    switch (trend) {
      case DashboardVarioTrend.climb:
        trendColor = Colors.green;
        trendIcon = Icons.trending_up;
        break;
      case DashboardVarioTrend.sink:
        trendColor = Colors.red;
        trendIcon = Icons.trending_down;
        break;
      case DashboardVarioTrend.neutral:
        trendColor = Colors.grey;
        trendIcon = Icons.trending_flat;
        break;
    }

    return Card(
      child: Padding(
        padding: const EdgeInsets.all(18),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text('Vario', style: Theme.of(context).textTheme.titleMedium),
            const SizedBox(height: 12),
            Row(
              crossAxisAlignment: CrossAxisAlignment.end,
              children: [
                Expanded(
                  child: Text(
                    DashboardFormatter.formatVarioMs(data),
                    maxLines: 1,
                    overflow: TextOverflow.ellipsis,
                    style: Theme.of(
                      context,
                    ).textTheme.displayMedium?.copyWith(color: trendColor),
                  ),
                ),
                const SizedBox(width: 8),
                Padding(
                  padding: const EdgeInsets.only(bottom: 10),
                  child: Text(
                    'm/s',
                    style: Theme.of(context).textTheme.titleLarge,
                  ),
                ),
              ],
            ),
            const SizedBox(height: 8),
            Row(
              children: [
                Icon(trendIcon, color: trendColor),
                const SizedBox(width: 6),
                Text(
                  'Dead zone: ±0.1 m/s',
                  style: Theme.of(context).textTheme.bodySmall,
                ),
              ],
            ),
          ],
        ),
      ),
    );
  }
}

class _SecondaryValueCard extends StatelessWidget {
  const _SecondaryValueCard({
    required this.label,
    required this.value,
    required this.unit,
  });

  final String label;
  final String value;
  final String unit;

  @override
  Widget build(BuildContext context) {
    return Card(
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Text(label, style: Theme.of(context).textTheme.titleSmall),
            const SizedBox(height: 6),
            Row(
              crossAxisAlignment: CrossAxisAlignment.end,
              children: [
                Text(value, style: Theme.of(context).textTheme.headlineSmall),
                const SizedBox(width: 8),
                Padding(
                  padding: const EdgeInsets.only(bottom: 2),
                  child: Text(
                    unit,
                    style: Theme.of(context).textTheme.bodyLarge,
                  ),
                ),
              ],
            ),
          ],
        ),
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
