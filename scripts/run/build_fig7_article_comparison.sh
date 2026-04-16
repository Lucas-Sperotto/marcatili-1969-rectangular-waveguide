#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
OUTPUT_DIR="${OUTPUT_DIR:-$ROOT_DIR/data/output}"

"$ROOT_DIR/scripts/run/reproduce_fig7_nomogram.sh"

"$ROOT_DIR/scripts/plot_fig7.py" \
    "$OUTPUT_DIR/reproduce_fig7.lines.csv" \
    --intersections-csv "$OUTPUT_DIR/reproduce_fig7.intersections.csv" \
    --no-title \
    -o "$OUTPUT_DIR/reproduce_fig7_article_style.png"

"$ROOT_DIR/scripts/compare_fig7_article.py" \
    "$OUTPUT_DIR/reproduce_fig7_article_style.png" \
    -o "$OUTPUT_DIR/reproduce_fig7_article_compare.png"
