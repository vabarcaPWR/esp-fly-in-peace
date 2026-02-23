class DeviceConfig {
  const DeviceConfig({
    required this.bleName,
    required this.sampleRateHz,
    required this.unitsMetric,
  });

  final String bleName;
  final int sampleRateHz;
  final bool unitsMetric;
}
