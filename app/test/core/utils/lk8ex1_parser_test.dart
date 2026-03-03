import 'package:flutter_test/flutter_test.dart';
import 'package:fly_in_peace/core/utils/lk8ex1_parser.dart';

void main() {
  group('LK8EX1 parser phase 4', () {
    test('parse valid sentence with all fields', () {
      const Lk8ex1Parser parser = Lk8ex1Parser();
      final data = parser.parseLk8ex1(
        _buildSentence('LK8EX1,101325,1234,250,236,4200,'),
      );

      expect(data, isNotNull);
      expect(data!.pressurePa, 101325);
      expect(data.altitudeM, 1234);
      expect(data.varioMs, 2.5);
      expect(data.temperatureC, 23.6);
      expect(data.batteryMv, 4200);
    });

    test('parses altitude placeholder 99999 as NaN', () {
      const Lk8ex1Parser parser = Lk8ex1Parser();
      final data = parser.parseLk8ex1(
        _buildSentence('LK8EX1,101325,99999,10,200,4200,'),
      );

      expect(data, isNotNull);
      expect(data!.altitudeM.isNaN, isTrue);
    });

    test('parses battery placeholder 999 as null', () {
      const Lk8ex1Parser parser = Lk8ex1Parser();
      final data = parser.parseLk8ex1(
        _buildSentence('LK8EX1,101325,1000,10,200,999,'),
      );

      expect(data, isNotNull);
      expect(data!.batteryMv, isNull);
    });

    test('checksum validation passes for valid sentence', () {
      const Lk8ex1Parser parser = Lk8ex1Parser();
      final bool isValid = parser.validateChecksum(
        _buildSentence('LK8EX1,101325,1000,10,200,4200,'),
      );

      expect(isValid, isTrue);
    });

    test('checksum validation fails for corrupted sentence', () {
      const Lk8ex1Parser parser = Lk8ex1Parser();
      final bool isValid = parser.validateChecksum(
        r'$LK8EX1,101325,1000,10,200,4200,*00',
      );

      expect(isValid, isFalse);
    });

    test('returns null for empty string', () {
      const Lk8ex1Parser parser = Lk8ex1Parser();
      expect(parser.parseLk8ex1('   '), isNull);
    });

    test('returns null for non-LK8EX1 sentence', () {
      const Lk8ex1Parser parser = Lk8ex1Parser();
      expect(parser.parseLk8ex1(r'$GPGGA,1,2,3*47'), isNull);
    });
  });
}

String _buildSentence(String payload) {
  int checksum = 0;
  for (final int byte in payload.codeUnits) {
    checksum ^= byte;
  }
  final String checksumText = checksum
      .toRadixString(16)
      .toUpperCase()
      .padLeft(2, '0');
  return '\$$payload*$checksumText';
}
