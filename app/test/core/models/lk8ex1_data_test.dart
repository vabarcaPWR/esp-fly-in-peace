import 'package:flutter_test/flutter_test.dart';
import 'package:fly_in_peace/core/models/lk8ex1_data.dart';

void main() {
  group('Lk8ex1Data model', () {
    test('supports copyWith and value equality', () {
      final DateTime timestamp = DateTime.utc(2026, 2, 28, 21, 0, 0);
      final Lk8ex1Data original = Lk8ex1Data(
        pressurePa: 101325,
        altitudeM: 1234,
        varioMs: 1.5,
        temperatureC: 24.2,
        batteryMv: 4120,
        timestamp: timestamp,
      );
      final Lk8ex1Data same = original.copyWith();
      final Lk8ex1Data updated = original.copyWith(varioMs: -0.8, batteryMv: null);

      expect(same, original);
      expect(same.hashCode, original.hashCode);
      expect(updated.varioMs, -0.8);
      expect(updated.batteryMv, isNull);
      expect(updated.toString(), contains('pressurePa: 101325'));
    });
  });
}
