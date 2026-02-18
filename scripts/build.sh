#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(dirname "$SCRIPT_DIR")"
MICRO_DIR="${ROOT_DIR}/micro"

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

if ! command -v idf.py >/dev/null 2>&1; then
    if [[ -f "${SCRIPT_DIR}/env.sh" ]]; then
        echo -e "${YELLOW}[ENV]${NC} idf.py not found. Sourcing scripts/env.sh..."
        # shellcheck source=/dev/null
        source "${SCRIPT_DIR}/env.sh"
    fi
fi

if ! command -v idf.py >/dev/null 2>&1; then
    echo -e "${RED}[BUILD]${NC} idf.py is not available. Source ESP-IDF env first:" >&2
    echo "  source ./scripts/env.sh" >&2
    exit 1
fi

cd "$MICRO_DIR"
echo -e "${GREEN}[BUILD]${NC} Building esp-fly-in-peace firmware..."

if idf.py build; then
    echo -e "${GREEN}[BUILD]${NC} Build succeeded ✔"
else
    echo -e "${RED}[BUILD]${NC} Build failed ✘" >&2
    exit 1
fi
