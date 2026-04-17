#!/usr/bin/env bash

set -Eeuo pipefail

readonly SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
readonly ROOT_DIR="$(cd -- "$SCRIPT_DIR/../.." && pwd)"
readonly BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build}"
readonly OUTPUT_DIR="${OUTPUT_DIR:-$ROOT_DIR/data/output}"
readonly CLEAN_TRACKED_OUTPUT="${CLEAN_TRACKED_OUTPUT:-0}"

log() {
    printf '[%s] %s\n' "$1" "$2"
}

remove_build_dir() {
    [[ -d "$BUILD_DIR" ]] || return 0
    log "clean" "Removing build directory: $BUILD_DIR"
    rm -rf -- "$BUILD_DIR"
}

is_tracked_file() {
    local file="$1"
    local relative_path="${file#$ROOT_DIR/}"

    git -C "$ROOT_DIR" ls-files --error-unmatch "$relative_path" >/dev/null 2>&1
}

should_remove_file() {
    local file="$1"

    if [[ "$CLEAN_TRACKED_OUTPUT" != "1" ]] && is_tracked_file "$file"; then
        return 1
    fi

    return 0
}

clean_output_files() {
    local file
    local removed_count=0
    local skipped_count=0

    [[ -d "$OUTPUT_DIR" ]] || return 0

    while IFS= read -r -d '' file; do
        if should_remove_file "$file"; then
            rm -f -- "$file"
            ((removed_count += 1))
        else
            ((skipped_count += 1))
        fi
    done < <(
        find "$OUTPUT_DIR" -type f \
            \( -name '*.json' -o -name '*.csv' -o -name '*.png' \) \
            -print0
    )

    log "clean" "Output files removed: $removed_count"
    if [[ "$CLEAN_TRACKED_OUTPUT" != "1" ]]; then
        log "clean" "Tracked output files preserved: $skipped_count"
    fi
}

main() {
    remove_build_dir
    clean_output_files
}

main "$@"