#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build}"
OUTPUT_DIR="${OUTPUT_DIR:-$ROOT_DIR/data/output}"

rm -rf "$BUILD_DIR"

if [[ -d "$OUTPUT_DIR" ]]; then
    # By default keep tracked artifacts untouched to avoid dirtying the git tree.
    # Set CLEAN_TRACKED_OUTPUT=1 to remove tracked output files as well.
    while IFS= read -r -d '' file; do
        relative_path="${file#$ROOT_DIR/}"

        if [[ "${CLEAN_TRACKED_OUTPUT:-0}" != "1" ]] \
            && git -C "$ROOT_DIR" ls-files --error-unmatch "$relative_path" >/dev/null 2>&1; then
            continue
        fi

        rm -f "$file"
    done < <(
        find "$OUTPUT_DIR" -type f \
            \( -name '*.json' -o -name '*.csv' -o -name '*.png' \) \
            -print0
    )
fi
