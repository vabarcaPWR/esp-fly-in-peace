import 'package:flutter/widgets.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:fly_in_peace/app.dart';

void main() {
  group('AppLifecycleBlePolicy', () {
    test('disconnects on non-resumed lifecycle states', () {
      expect(
        AppLifecycleBlePolicy.shouldDisconnectForState(
          AppLifecycleState.inactive,
        ),
        isTrue,
      );
      expect(
        AppLifecycleBlePolicy.shouldDisconnectForState(
          AppLifecycleState.paused,
        ),
        isTrue,
      );
      expect(
        AppLifecycleBlePolicy.shouldDisconnectForState(
          AppLifecycleState.detached,
        ),
        isTrue,
      );
      expect(
        AppLifecycleBlePolicy.shouldDisconnectForState(
          AppLifecycleState.hidden,
        ),
        isTrue,
      );
    });

    test('keeps connection on resumed state', () {
      expect(
        AppLifecycleBlePolicy.shouldDisconnectForState(
          AppLifecycleState.resumed,
        ),
        isFalse,
      );
    });
  });
}
