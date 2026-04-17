#!/usr/bin/env bash

set -Eeuo pipefail

readonly SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
readonly ROOT_DIR="$(cd -- "$SCRIPT_DIR/../.." && pwd)"
readonly BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build}"
readonly BUILD_TYPE="${BUILD_TYPE:-Release}"
readonly RUN_TESTS="${RUN_TESTS:-0}"

log() {
    printf '[%s] %s\n' "$1" "$2"
}

die() {
    printf '[error] %s\n' "$1" >&2
    exit 1
}

require_command() {
    local command_name="$1"
    command -v "$command_name" >/dev/null 2>&1 || die "Required command not found: $command_name"
}

is_working_ctest() {
    local candidate="$1"

    [[ -x "$candidate" ]] || return 1
    "$candidate" --version >/dev/null 2>&1
}

find_ctest() {
    if [[ -n "${CTEST_BIN:-}" ]]; then
        is_working_ctest "$CTEST_BIN" ||
            die "Configured CTEST_BIN is not executable or not working: $CTEST_BIN"
        printf '%s\n' "$CTEST_BIN"
        return 0
    fi

    local cmake_path
    cmake_path="$(command -v cmake)" || die "Required command not found: cmake"

    local cmake_bin_dir
    cmake_bin_dir="$(dirname "$cmake_path")"

    local detected_ctest="$cmake_bin_dir/ctest"
    if is_working_ctest "$detected_ctest"; then
        printf '%s\n' "$detected_ctest"
        return 0
    fi

    local path_ctest
    path_ctest="$(command -v ctest)" || die "Unable to find a working ctest binary."

    if is_working_ctest "$path_ctest"; then
        printf '%s\n' "$path_ctest"
        return 0
    fi

    if is_working_ctest "/usr/bin/ctest"; then
        printf '%s\n' "/usr/bin/ctest"
        return 0
    fi

    die "Unable to find a working ctest binary."
}

configure() {
    log "build" "Configuring project with CMake ($BUILD_TYPE)"
    cmake -S "$ROOT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
}

build() {
    log "build" "Building project"

    if [[ -n "${BUILD_JOBS:-}" ]]; then
        cmake --build "$BUILD_DIR" --parallel "$BUILD_JOBS"
    else
        cmake --build "$BUILD_DIR"
    fi
}

run_tests() {
    [[ "$RUN_TESTS" == "1" ]] || return 0

    local ctest_bin
    ctest_bin="$(find_ctest)"

    log "test" "Running test suite"
    "$ctest_bin" --test-dir "$BUILD_DIR" --output-on-failure
}

main() {
    require_command cmake
    configure
    build
    run_tests
}

main "$@"
