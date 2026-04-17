#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build}"
INPUT_FILE="${INPUT_FILE:-$ROOT_DIR/data/input/reproduce_fig11.json}"
OUTPUT_JSON="${OUTPUT_JSON:-$ROOT_DIR/data/output/reproduce_fig11.json}"
OUTPUT_CSV="${OUTPUT_CSV:-${OUTPUT_JSON%.json}.csv}"
OUTPUT_PNG="${OUTPUT_PNG:-${OUTPUT_JSON%.json}.png}"

"$ROOT_DIR/scripts/run/build.sh"

mkdir -p "$(dirname "$OUTPUT_JSON")"

"$BUILD_DIR/bin/reproduce_fig11" "$INPUT_FILE" "$OUTPUT_JSON"
"$ROOT_DIR/scripts/plot_fig11.py" "$OUTPUT_CSV" -o "$OUTPUT_PNG"
