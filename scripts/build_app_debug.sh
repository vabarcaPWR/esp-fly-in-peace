#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(dirname "$SCRIPT_DIR")"
APP_DIR="${ROOT_DIR}/app"

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

usage()
{
    echo "Usage: $0 [flutter build args]"
    echo "Example: $0 --split-per-abi"
}

resolve_java_home()
{
    is_supported_java_home()
    {
        local java_home="$1"
        if [[ ! -x "${java_home}/bin/java" ]]; then
            return 1
        fi

        local java_version_line
        java_version_line="$(${java_home}/bin/java -version 2>&1 | head -n 1)"

        local java_major
        java_major="$(echo "$java_version_line" | sed -E 's/.*version "([0-9]+).*/\1/')"

        if [[ "$java_major" =~ ^[0-9]+$ ]] && [[ "$java_major" -ge 17 ]] && [[ "$java_major" -le 21 ]]; then
            return 0
        fi

        return 1
    }

    if [[ -n "${JAVA_HOME:-}" ]] && is_supported_java_home "$JAVA_HOME"; then
        return 0
    fi

    if [[ -n "${JAVA_HOME:-}" && -x "${JAVA_HOME}/bin/java" ]]; then
        echo -e "${YELLOW}[APP]${NC} Ignoring unsupported JAVA_HOME=${JAVA_HOME} (expected Java 17..21)."
    fi

    local candidates=(
        "$HOME/.local/jdk-21"
        "$HOME/.local/jdk-17"
        "/usr/lib/jvm/java-21-openjdk-amd64"
        "/usr/lib/jvm/java-17-openjdk-amd64"
        "/usr/lib/jvm/default-java"
    )

    local candidate
    for candidate in "${candidates[@]}"; do
        if is_supported_java_home "$candidate"; then
            JAVA_HOME="$candidate"
            export JAVA_HOME
            return 0
        fi
    done

    if command -v java >/dev/null 2>&1; then
        local java_path
        java_path="$(readlink -f "$(command -v java)")"
        local java_home_from_path
        java_home_from_path="$(dirname "$(dirname "$java_path")")"
        if is_supported_java_home "$java_home_from_path"; then
            JAVA_HOME="$java_home_from_path"
            export JAVA_HOME
            return 0
        fi

        echo -e "${YELLOW}[APP]${NC} java in PATH is unsupported for Android build: $java_path"
    fi

    for candidate in /usr/lib/jvm/*; do
        if is_supported_java_home "$candidate"; then
            JAVA_HOME="$candidate"
            export JAVA_HOME
            return 0
        fi
    done

    return 1
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
    usage
    exit 0
fi

if ! command -v flutter >/dev/null 2>&1; then
    echo -e "${RED}[APP]${NC} flutter is not available in PATH." >&2
    exit 1
fi

if ! resolve_java_home; then
    echo -e "${RED}[APP]${NC} Compatible Java runtime not found (required Java 17..21)." >&2
    echo "  Detected Java 25 in PATH is not supported by this Android/Gradle setup." >&2
    echo "  Install JDK 17 and set JAVA_HOME to it, for example:" >&2
    echo "  export JAVA_HOME=\$HOME/.local/jdk-17" >&2
    echo "  export PATH=\$JAVA_HOME/bin:\$PATH" >&2
    exit 1
fi

export PATH="${JAVA_HOME}/bin:${PATH}"

echo -e "${GREEN}[APP]${NC} Using JAVA_HOME=${JAVA_HOME}"
echo -e "${GREEN}[APP]${NC} $(java -version 2>&1 | head -n 1)"

echo -e "${YELLOW}[APP]${NC} Running Flutter debug APK build..."
cd "$APP_DIR"
flutter build apk --debug "$@"

echo -e "${GREEN}[APP]${NC} Debug APK build completed successfully."
