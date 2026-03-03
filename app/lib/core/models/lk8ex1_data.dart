class Lk8ex1Data {
  const Lk8ex1Data({
    required this.pressurePa,
    required this.altitudeM,
    required this.varioMs,
    required this.temperatureC,
    required this.batteryMv,
    required this.timestamp,
  });

  final int pressurePa;
  final double altitudeM;
  final double varioMs;
  final double temperatureC;
  final int? batteryMv;
  final DateTime timestamp;

  int get varioCms => (varioMs * 100).round();
  int get temperatureDc => (temperatureC * 10).round();
  int get battery => batteryMv ?? 999;

  bool get hasPlaceholderAltitude => altitudeM.isNaN;
  bool get hasPlaceholderBattery => batteryMv == null;

  Lk8ex1Data copyWith({
    int? pressurePa,
    double? altitudeM,
    double? varioMs,
    double? temperatureC,
    Object? batteryMv = _keepCurrentValue,
    DateTime? timestamp,
  }) {
    return Lk8ex1Data(
      pressurePa: pressurePa ?? this.pressurePa,
      altitudeM: altitudeM ?? this.altitudeM,
      varioMs: varioMs ?? this.varioMs,
      temperatureC: temperatureC ?? this.temperatureC,
      batteryMv: identical(batteryMv, _keepCurrentValue)
          ? this.batteryMv
          : batteryMv as int?,
      timestamp: timestamp ?? this.timestamp,
    );
  }

  @override
  String toString() {
    return 'Lk8ex1Data(pressurePa: $pressurePa, altitudeM: $altitudeM, varioMs: $varioMs, temperatureC: $temperatureC, batteryMv: $batteryMv, timestamp: $timestamp)';
  }

  @override
  bool operator ==(Object other) {
    if (identical(this, other)) {
      return true;
    }
    if (other is! Lk8ex1Data) {
      return false;
    }
    return pressurePa == other.pressurePa &&
        _doubleEquals(altitudeM, other.altitudeM) &&
        _doubleEquals(varioMs, other.varioMs) &&
        _doubleEquals(temperatureC, other.temperatureC) &&
        batteryMv == other.batteryMv &&
        timestamp == other.timestamp;
  }

  @override
  int get hashCode {
    return Object.hash(
      pressurePa,
      _doubleHash(altitudeM),
      _doubleHash(varioMs),
      _doubleHash(temperatureC),
      batteryMv,
      timestamp,
    );
  }

  static bool _doubleEquals(double left, double right) {
    if (left.isNaN && right.isNaN) {
      return true;
    }
    return left == right;
  }

  static int _doubleHash(double value) {
    if (value.isNaN) {
      return double.nan.hashCode;
    }
    return value.hashCode;
  }
}

const Object _keepCurrentValue = Object();
