# fly_in_peace

Flutter companion app for `esp-fly-in-peace`.

## Run from repository root

Use project scripts from `.`:

- `./scripts/app/app_test_option.sh list`
- `./scripts/app/app_test_option.sh 1 linux`
- `./scripts/app/app_test_option.sh 1 chrome`
- `./scripts/app/app_test_option.sh 3`
- `./scripts/app/build_app_debug.sh`

## Flutter auto-detection in scripts

App scripts automatically try to locate `flutter` when it is not available in the current terminal `PATH`.

Checked locations:

- `/snap/bin`
- `/var/lib/snapd/snap/bin`
- `~/flutter/bin`
- `~/development/flutter/bin`
- `~/sdk/flutter/bin`

Shared helper used by scripts: `scripts/app/flutter_env.sh`.

If Flutter is still not found, add Flutter to your shell `PATH` or install Flutter and retry.

## Flutter references

- [Learn Flutter](https://docs.flutter.dev/get-started/learn-flutter)
- [Write your first Flutter app](https://docs.flutter.dev/get-started/codelab)
- [Flutter learning resources](https://docs.flutter.dev/reference/learning-resources)
