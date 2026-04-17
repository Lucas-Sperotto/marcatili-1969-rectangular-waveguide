#!/usr/bin/env bash

set -Eeuo pipefail

readonly SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
readonly ROOT_DIR="$(cd -- "$SCRIPT_DIR/../.." && pwd)"

readonly CLEAN_SCRIPT="$ROOT_DIR/scripts/run/clean.sh"
readonly BUILD_SCRIPT="$ROOT_DIR/scripts/run/build.sh"
readonly REPRODUCE_SCRIPT="$ROOT_DIR/scripts/run/reproduce_all.sh"

log() {
    printf '[%s] %s\n' "$1" "$2"
}

die() {
    printf '[error] %s\n' "$1" >&2
    exit 1
}

require_file() {
    local file="$1"
    [[ -f "$file" ]] || die "Required script not found: $file"
}

run_step() {
    local step="$1"
    local message="$2"
    shift 2

    log "$step" "$message"
    "$@"
}

on_error() {
    local exit_code=$?
    printf '[error] Command failed with exit code %d: %s\n' "$exit_code" "$BASH_COMMAND" >&2
    exit "$exit_code"
}

main() {
    require_file "$CLEAN_SCRIPT"
    require_file "$BUILD_SCRIPT"
    require_file "$REPRODUCE_SCRIPT"

    run_step "1/3" "Cleaning previous build/output artifacts" \
        bash "$CLEAN_SCRIPT"

    run_step "2/3" "Building C++ applications" \
        bash "$BUILD_SCRIPT"

    run_step "3/3" "Reproducing all configured cases and figures" \
        env SKIP_BUILD=1 bash "$REPRODUCE_SCRIPT"

    log "done" "Clean build and full reproduction completed."
}

trap on_error ERR

main "$@"