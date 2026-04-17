#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
OUTPUT_DIR="${OUTPUT_DIR:-$ROOT_DIR/data/output}"

if [[ "${RUN_PIPELINE:-0}" == "1" ]]; then
    "$ROOT_DIR/scripts/run/clean_build_reproduce_all.sh"
fi

required_files=(
    "$OUTPUT_DIR/solve_single_guide.json"
    "$OUTPUT_DIR/solve_single_guide.csv"
    "$OUTPUT_DIR/solve_coupler.json"
    "$OUTPUT_DIR/solve_coupler.csv"
    "$OUTPUT_DIR/reproduce_fig6.json"
    "$OUTPUT_DIR/reproduce_fig6.csv"
    "$OUTPUT_DIR/reproduce_fig6.png"
    "$OUTPUT_DIR/reproduce_fig7.json"
    "$OUTPUT_DIR/reproduce_fig7.lines.csv"
    "$OUTPUT_DIR/reproduce_fig7.intersections.csv"
    "$OUTPUT_DIR/reproduce_fig7.png"
    "$OUTPUT_DIR/reproduce_fig8.json"
    "$OUTPUT_DIR/reproduce_fig8.csv"
    "$OUTPUT_DIR/reproduce_fig8.png"
    "$OUTPUT_DIR/reproduce_fig10.json"
    "$OUTPUT_DIR/reproduce_fig10.csv"
    "$OUTPUT_DIR/reproduce_fig10.png"
    "$OUTPUT_DIR/reproduce_fig11.json"
    "$OUTPUT_DIR/reproduce_fig11.csv"
    "$OUTPUT_DIR/reproduce_fig11.png"
    "$OUTPUT_DIR/reproduce_table1.json"
    "$OUTPUT_DIR/reproduce_table1.summary.csv"
    "$OUTPUT_DIR/reproduce_table1.details.csv"
)

missing_count=0
for file in "${required_files[@]}"; do
    if [[ -f "$file" ]]; then
        echo "[ok] $file"
    else
        echo "[missing] $file"
        missing_count=$((missing_count + 1))
    fi
done

if [[ "$missing_count" -ne 0 ]]; then
    echo "Reproduction check failed: $missing_count required artifact(s) missing." >&2
    exit 1
fi

echo "Reproduction checklist passed: all required artifacts exist."
