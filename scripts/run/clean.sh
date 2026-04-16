#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build}"
OUTPUT_DIR="${OUTPUT_DIR:-$ROOT_DIR/data/output}"

rm -rf "$BUILD_DIR"

if [[ -d "$OUTPUT_DIR" ]]; then
    find "$OUTPUT_DIR" -type f \( -name '*.json' -o -name '*.csv' -o -name '*.png' \) -delete
fi
