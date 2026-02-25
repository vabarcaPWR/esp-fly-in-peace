#!/usr/bin/env bash

ensure_flutter_available()
{
    if command -v flutter >/dev/null 2>&1; then
        return 0
    fi

    local -a candidate_bins=(
        "/snap/bin"
        "/var/lib/snapd/snap/bin"
        "${HOME}/flutter/bin"
        "${HOME}/development/flutter/bin"
        "${HOME}/sdk/flutter/bin"
    )

    local bin_dir
    for bin_dir in "${candidate_bins[@]}"; do
        if [[ -x "${bin_dir}/flutter" ]]; then
            export PATH="${bin_dir}:${PATH}"
            break
        fi
    done

    if command -v flutter >/dev/null 2>&1; then
        return 0
    fi

    local context="${1:-APP}"
    if [[ -n "${RED:-}" && -n "${NC:-}" ]]; then
        echo -e "${RED}[${context}]${NC} Flutter not found in PATH." >&2
    else
        echo "[${context}] Flutter not found in PATH." >&2
    fi

    echo "  Checked common locations: /snap/bin, /var/lib/snapd/snap/bin, ~/flutter/bin, ~/development/flutter/bin, ~/sdk/flutter/bin" >&2
    echo "  Fix options:" >&2
    echo "    1) Add Flutter to PATH in your shell profile" >&2
    echo "    2) Install Flutter and retry" >&2
    return 1
}