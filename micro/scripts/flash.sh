#!/usr/bin/env bash
# flash.sh — Flash firmware to ESP32-C3
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
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

cd "$PROJECT_DIR"

echo -e "${GREEN}[FLASH]${NC} Flashing to ${PORT}..."

if idf.py -p "$PORT" flash; then
    echo -e "${GREEN}[FLASH]${NC} Flash succeeded ✔"
else
    echo -e "${RED}[FLASH]${NC} Flash failed ✘" >&2
    exit 1
fi
