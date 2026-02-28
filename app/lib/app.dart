import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import 'core/ble/ble_providers.dart';
import 'core/ble/ble_service.dart';
import 'features/config/config_screen.dart';
import 'features/dashboard/dashboard_screen.dart';
import 'features/scanner/scanner_screen.dart';
import 'features/settings/settings_screen.dart';

class FlyInPeaceApp extends StatelessWidget {
  const FlyInPeaceApp({super.key});

  @override
  Widget build(BuildContext context) {
    const seedColor = Color(0xFF1E88E5);

    final lightColorScheme = ColorScheme.fromSeed(
      seedColor: seedColor,
      brightness: Brightness.light,
    );
    final darkColorScheme = ColorScheme.fromSeed(
      seedColor: seedColor,
      brightness: Brightness.dark,
    );

    return MaterialApp(
      title: 'FLY IN PEACE',
      debugShowCheckedModeBanner: false,
      themeMode: ThemeMode.system,
      theme: ThemeData(
        useMaterial3: true,
        colorScheme: lightColorScheme,
        textTheme: const TextTheme(
          headlineLarge: TextStyle(fontSize: 40, fontWeight: FontWeight.w700),
          titleLarge: TextStyle(fontSize: 22, fontWeight: FontWeight.w600),
          bodyLarge: TextStyle(fontSize: 16),
        ),
      ),
      darkTheme: ThemeData(
        useMaterial3: true,
        colorScheme: darkColorScheme,
        textTheme: const TextTheme(
          headlineLarge: TextStyle(fontSize: 40, fontWeight: FontWeight.w700),
          titleLarge: TextStyle(fontSize: 22, fontWeight: FontWeight.w600),
          bodyLarge: TextStyle(fontSize: 16),
        ),
      ),
      home: const AppShell(),
    );
  }
}

class AppLifecycleBlePolicy {
  const AppLifecycleBlePolicy._();

  static bool shouldDisconnectForState(AppLifecycleState state) {
    switch (state) {
      case AppLifecycleState.resumed:
        return false;
      case AppLifecycleState.inactive:
      case AppLifecycleState.hidden:
      case AppLifecycleState.paused:
      case AppLifecycleState.detached:
        return true;
    }
  }
}

class AppShell extends ConsumerStatefulWidget {
  const AppShell({super.key});

  @override
  ConsumerState<AppShell> createState() => _AppShellState();
}

class _AppShellState extends ConsumerState<AppShell>
    with WidgetsBindingObserver {
  int _selectedIndex = 0;
  late final BleService _bleService;

  static const List<Widget> _screens = <Widget>[
    ScannerScreen(),
    DashboardScreen(),
    ConfigScreen(),
    SettingsScreen(),
  ];

  static const List<NavigationDestination> _destinations =
      <NavigationDestination>[
        NavigationDestination(
          icon: Icon(Icons.bluetooth_searching),
          label: 'Scanner',
        ),
        NavigationDestination(icon: Icon(Icons.flight), label: 'Dashboard'),
        NavigationDestination(icon: Icon(Icons.tune), label: 'Config'),
        NavigationDestination(icon: Icon(Icons.settings), label: 'Settings'),
      ];

  @override
  void initState() {
    super.initState();
    _bleService = ref.read(bleServiceProvider);
    WidgetsBinding.instance.addObserver(this);
  }

  @override
  void didChangeAppLifecycleState(AppLifecycleState state) {
    if (!AppLifecycleBlePolicy.shouldDisconnectForState(state)) {
      return;
    }
    unawaited(_disconnectBleIfNeeded());
  }

  @override
  void dispose() {
    WidgetsBinding.instance.removeObserver(this);
    unawaited(_disconnectBleIfNeeded());
    super.dispose();
  }

  Future<void> _disconnectBleIfNeeded() async {
    if (_bleService.status == BleConnectionStatus.disconnected ||
        _bleService.status == BleConnectionStatus.disconnecting) {
      return;
    }
    await _bleService.disconnect();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: SafeArea(child: _screens[_selectedIndex]),
      bottomNavigationBar: NavigationBar(
        selectedIndex: _selectedIndex,
        destinations: _destinations,
        onDestinationSelected: (index) {
          setState(() {
            _selectedIndex = index;
          });
        },
      ),
    );
  }
}
