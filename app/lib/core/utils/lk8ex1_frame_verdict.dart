import '../models/lk8ex1_data.dart';
import 'lk8ex1_parser.dart';

enum Lk8ex1FrameVerdict { valid, warning, error }

class Lk8ex1FrameVerdictResult {
  const Lk8ex1FrameVerdictResult({required this.verdict, required this.reason});

  final Lk8ex1FrameVerdict verdict;
  final String reason;
}

class Lk8ex1FrameVerdictEngine {
  const Lk8ex1FrameVerdictEngine();

  Lk8ex1FrameVerdictResult evaluate(Lk8ex1ParseResult parseResult) {
    if (!parseResult.hasChecksum) {
      return const Lk8ex1FrameVerdictResult(
        verdict: Lk8ex1FrameVerdict.error,
        reason: 'Missing checksum',
      );
    }

    if (!parseResult.checksumValid) {
      return Lk8ex1FrameVerdictResult(
        verdict: Lk8ex1FrameVerdict.error,
        reason: parseResult.errorReason ?? 'Checksum mismatch',
      );
    }

    final Lk8ex1Data? data = parseResult.data;
    if (data == null) {
      return Lk8ex1FrameVerdictResult(
        verdict: Lk8ex1FrameVerdict.error,
        reason: parseResult.errorReason ?? 'Malformed LK8EX1 sentence',
      );
    }

    final String? impossibleReason = _impossibleCombinationReason(data);
    if (impossibleReason != null) {
      return Lk8ex1FrameVerdictResult(
        verdict: Lk8ex1FrameVerdict.error,
        reason: impossibleReason,
      );
    }

    final List<String> warningReasons = _warningReasons(data);
    if (warningReasons.isNotEmpty) {
      return Lk8ex1FrameVerdictResult(
        verdict: Lk8ex1FrameVerdict.warning,
        reason: warningReasons.join('; '),
      );
    }

    return const Lk8ex1FrameVerdictResult(
      verdict: Lk8ex1FrameVerdict.valid,
      reason: 'Checksum and fields are valid',
    );
  }

  String? _impossibleCombinationReason(Lk8ex1Data data) {
    if (data.pressurePa <= 0) {
      return 'Impossible pressure value';
    }

    if (!data.hasPlaceholderAltitude &&
        (data.altitudeM < -1000 || data.altitudeM > 15000)) {
      return 'Impossible altitude value';
    }

    if (data.varioCms < -5000 || data.varioCms > 5000) {
      return 'Impossible vario value';
    }

    if (data.temperatureDc < -600 || data.temperatureDc > 1200) {
      return 'Impossible temperature value';
    }

    if (data.battery != 999 && (data.battery < 0 || data.battery > 100)) {
      return 'Impossible battery value';
    }

    return null;
  }

  List<String> _warningReasons(Lk8ex1Data data) {
    final List<String> reasons = <String>[];

    if (data.hasPlaceholderAltitude) {
      reasons.add('Placeholder altitude (99999)');
    }

    if (data.hasPlaceholderBattery) {
      reasons.add('Placeholder battery (999)');
    }

    if (data.pressurePa < 80000 || data.pressurePa > 110000) {
      reasons.add('Borderline pressure range');
    }

    if (!data.hasPlaceholderAltitude &&
        (data.altitudeM < -500 || data.altitudeM > 9000)) {
      reasons.add('Borderline altitude range');
    }

    if (data.varioCms.abs() > 1500) {
      reasons.add('Borderline vario range');
    }

    if (data.temperatureDc < -300 || data.temperatureDc > 600) {
      reasons.add('Borderline temperature range');
    }

    return reasons;
  }
}
