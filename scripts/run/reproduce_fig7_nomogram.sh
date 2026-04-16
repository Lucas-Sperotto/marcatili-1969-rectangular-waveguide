#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build}"
INPUT_FILE="${INPUT_FILE:-$ROOT_DIR/data/input/reproduce_fig7.json}"
OUTPUT_JSON="${OUTPUT_JSON:-$ROOT_DIR/data/output/reproduce_fig7.json}"

"$ROOT_DIR/scripts/run/build.sh"

mkdir -p "$(dirname "$OUTPUT_JSON")"

echo "[fig7] $(basename "$INPUT_FILE")"
"$BUILD_DIR/bin/reproduce_fig7" "$INPUT_FILE" "$OUTPUT_JSON"
"$ROOT_DIR/scripts/plot_fig7.py" \
    "${OUTPUT_JSON%.json}.lines.csv" \
    --intersections-csv "${OUTPUT_JSON%.json}.intersections.csv" \
    -o "${OUTPUT_JSON%.json}.png"
