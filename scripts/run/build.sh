#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
CMAKE_BIN_DIR="$(dirname "$(command -v cmake)")"
CTEST_BIN="${CTEST_BIN:-$CMAKE_BIN_DIR/ctest}"

cmake -S "$ROOT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
cmake --build "$BUILD_DIR"

if [[ "${RUN_TESTS:-0}" == "1" ]]; then
    if [[ ! -x "$CTEST_BIN" ]]; then
        echo "Unable to find a working ctest binary." >&2
        exit 1
    fi

    "$CTEST_BIN" --test-dir "$BUILD_DIR" --output-on-failure
fi
