#!/usr/bin/env bash
# build.sh — Build the esp-fly-in-peace firmware
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

cd "$PROJECT_DIR"

echo -e "${GREEN}[BUILD]${NC} Building esp-fly-in-peace firmware..."

if idf.py build; then
    echo -e "${GREEN}[BUILD]${NC} Build succeeded ✔"
else
    echo -e "${RED}[BUILD]${NC} Build failed ✘" >&2
    exit 1
fi
