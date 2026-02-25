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
  const Lk8ex1Parser();

  static const String _prefix = 'LK8EX1,';

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

    final String content = trimmedLine.startsWith(r'$')
        ? trimmedLine.substring(1)
        : trimmedLine;
    final int checksumSeparatorIndex = content.indexOf('*');
    if (checksumSeparatorIndex <= 0 ||
        checksumSeparatorIndex >= content.length - 1) {
      return Lk8ex1ParseResult(
        rawLine: trimmedLine,
        hasChecksum: false,
        checksumValid: false,
        data: null,
        errorReason: 'Missing checksum separator (*)',
      );
    }

    final String payload = content.substring(0, checksumSeparatorIndex);
    final String checksumText = content
        .substring(checksumSeparatorIndex + 1)
        .trim();
    if (checksumText.length < 2) {
      return Lk8ex1ParseResult(
        rawLine: trimmedLine,
        hasChecksum: true,
        checksumValid: false,
        data: null,
        errorReason: 'Checksum too short',
      );
    }

    final String normalizedChecksum = checksumText
        .substring(0, 2)
        .toUpperCase();
    final int? checksumValue = int.tryParse(normalizedChecksum, radix: 16);
    if (checksumValue == null) {
      return Lk8ex1ParseResult(
        rawLine: trimmedLine,
        hasChecksum: true,
        checksumValid: false,
        data: null,
        errorReason: 'Checksum is not hexadecimal',
      );
    }

    final int calculatedChecksum = _calculateChecksum(payload);
    final bool checksumValid = checksumValue == calculatedChecksum;
    if (!checksumValid) {
      return Lk8ex1ParseResult(
        rawLine: trimmedLine,
        hasChecksum: true,
        checksumValid: false,
        data: null,
        errorReason:
            'Checksum mismatch (expected $normalizedChecksum, calculated ${calculatedChecksum.toRadixString(16).toUpperCase().padLeft(2, '0')})',
      );
    }

    if (!payload.startsWith(_prefix)) {
      return Lk8ex1ParseResult(
        rawLine: trimmedLine,
        hasChecksum: true,
        checksumValid: true,
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
        hasChecksum: true,
        checksumValid: true,
        data: null,
        errorReason: 'Malformed field count (${parts.length})',
      );
    }

    final int? pressurePa = int.tryParse(parts[0]);
    final int? altitudeM = int.tryParse(parts[1]);
    final int? varioCms = int.tryParse(parts[2]);
    final int? temperatureDc = int.tryParse(parts[3]);
    final int? battery = int.tryParse(parts[4]);

    if (pressurePa == null ||
        altitudeM == null ||
        varioCms == null ||
        temperatureDc == null ||
        battery == null) {
      return Lk8ex1ParseResult(
        rawLine: trimmedLine,
        hasChecksum: true,
        checksumValid: true,
        data: null,
        errorReason: 'One or more fields are not integers',
      );
    }

    final Lk8ex1Data data = Lk8ex1Data(
      pressurePa: pressurePa,
      altitudeM: altitudeM,
      varioCms: varioCms,
      temperatureDc: temperatureDc,
      battery: battery,
    );

    return Lk8ex1ParseResult(
      rawLine: trimmedLine,
      hasChecksum: true,
      checksumValid: true,
      data: data,
      errorReason: null,
    );
  }

  int _calculateChecksum(String payload) {
    int checksum = 0;
    for (final int codeUnit in payload.codeUnits) {
      checksum ^= codeUnit;
    }

    return checksum;
  }
}
