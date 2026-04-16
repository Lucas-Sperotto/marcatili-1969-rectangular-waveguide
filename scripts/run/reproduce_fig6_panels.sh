#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build}"
INPUT_DIR="${INPUT_DIR:-$ROOT_DIR/data/input/fig6}"
OUTPUT_DIR="${OUTPUT_DIR:-$ROOT_DIR/data/output/fig6}"

"$ROOT_DIR/scripts/run/build.sh"

mkdir -p "$OUTPUT_DIR"

for input_file in "$INPUT_DIR"/*.json; do
    panel_name="$(basename "$input_file" .json)"
    output_json="$OUTPUT_DIR/${panel_name}.json"
    output_png="$OUTPUT_DIR/${panel_name}.png"

    echo "[fig6] $panel_name"
    "$BUILD_DIR/bin/reproduce_fig6" "$input_file" "$output_json"
    "$ROOT_DIR/scripts/plot_fig6.py" "${output_json%.json}.csv" -o "$output_png" --panel-title "$panel_name"
done

