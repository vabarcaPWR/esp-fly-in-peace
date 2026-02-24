#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(dirname "$(dirname "$SCRIPT_DIR")")"
TEST_DIR="${ROOT_DIR}/micro/test"

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

cd "$TEST_DIR"
echo -e "${GREEN}[TEST]${NC} Running Ceedling unit tests..."

if ceedling test:all; then
    echo -e "${GREEN}[TEST]${NC} All tests passed ✔"
else
    echo -e "${RED}[TEST]${NC} Tests failed ✘" >&2
    exit 1
fi
