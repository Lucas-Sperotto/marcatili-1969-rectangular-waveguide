#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build}"
OUTPUT_DIR="${OUTPUT_DIR:-$ROOT_DIR/data/output}"

"$ROOT_DIR/run/build.sh"

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

