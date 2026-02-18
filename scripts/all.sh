#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PORT="/dev/ttyUSB0"
PORT_EXPLICIT=false
FORCE_RELEASE_PORT=false

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

usage()
{
    echo "Usage: $0 [-p PORT] [-f|--force-release-port]"
    echo "  -p PORT    Serial port (default: /dev/ttyUSB0)"
    echo "  -f         Force release busy port using fuser -k"
    echo "  --force-release-port  Same as -f"
}

find_serial_port()
{
    for candidate in /dev/ttyUSB* /dev/ttyACM*; do
        if [[ -e "$candidate" ]]; then
            echo "$candidate"
            return 0
        fi
    done

    return 1
}

filtered_args=()
while [[ $# -gt 0 ]]; do
    case "$1" in
        --force-release-port)
            FORCE_RELEASE_PORT=true
            shift
            ;;
        *)
            filtered_args+=("$1")
            shift
            ;;
    esac
done
set -- "${filtered_args[@]}"

while getopts ":p:hf" opt; do
    case $opt in
        p) PORT="$OPTARG"; PORT_EXPLICIT=true ;;
        f) FORCE_RELEASE_PORT=true ;;
        h) usage; exit 0 ;;
        :) echo "Option -$OPTARG requires an argument." >&2; usage >&2; exit 1 ;;
        \?) echo "Invalid option: -$OPTARG" >&2; usage >&2; exit 1 ;;
    esac
done

if [[ ! -e "$PORT" ]]; then
    if [[ "$PORT_EXPLICIT" == true ]]; then
        echo -e "${RED}[ALL]${NC} Selected port does not exist: ${PORT}" >&2
        exit 1
    fi

    if detected_port="$(find_serial_port)"; then
        echo -e "${YELLOW}[PORT]${NC} ${PORT} not found. Using detected port: ${detected_port}"
        PORT="$detected_port"
    else
        echo -e "${RED}[ALL]${NC} No serial port detected (/dev/ttyUSB* or /dev/ttyACM*)." >&2
        echo "  Connect the board and retry, or pass one explicitly with -p." >&2
        exit 1
    fi
fi

if command -v fuser >/dev/null 2>&1 && fuser "$PORT" >/dev/null 2>&1; then
    if [[ "$FORCE_RELEASE_PORT" == true ]]; then
        echo -e "${YELLOW}[PORT]${NC} Releasing busy port: ${PORT}"
        fuser -k "$PORT" >/dev/null 2>&1 || true
        sleep 1
        if command -v fuser >/dev/null 2>&1 && fuser "$PORT" >/dev/null 2>&1; then
            echo -e "${RED}[ALL]${NC} Unable to release busy port: ${PORT}" >&2
            echo "  Try closing the process manually and retry." >&2
            exit 1
        fi
    else
        echo -e "${RED}[ALL]${NC} Port is busy: ${PORT}" >&2
        echo "  Process using the port:" >&2
        fuser "$PORT" >&2 || true
        echo "  Close the process manually or retry with --force-release-port." >&2
        exit 1
    fi
fi

echo -e "${GREEN}[ALL]${NC} Starting full workflow: build → flash → monitor"
"${SCRIPT_DIR}/build.sh"
"${SCRIPT_DIR}/flash.sh" -p "$PORT"
"${SCRIPT_DIR}/monitor.sh" -p "$PORT"
