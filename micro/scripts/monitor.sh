#!/usr/bin/env bash
# monitor.sh — Open serial monitor to ESP32-C3
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
PORT="/dev/ttyUSB0"

GREEN='\033[0;32m'
NC='\033[0m'

while getopts "p:" opt; do
    case $opt in
        p) PORT="$OPTARG" ;;
        *) echo "Usage: $0 [-p PORT]" >&2; exit 1 ;;
    esac
done

cd "$PROJECT_DIR"

echo -e "${GREEN}[MONITOR]${NC} Opening monitor on ${PORT}... (Ctrl+] to exit)"

idf.py -p "$PORT" monitor
