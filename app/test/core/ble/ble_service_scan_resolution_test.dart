import 'package:flutter_test/flutter_test.dart';

import 'package:fly_in_peace/core/ble/ble_service.dart';

void main() {
  group('BleService scan name resolution', () {
    test('detects FlyInPeace profile from advertisement name', () {
      final BleCompatibilityProfile profile =
          BleService.detectCompatibilityProfileFromScanNames(
            platformName: 'ESP32-C3',
            advertisementName: 'FlyInPeace',
          );

      expect(profile, BleCompatibilityProfile.flyInPeace);
    });

    test('prefers advertisement name when it contains compatible branding', () {
      final String name = BleService.resolveScanDisplayName(
        platformName: 'ESP32-C3',
        advertisementName: 'FlyInPeace',
      );

      expect(name, 'FlyInPeace');
    });

    test('keeps platform name when advertisement name is empty', () {
      final String name = BleService.resolveScanDisplayName(
        platformName: 'MyDevice',
        advertisementName: '',
      );

      expect(name, 'MyDevice');
    });

    test('detects FlyInPeace profile from hyphenated name variants', () {
      final BleCompatibilityProfile profile = BleService.detectCompatibilityProfile(
        'ESP32-C3 Fly-In-Peace',
      );

      expect(profile, BleCompatibilityProfile.flyInPeace);
    });

    test('detects FlyInPeace profile from underscored name variants', () {
      final BleCompatibilityProfile profile = BleService.detectCompatibilityProfile(
        'ESP32_C3 fly_in_peace',
      );

      expect(profile, BleCompatibilityProfile.flyInPeace);
    });

    test('detects FlyInPeace profile from short advertising name', () {
      final BleCompatibilityProfile profile = BleService.detectCompatibilityProfile(
        'FlyInP',
      );

      expect(profile, BleCompatibilityProfile.flyInPeace);
    });
  });
}
