#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
OUTPUT_JSON="${OUTPUT_JSON:-$ROOT_DIR/data/output/reproduce_fig11.json}"
OUTPUT_PNG="${OUTPUT_PNG:-$ROOT_DIR/data/output/reproduce_fig11.png}"
ARTICLE_STYLE_PNG="${ARTICLE_STYLE_PNG:-$ROOT_DIR/data/output/reproduce_fig11.article_style.png}"
COMPARISON_PNG="${COMPARISON_PNG:-$ROOT_DIR/data/output/reproduce_fig11_article_compare.png}"

"$ROOT_DIR/scripts/run/reproduce_fig11_case.sh"
"$ROOT_DIR/scripts/plot_fig11.py" "$ROOT_DIR/data/output/reproduce_fig11.csv" \
    -o "$ARTICLE_STYLE_PNG" \
    --no-title
"$ROOT_DIR/scripts/compare_fig11_article.py" "$ARTICLE_STYLE_PNG" -o "$COMPARISON_PNG"

echo "Wrote Figure 11 comparison artifacts:"
echo "  $OUTPUT_JSON"
echo "  $OUTPUT_PNG"
echo "  $ARTICLE_STYLE_PNG"
echo "  $COMPARISON_PNG"
