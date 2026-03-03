import 'package:flutter/foundation.dart';

import '../models/lk8ex1_data.dart';

class Lk8ex1ParseResult {
  const Lk8ex1ParseResult({
    required this.rawLine,
    required this.hasChecksum,
    required this.checksumValid,
    required this.data,
    required this.errorReason,
  });

  final String rawLine;
  final bool hasChecksum;
  final bool checksumValid;
  final Lk8ex1Data? data;
  final String? errorReason;

  bool get isParsed => data != null;
}

class Lk8ex1Parser {
  const Lk8ex1Parser({this.allowSentencesWithoutChecksum = false});

  static const String _prefix = 'LK8EX1,';
  final bool allowSentencesWithoutChecksum;

  Lk8ex1Data? parseLk8ex1(String sentence) {
    final Lk8ex1ParseResult result = parseLine(sentence);
    return result.data;
  }

  bool validateChecksum(String sentence) {
    final _ChecksumEvaluation checksum = _evaluateChecksum(sentence.trim());
    return checksum.valid;
  }

  Lk8ex1ParseResult parseLine(String line) {
    final String trimmedLine = line.trim();
    if (trimmedLine.isEmpty) {
      return const Lk8ex1ParseResult(
        rawLine: '',
        hasChecksum: false,
        checksumValid: false,
        data: null,
        errorReason: 'Empty frame',
      );
    }

    final _ChecksumEvaluation checksum = _evaluateChecksum(trimmedLine);
    if (!checksum.hasChecksum && !allowSentencesWithoutChecksum) {
      return Lk8ex1ParseResult(
        rawLine: trimmedLine,
        hasChecksum: false,
        checksumValid: false,
        data: null,
        errorReason: checksum.errorReason ?? 'Missing checksum separator (*)',
      );
    }
    if (!checksum.valid) {
      debugPrint('LK8EX1 checksum warning: ${checksum.errorReason}');
      return Lk8ex1ParseResult(
        rawLine: trimmedLine,
        hasChecksum: checksum.hasChecksum,
        checksumValid: false,
        data: null,
        errorReason: checksum.errorReason,
      );
    }

    final String payload = checksum.payload;
    if (!payload.startsWith(_prefix)) {
      return Lk8ex1ParseResult(
        rawLine: trimmedLine,
        hasChecksum: checksum.hasChecksum,
        checksumValid: checksum.valid,
        data: null,
        errorReason: 'Unsupported sentence prefix',
      );
    }

    final String valuesBody = payload.substring(_prefix.length);
    final List<String> rawParts = valuesBody.split(',');
    final List<String> parts = List<String>.from(rawParts);
    if (parts.isNotEmpty && parts.last.isEmpty) {
      parts.removeLast();
    }

    if (parts.length != 5) {
      return Lk8ex1ParseResult(
        rawLine: trimmedLine,
        hasChecksum: checksum.hasChecksum,
        checksumValid: checksum.valid,
        data: null,
        errorReason: 'Malformed field count (${parts.length})',
      );
    }

    final int? pressurePa = int.tryParse(parts[0]);
    final int? altitudeM = int.tryParse(parts[1]);
    final int? varioCms = int.tryParse(parts[2]);
    final int? temperatureDc = _parseTemperatureDeci(parts[3]);
    final int? battery = int.tryParse(parts[4]);

    if (pressurePa == null ||
        altitudeM == null ||
        varioCms == null ||
        temperatureDc == null ||
        battery == null) {
      return Lk8ex1ParseResult(
        rawLine: trimmedLine,
        hasChecksum: checksum.hasChecksum,
        checksumValid: checksum.valid,
        data: null,
        errorReason: 'One or more fields are not valid numbers',
      );
    }

    final Lk8ex1Data data = Lk8ex1Data(
      pressurePa: pressurePa,
      altitudeM: _normalizeAltitude(altitudeM),
      varioMs: varioCms / 100.0,
      temperatureC: temperatureDc / 10.0,
      batteryMv: _normalizeBattery(battery),
      timestamp: DateTime.now().toUtc(),
    );

    return Lk8ex1ParseResult(
      rawLine: trimmedLine,
      hasChecksum: checksum.hasChecksum,
      checksumValid: checksum.valid,
      data: data,
      errorReason: null,
    );
  }

  _ChecksumEvaluation _evaluateChecksum(String trimmedLine) {
    final String content = trimmedLine.startsWith(r'$')
        ? trimmedLine.substring(1)
        : trimmedLine;
    final int checksumSeparatorIndex = content.indexOf('*');
    if (checksumSeparatorIndex <= 0 ||
        checksumSeparatorIndex >= content.length - 1) {
      if (allowSentencesWithoutChecksum) {
        return _ChecksumEvaluation.validWithoutChecksum(payload: content);
      }
      return _ChecksumEvaluation.invalid(
        payload: content,
        hasChecksum: false,
        errorReason: 'Missing checksum separator (*)',
      );
    }

    final String payload = content.substring(0, checksumSeparatorIndex);
    final String checksumText = content
        .substring(checksumSeparatorIndex + 1)
        .trim();
    if (checksumText.length < 2) {
      return _ChecksumEvaluation.invalid(
        payload: payload,
        hasChecksum: true,
        errorReason: 'Checksum too short',
      );
    }

    final String normalizedChecksum = checksumText
        .substring(0, 2)
        .toUpperCase();
    final int? checksumValue = int.tryParse(normalizedChecksum, radix: 16);
    if (checksumValue == null) {
      return _ChecksumEvaluation.invalid(
        payload: payload,
        hasChecksum: true,
        errorReason: 'Checksum is not hexadecimal',
      );
    }

    final int calculatedChecksum = _calculateChecksum(payload);
    if (checksumValue != calculatedChecksum) {
      return _ChecksumEvaluation.invalid(
        payload: payload,
        hasChecksum: true,
        errorReason:
            'Checksum mismatch (expected $normalizedChecksum, calculated ${calculatedChecksum.toRadixString(16).toUpperCase().padLeft(2, '0')})',
      );
    }

    return _ChecksumEvaluation.valid(payload: payload);
  }

  int _calculateChecksum(String payload) {
    int checksum = 0;
    for (final int codeUnit in payload.codeUnits) {
      checksum ^= codeUnit;
    }

    return checksum;
  }

  int? _parseTemperatureDeci(String value) {
    final int? integerTemperature = int.tryParse(value);
    if (integerTemperature != null) {
      return integerTemperature;
    }

    final double? decimalTemperature = double.tryParse(value);
    if (decimalTemperature == null) {
      return null;
    }

    return (decimalTemperature * 10).round();
  }

  double _normalizeAltitude(int value) {
    if (value == 99999) {
      return double.nan;
    }
    return value.toDouble();
  }

  int? _normalizeBattery(int value) {
    if (value == 999) {
      return null;
    }
    return value;
  }
}

class _ChecksumEvaluation {
  const _ChecksumEvaluation({
    required this.payload,
    required this.hasChecksum,
    required this.valid,
    required this.errorReason,
  });

  factory _ChecksumEvaluation.valid({required String payload}) {
    return _ChecksumEvaluation(
      payload: payload,
      hasChecksum: true,
      valid: true,
      errorReason: null,
    );
  }

  factory _ChecksumEvaluation.validWithoutChecksum({required String payload}) {
    return _ChecksumEvaluation(
      payload: payload,
      hasChecksum: false,
      valid: true,
      errorReason: null,
    );
  }

  factory _ChecksumEvaluation.invalid({
    required String payload,
    required bool hasChecksum,
    required String errorReason,
  }) {
    return _ChecksumEvaluation(
      payload: payload,
      hasChecksum: hasChecksum,
      valid: false,
      errorReason: errorReason,
    );
  }

  final String payload;
  final bool hasChecksum;
  final bool valid;
  final String? errorReason;
}

Lk8ex1Data? parseLk8ex1(
  String sentence, {
  bool allowSentencesWithoutChecksum = false,
}) {
  return Lk8ex1Parser(
    allowSentencesWithoutChecksum: allowSentencesWithoutChecksum,
  ).parseLk8ex1(sentence);
}

bool validateChecksum(String sentence) {
  return const Lk8ex1Parser().validateChecksum(sentence);
}
