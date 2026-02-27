import 'package:flutter/material.dart';

import 'frame_inspector_screen.dart';
import 'raw_data_debug_screen.dart';

class BleDebugConsoleScreen extends StatefulWidget {
  const BleDebugConsoleScreen({super.key});

  @override
  State<BleDebugConsoleScreen> createState() => _BleDebugConsoleScreenState();
}

class _BleDebugConsoleScreenState extends State<BleDebugConsoleScreen> {
  int _selectedIndex = 0;

  static const List<Widget> _pages = <Widget>[
    FrameInspectorScreen(),
    RawDataDebugScreen(),
  ];

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: IndexedStack(index: _selectedIndex, children: _pages),
      bottomNavigationBar: NavigationBar(
        selectedIndex: _selectedIndex,
        onDestinationSelected: (int index) {
          setState(() {
            _selectedIndex = index;
          });
        },
        destinations: const <NavigationDestination>[
          NavigationDestination(
            icon: Icon(Icons.data_object),
            label: 'Frame Inspector',
          ),
          NavigationDestination(
            icon: Icon(Icons.developer_mode),
            label: 'Raw BLE Debug',
          ),
        ],
      ),
    );
  }
}
