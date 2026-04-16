#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
OUTPUT_DIR="${OUTPUT_DIR:-$ROOT_DIR/data/output/fig6}"
COMPARE_DIR="${COMPARE_DIR:-$OUTPUT_DIR/article_compare}"

"$ROOT_DIR/scripts/run/reproduce_fig6_panels.sh"

mkdir -p "$COMPARE_DIR"

for panel_id in SG-006d SG-006k; do
    "$ROOT_DIR/scripts/compare_fig6_article.py" \
        "$panel_id" \
        "$OUTPUT_DIR/${panel_id}.png" \
        -o "$COMPARE_DIR/${panel_id}_article_compare.png"
done

for panel_id in SG-006h SG-006i SG-006j; do
    "$ROOT_DIR/scripts/compare_fig6_article.py" \
        "$panel_id" \
        "$OUTPUT_DIR/${panel_id}.png" \
        -o "$COMPARE_DIR/${panel_id}_article_compare.png"
done
