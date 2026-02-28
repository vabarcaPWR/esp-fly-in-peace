import 'package:flutter_test/flutter_test.dart';
import 'package:fly_in_peace/core/ble/ble_service.dart';
import 'package:fly_in_peace/features/scanner/scanner_provider.dart';

void main() {
  group('Scanner auto-connect selection', () {
    test('selects FlyInPeace candidate that was not attempted yet', () {
      final List<ScannerAutoConnectCandidate> candidates =
          <ScannerAutoConnectCandidate>[
            const ScannerAutoConnectCandidate(
              remoteId: 'bluefly-1',
              hasFlyInPeaceName: false,
              profile: BleCompatibilityProfile.blueFlyVario,
            ),
            const ScannerAutoConnectCandidate(
              remoteId: 'fip-1',
              hasFlyInPeaceName: true,
              profile: BleCompatibilityProfile.flyInPeace,
            ),
          ];

      final String? picked = ScannerController.pickAutoConnectCandidateId(
        candidates: candidates,
        attemptedDeviceIds: <String>{},
      );

      expect(picked, 'fip-1');
    });

    test('skips already attempted candidate and picks next eligible one', () {
      final List<ScannerAutoConnectCandidate> candidates =
          <ScannerAutoConnectCandidate>[
            const ScannerAutoConnectCandidate(
              remoteId: 'fip-1',
              hasFlyInPeaceName: true,
              profile: BleCompatibilityProfile.flyInPeace,
            ),
            const ScannerAutoConnectCandidate(
              remoteId: 'fip-2',
              hasFlyInPeaceName: false,
              profile: BleCompatibilityProfile.flyInPeace,
            ),
          ];

      final String? picked = ScannerController.pickAutoConnectCandidateId(
        candidates: candidates,
        attemptedDeviceIds: <String>{'fip-1'},
      );

      expect(picked, 'fip-2');
    });

    test('returns null when no FlyInPeace candidate is available', () {
      final List<ScannerAutoConnectCandidate> candidates =
          <ScannerAutoConnectCandidate>[
            const ScannerAutoConnectCandidate(
              remoteId: 'dev-1',
              hasFlyInPeaceName: false,
              profile: BleCompatibilityProfile.blueFlyVario,
            ),
            const ScannerAutoConnectCandidate(
              remoteId: 'dev-2',
              hasFlyInPeaceName: false,
              profile: BleCompatibilityProfile.unsupported,
            ),
          ];

      final String? picked = ScannerController.pickAutoConnectCandidateId(
        candidates: candidates,
        attemptedDeviceIds: <String>{},
      );

      expect(picked, isNull);
    });
  });
}
