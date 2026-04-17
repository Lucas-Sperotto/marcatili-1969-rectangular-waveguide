#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

echo "[1/3] Cleaning previous build/output artifacts"
"$ROOT_DIR/scripts/run/clean.sh"

echo "[2/3] Building C++ applications"
"$ROOT_DIR/scripts/run/build.sh"

echo "[3/3] Reproducing all configured cases and figures"
SKIP_BUILD=1 "$ROOT_DIR/scripts/run/reproduce_all.sh"

echo "[done] Clean build and full reproduction completed."
