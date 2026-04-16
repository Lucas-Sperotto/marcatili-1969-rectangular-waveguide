#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build}"
OUTPUT_DIR="${OUTPUT_DIR:-$ROOT_DIR/data/output}"

"$ROOT_DIR/scripts/run/build.sh"

mkdir -p "$OUTPUT_DIR"

APPS=(
    solve_single_guide
    solve_coupler
    reproduce_fig6
    reproduce_fig7
    reproduce_fig8
    reproduce_fig10
    reproduce_fig11
    reproduce_table1
)

for app in "${APPS[@]}"; do
    input_file="$ROOT_DIR/data/input/${app}.json"
    output_file="$OUTPUT_DIR/${app}.json"

    echo "[run] $app"
    "$BUILD_DIR/bin/$app" "$input_file" "$output_file"
done

# Keep the default one-file reproductions accompanied by their PNG renderings.
if [[ -f "$OUTPUT_DIR/reproduce_fig6.csv" ]]; then
    "$ROOT_DIR/scripts/plot_fig6.py" \
        "$OUTPUT_DIR/reproduce_fig6.csv" \
        -o "$OUTPUT_DIR/reproduce_fig6.png"
fi

if [[ -f "$OUTPUT_DIR/reproduce_fig7.lines.csv" ]]; then
    "$ROOT_DIR/scripts/plot_fig7.py" \
        "$OUTPUT_DIR/reproduce_fig7.lines.csv" \
        --intersections-csv "$OUTPUT_DIR/reproduce_fig7.intersections.csv" \
        -o "$OUTPUT_DIR/reproduce_fig7.png"
fi
