# env.sh — ESP-IDF environment activation/deactivation
# Usage:
#   Activate:   source ./scripts/env.sh   (or . ./scripts/env.sh)
#   Deactivate: idf_deactivate
#
# DO NOT execute directly (./scripts/env.sh) — must be sourced to modify the shell.

_GREEN='\033[0;32m'
_YELLOW='\033[0;33m'
_RED='\033[0;31m'
_NC='\033[0m'

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    echo -e "${_RED}[ENV]${_NC} This script must be sourced, not executed." >&2
    echo -e "      Usage: ${_YELLOW}source ./scripts/env.sh${_NC}" >&2
    exit 1
fi

idf_deactivate()
{
    if [[ -z "${_IDF_OLD_PATH+x}" ]]; then
        echo -e "${_YELLOW}[ENV]${_NC} ESP-IDF environment is not active."
        return 1
    fi

    export PATH="$_IDF_OLD_PATH"
    unset _IDF_OLD_PATH
    unset IDF_PATH
    unset IDF_PYTHON_ENV_PATH
    unset IDF_TOOLS_EXPORT_CMD
    unset IDF_TOOLS_INSTALL_CMD
    unset IDF_DEACTIVATE

    echo -e "${_GREEN}[ENV]${_NC} ESP-IDF environment deactivated. Original PATH restored."
}

if [[ -n "${_IDF_OLD_PATH+x}" ]]; then
    echo -e "${_YELLOW}[ENV]${_NC} ESP-IDF environment is already active. Run ${_YELLOW}idf_deactivate${_NC} first to re-source."
    return 0
fi

if [[ -z "${IDF_PATH}" ]]; then
    _IDF_CANDIDATES=(
        "$HOME/.espressif/v5.5.2/esp-idf"
        "$HOME/esp/esp-idf"
        "$HOME/.espressif/esp-idf"
    )
    for _candidate in "${_IDF_CANDIDATES[@]}"; do
        if [[ -f "$_candidate/export.sh" ]]; then
            export IDF_PATH="$_candidate"
            break
        fi
    done
    unset _candidate _IDF_CANDIDATES
fi

if [[ -z "${IDF_PATH}" || ! -f "${IDF_PATH}/export.sh" ]]; then
    echo -e "${_RED}[ENV]${_NC} ESP-IDF not found. Set IDF_PATH or install ESP-IDF." >&2
    return 1
fi

export _IDF_OLD_PATH="$PATH"

source "$IDF_PATH/export.sh" > /dev/null 2>&1
_rc=$?

if [[ $_rc -ne 0 ]]; then
    echo -e "${_RED}[ENV]${_NC} Failed to source ESP-IDF export.sh (exit code: $_rc)." >&2
    unset _IDF_OLD_PATH
    return 1
fi

echo -e "${_GREEN}[ENV]${_NC} ESP-IDF environment activated (${IDF_PATH})"
echo -e "      idf.py $(idf.py --version 2>/dev/null || echo 'version unknown')"
echo -e "      Deactivate with: ${_YELLOW}idf_deactivate${_NC}"

unset _rc
