#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(dirname "$(dirname "$SCRIPT_DIR")")"
APP_DIR="${ROOT_DIR}/app"
APK_PATH="${APP_DIR}/build/app/outputs/flutter-apk/app-debug.apk"
FLUTTER_ENV_HELPER="${ROOT_DIR}/scripts/app/flutter_env.sh"

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

if [[ ! -f "${FLUTTER_ENV_HELPER}" ]]; then
    echo -e "${RED}[APP]${NC} Missing helper script: ${FLUTTER_ENV_HELPER}" >&2
    exit 1
fi

# shellcheck source=/dev/null
source "${FLUTTER_ENV_HELPER}"

usage()
{
    echo "Usage: ./scripts/app/app_test_option.sh <1|2|3|list> [device-id] [extra args...]"
    echo ""
    echo "  1  Run app in development mode on selected Flutter device"
    echo "  2  Install debug APK with adb"
    echo "  3  Run static + widget checks (flutter analyze && flutter test)"
    echo "  list  Show supported devices for option 1"
    echo ""
    echo "Examples:"
    echo "  ./scripts/app/app_test_option.sh 1 chrome"
    echo "  ./scripts/app/app_test_option.sh 1 linux"
    echo "  ./scripts/app/app_test_option.sh 1 chrome --no-resident"
    echo "  ./scripts/app/app_test_option.sh 2"
    echo "  ./scripts/app/app_test_option.sh 3"
    echo "  ./scripts/app/app_test_option.sh list"
    echo ""
    echo "Tip: list devices with: cd app && flutter devices"
}

require_command()
{
    local cmd="$1"
    if ! command -v "$cmd" >/dev/null 2>&1; then
        echo -e "${RED}[APP]${NC} Required command not found: ${cmd}" >&2
        exit 1
    fi
}

require_python3()
{
    require_command python3
}

ensure_repo_layout()
{
    if [[ ! -d "$APP_DIR" || ! -f "${APP_DIR}/pubspec.yaml" ]]; then
        echo -e "${RED}[APP]${NC} Could not locate app directory at ${APP_DIR}" >&2
        exit 1
    fi
}

first_connected_adb_device()
{
    adb devices | awk 'NR>1 && $2=="device" {print $1; exit}'
}

flutter_device_exists()
{
    local device_id="$1"
    flutter devices --machine 2>/dev/null | python3 -c '
import json
import sys

target = sys.argv[1]
try:
    devices = json.load(sys.stdin)
except Exception:
    sys.exit(1)

sys.exit(0 if any(device.get("id") == target for device in devices) else 1)
' "$device_id"
}

is_supported_option1_device()
{
    local device_id="$1"
    flutter devices --machine 2>/dev/null | python3 -c '
import json
import sys

target = sys.argv[1]
try:
    devices = json.load(sys.stdin)
except Exception:
    sys.exit(1)

for device in devices:
    if device.get("id") != target:
        continue

    target_platform = str(device.get("targetPlatform", "")).lower()
    if (
        target == "chrome"
        or target == "linux"
        or target_platform.startswith("android")
        or target_platform.startswith("linux")
    ):
        sys.exit(0)

    sys.exit(1)

sys.exit(1)
' "$device_id"
}

normalize_option1_device_id()
{
    local device_id="$1"
    local lowered
    lowered="$(echo "$device_id" | tr '[:upper:]' '[:lower:]')"

    case "$lowered" in
        chrome)
            echo "chrome"
            ;;
        linux)
            echo "linux"
            ;;
        *)
            echo "$device_id"
            ;;
    esac
}

list_supported_option1_devices()
{
    ensure_flutter_available "APP"
    require_python3
    ensure_repo_layout

    cd "$APP_DIR"

    echo -e "${GREEN}[APP]${NC} Supported targets for option 1:"
    flutter devices --machine 2>/dev/null | python3 -c '
import json
import sys

try:
    devices = json.load(sys.stdin)
except Exception:
    print("  (no devices detected)")
    sys.exit(0)

count = 0
for device in devices:
    dev_id = str(device.get("id", ""))
    name = str(device.get("name", ""))
    platform = str(device.get("targetPlatform", "")).lower()
    if dev_id in {"chrome", "linux"} or platform.startswith("android") or platform.startswith("linux"):
        print(f"  - {name} ({dev_id})")
        count += 1

if count == 0:
    print("  (no supported targets found; use chrome, linux, or android)")
'
}

run_option_1()
{
    ensure_flutter_available "APP"
    require_python3
    ensure_repo_layout

    local device_id="${1:-}"
    shift || true
    local extra_args=("$@")

    echo -e "${GREEN}[APP]${NC} Option 1 selected: flutter run"
    cd "$APP_DIR"

    if [[ -z "$device_id" ]]; then
        echo -e "${RED}[APP]${NC} Option 1 requires an explicit device-id." >&2
        echo "  Example: ./scripts/app/app_test_option.sh 1 chrome" >&2
        echo "  Devices available:" >&2
        flutter devices >&2 || true
        exit 1
    fi

    device_id="$(normalize_option1_device_id "$device_id")"

    if ! flutter_device_exists "$device_id"; then
        echo -e "${RED}[APP]${NC} Flutter device not found: ${device_id}" >&2
        echo "  Devices available:" >&2
        flutter devices >&2 || true
        exit 1
    fi

    if ! is_supported_option1_device "$device_id"; then
        echo -e "${RED}[APP]${NC} Unsupported target for option 1: ${device_id}" >&2
        echo "  Supported targets for option 1 are: Chrome, Linux desktop, and Android devices/emulators." >&2
        echo "  Devices available:" >&2
        flutter devices >&2 || true
        exit 1
    fi

    echo -e "${YELLOW}[APP]${NC} Using target: ${device_id}"
    flutter run -d "$device_id" "${extra_args[@]}"
}

run_option_2()
{
    ensure_flutter_available "APP"
    require_command adb
    ensure_repo_layout

    local device_id="${1:-}"

    if [[ ! -f "$APK_PATH" ]]; then
        echo -e "${YELLOW}[APP]${NC} APK not found. Building debug APK first..."
        "${SCRIPT_DIR}/build_app_debug.sh"
    fi

    if [[ -z "$device_id" ]]; then
        device_id="$(first_connected_adb_device)"
    fi

    if [[ -z "$device_id" ]]; then
        echo -e "${RED}[APP]${NC} No connected Android device found in adb." >&2
        echo "  Connect a device and run: adb devices" >&2
        exit 1
    fi

    echo -e "${GREEN}[APP]${NC} Option 2 selected: install APK"
    echo -e "${YELLOW}[APP]${NC} Device: ${device_id}"
    adb -s "$device_id" install -r "$APK_PATH"

    echo -e "${GREEN}[APP]${NC} APK installed successfully: ${APK_PATH}"
}

run_option_3()
{
    ensure_flutter_available "APP"
    ensure_repo_layout

    echo -e "${GREEN}[APP]${NC} Option 3 selected: analyze + test"
    cd "$APP_DIR"

    flutter analyze
    flutter test

    echo -e "${GREEN}[APP]${NC} Static analysis and tests completed successfully."
}

main()
{
    if [[ "${1:-}" == "-h" || "${1:-}" == "--help" || $# -lt 1 ]]; then
        usage
        exit 0
    fi

    local option="$1"
    option="$(echo "$option" | tr '[:upper:]' '[:lower:]')"
    shift

    case "$option" in
        1)
            run_option_1 "$@"
            ;;
        2)
            run_option_2 "${1:-}"
            ;;
        3)
            run_option_3
            ;;
        list)
            list_supported_option1_devices
            ;;
        *)
            echo -e "${RED}[APP]${NC} Invalid option: ${option}" >&2
            usage
            exit 1
            ;;
    esac
}

main "$@"
