// This is a basic Flutter widget test.
//
// To perform an interaction with a widget in your test, use the WidgetTester
// utility in the flutter_test package. For example, you can send tap and scroll
// gestures. You can also use WidgetTester to find child widgets in the widget
// tree, read text, and verify that the values of widget properties are correct.

import 'package:flutter_test/flutter_test.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import 'package:fly_in_peace/app.dart';

void main() {
  testWidgets('Phase 0 navigation shows placeholder screens', (
    WidgetTester tester,
  ) async {
    await tester.pumpWidget(const ProviderScope(child: FlyInPeaceApp()));
    await tester.pumpAndSettle();

    final closeDialogFinder = find.text('Close');
    if (closeDialogFinder.evaluate().isNotEmpty) {
      await tester.tap(closeDialogFinder.first);
      await tester.pumpAndSettle();
    }

    expect(find.text('Scanner'), findsNWidgets(2));

    await tester.tap(find.text('Dashboard').first);
    await tester.pumpAndSettle();
    expect(find.text('Dashboard'), findsWidgets);

    await tester.tap(find.text('Config'));
    await tester.pumpAndSettle();
    expect(find.text('Device BLE Name'), findsOneWidget);

    await tester.tap(find.text('Settings'));
    await tester.pump();
    expect(find.text('Dark mode (placeholder preference)'), findsOneWidget);
  });
}
