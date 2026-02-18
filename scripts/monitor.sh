#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(dirname "$SCRIPT_DIR")"
MICRO_DIR="${ROOT_DIR}/micro"
PORT="/dev/ttyUSB0"

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
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

if ! command -v idf.py >/dev/null 2>&1; then
    if [[ -f "${SCRIPT_DIR}/env.sh" ]]; then
        echo -e "${YELLOW}[ENV]${NC} idf.py not found. Sourcing scripts/env.sh..."
        # shellcheck source=/dev/null
        source "${SCRIPT_DIR}/env.sh"
    fi
fi

if ! command -v idf.py >/dev/null 2>&1; then
    echo -e "${RED}[MONITOR]${NC} idf.py is not available. Source ESP-IDF env first:" >&2
    echo "  source ./scripts/env.sh" >&2
    exit 1
fi

cd "$MICRO_DIR"
echo -e "${GREEN}[MONITOR]${NC} Opening monitor on ${PORT}... (Ctrl+] to exit)"
idf.py -p "$PORT" monitor
