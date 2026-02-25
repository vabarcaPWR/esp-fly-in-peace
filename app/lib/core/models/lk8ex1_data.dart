class Lk8ex1Data {
  const Lk8ex1Data({
    required this.pressurePa,
    required this.altitudeM,
    required this.varioCms,
    required this.temperatureDc,
    required this.battery,
  });

  final int pressurePa;
  final int altitudeM;
  final int varioCms;
  final int temperatureDc;
  final int battery;

  bool get hasPlaceholderAltitude => altitudeM == 99999;
  bool get hasPlaceholderBattery => battery == 999;
}
