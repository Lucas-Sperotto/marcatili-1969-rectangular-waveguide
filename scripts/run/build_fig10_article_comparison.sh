#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
OUTPUT_JSON="${OUTPUT_JSON:-$ROOT_DIR/data/output/reproduce_fig10.json}"
ARTICLE_STYLE_PNG="${ARTICLE_STYLE_PNG:-${OUTPUT_JSON%.json}.article_style.png}"
COMPARISON_PNG="${COMPARISON_PNG:-${OUTPUT_JSON%.json}_article_compare.png}"

"$ROOT_DIR/scripts/run/reproduce_fig10_case.sh"

"$ROOT_DIR/scripts/plot_fig10.py" \
    "${OUTPUT_JSON%.json}.csv" \
    --no-title \
    -o "$ARTICLE_STYLE_PNG"

"$ROOT_DIR/scripts/compare_fig10_article.py" \
    "$ARTICLE_STYLE_PNG" \
    -o "$COMPARISON_PNG"
