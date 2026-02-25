import 'package:flutter_test/flutter_test.dart';
import 'package:fly_in_peace/core/ble/ble_service.dart';
import 'package:fly_in_peace/core/utils/lk8ex1_frame_verdict.dart';
import 'package:fly_in_peace/core/utils/lk8ex1_parser.dart';

void main() {
  const Lk8ex1Parser parser = Lk8ex1Parser();
  const Lk8ex1FrameVerdictEngine verdictEngine = Lk8ex1FrameVerdictEngine();

  group('Phase 3.5 Matrix A1-A8', () {
    test('A1 nominal -> valid', () {
      final List<String> frames = List<String>.generate(
        5,
        (index) => _buildSentence(
          pressurePa: 100900 - index,
          altitudeM: 1035 + (index % 2),
          varioCms: 5,
          temperatureDc: 235,
          battery: 95,
        ),
      );

      for (final String frame in frames) {
        final Lk8ex1ParseResult parseResult = parser.parseLine(frame);
        final Lk8ex1FrameVerdictResult verdict = verdictEngine.evaluate(
          parseResult,
        );

        expect(parseResult.data, isNotNull);
        expect(verdict.verdict, Lk8ex1FrameVerdict.valid);
      }
    });

    test('A2 climb -> valid', () {
      final List<String> frames = List<String>.generate(
        5,
        (index) => _buildSentence(
          pressurePa: 100800 - (index * 5),
          altitudeM: 950 + (index * 3),
          varioCms: 180,
          temperatureDc: 228,
          battery: 94,
        ),
      );

      for (final String frame in frames) {
        final Lk8ex1FrameVerdictResult verdict = verdictEngine.evaluate(
          parser.parseLine(frame),
        );
        expect(verdict.verdict, Lk8ex1FrameVerdict.valid);
      }
    });

    test('A3 sink -> valid', () {
      final List<String> frames = List<String>.generate(
        5,
        (index) => _buildSentence(
          pressurePa: 100700 + (index * 6),
          altitudeM: 1200 - (index * 4),
          varioCms: -180,
          temperatureDc: 224,
          battery: 93,
        ),
      );

      for (final String frame in frames) {
        final Lk8ex1FrameVerdictResult verdict = verdictEngine.evaluate(
          parser.parseLine(frame),
        );
        expect(verdict.verdict, Lk8ex1FrameVerdict.valid);
      }
    });

    test('A4 edge-placeholder-alt -> warning', () {
      final List<String> frames = List<String>.generate(
        5,
        (index) => _buildSentence(
          pressurePa: 100840 + index,
          altitudeM: 99999,
          varioCms: 0,
          temperatureDc: 230,
          battery: 92,
        ),
      );

      for (final String frame in frames) {
        final Lk8ex1FrameVerdictResult verdict = verdictEngine.evaluate(
          parser.parseLine(frame),
        );
        expect(verdict.verdict, Lk8ex1FrameVerdict.warning);
        expect(verdict.reason, contains('Placeholder altitude'));
      }
    });

    test('A5 edge-placeholder-bat -> warning', () {
      final List<String> frames = List<String>.generate(
        5,
        (index) => _buildSentence(
          pressurePa: 100860 + index,
          altitudeM: 1010,
          varioCms: -5,
          temperatureDc: 232,
          battery: 999,
        ),
      );

      for (final String frame in frames) {
        final Lk8ex1FrameVerdictResult verdict = verdictEngine.evaluate(
          parser.parseLine(frame),
        );
        expect(verdict.verdict, Lk8ex1FrameVerdict.warning);
        expect(verdict.reason, contains('Placeholder battery'));
      }
    });

    test('A6 malformed-checksum -> error', () {
      final String validFrame = _buildSentence(
        pressurePa: 100900,
        altitudeM: 1035,
        varioCms: 5,
        temperatureDc: 235,
        battery: 95,
      );
      final int separatorIndex = validFrame.indexOf('*');
      expect(separatorIndex, isNonNegative);
      final String validChecksum = validFrame.substring(separatorIndex + 1);
      final String corruptedChecksum = validChecksum == '00' ? '01' : '00';
      final String malformed =
          validFrame.substring(0, separatorIndex + 1) + corruptedChecksum;

      final Lk8ex1FrameVerdictResult verdict = verdictEngine.evaluate(
        parser.parseLine(malformed),
      );
      expect(verdict.verdict, Lk8ex1FrameVerdict.error);
      expect(verdict.reason, contains('Checksum mismatch'));
    });

    test('A7 malformed-shape -> error', () {
      final String malformedShape = _buildRawSentenceWithChecksum(
        'LK8EX1,101325,1000,120',
      );

      final Lk8ex1FrameVerdictResult verdict = verdictEngine.evaluate(
        parser.parseLine(malformedShape),
      );
      expect(verdict.verdict, Lk8ex1FrameVerdict.error);
      expect(verdict.reason, contains('Malformed field count'));
    });

    test('A8 interoperability-bluefly -> valid/warning + profile detected', () {
      final BleCompatibilityProfile profile =
          BleService.detectCompatibilityProfile('BlueFlyVario v23');

      final List<String> frames = List<String>.generate(
        5,
        (index) => _buildSentence(
          pressurePa: 100850 - index,
          altitudeM: 1080 + index,
          varioCms: 90,
          temperatureDc: 231,
          battery: 90,
        ),
      );

      expect(profile, BleCompatibilityProfile.blueFlyVario);

      for (final String frame in frames) {
        final Lk8ex1FrameVerdict verdict = verdictEngine
            .evaluate(parser.parseLine(frame))
            .verdict;
        expect(
          verdict == Lk8ex1FrameVerdict.valid ||
              verdict == Lk8ex1FrameVerdict.warning,
          isTrue,
        );
      }
    });
  });
}

String _buildSentence({
  required int pressurePa,
  required int altitudeM,
  required int varioCms,
  required int temperatureDc,
  required int battery,
}) {
  final String payload =
      'LK8EX1,$pressurePa,$altitudeM,$varioCms,$temperatureDc,$battery,';
  return _buildRawSentenceWithChecksum(payload);
}

String _buildRawSentenceWithChecksum(String payload) {
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
