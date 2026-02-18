#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PORT="/dev/ttyUSB0"

GREEN='\033[0;32m'
NC='\033[0m'

usage()
{
    echo "Usage: $0 [-p PORT]"
    echo "  -p PORT    Serial port (default: /dev/ttyUSB0)"
}

while getopts ":p:h" opt; do
    case $opt in
        p) PORT="$OPTARG" ;;
        h) usage; exit 0 ;;
        :) echo "Option -$OPTARG requires an argument." >&2; usage >&2; exit 1 ;;
        \?) echo "Invalid option: -$OPTARG" >&2; usage >&2; exit 1 ;;
    esac
done

echo -e "${GREEN}[ALL]${NC} Starting full workflow: build → flash → monitor"
"${SCRIPT_DIR}/build.sh"
"${SCRIPT_DIR}/flash.sh" -p "$PORT"
"${SCRIPT_DIR}/monitor.sh" -p "$PORT"
