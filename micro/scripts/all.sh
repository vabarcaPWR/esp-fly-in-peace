#!/usr/bin/env bash
# all.sh — Build → Flash → Monitor (full workflow)
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PORT="/dev/ttyUSB0"

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

while getopts "p:" opt; do
    case $opt in
        p) PORT="$OPTARG" ;;
        *) echo "Usage: $0 [-p PORT]" >&2; exit 1 ;;
    esac
done

echo -e "${GREEN}[ALL]${NC} Starting full workflow: build → flash → monitor"

"$SCRIPT_DIR/build.sh"
"$SCRIPT_DIR/flash.sh" -p "$PORT"
"$SCRIPT_DIR/monitor.sh" -p "$PORT"
