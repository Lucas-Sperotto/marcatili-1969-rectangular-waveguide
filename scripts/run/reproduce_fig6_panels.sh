#!/usr/bin/env bash

set -Eeuo pipefail

readonly SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
readonly ROOT_DIR="$(cd -- "$SCRIPT_DIR/../.." && pwd)"
readonly BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build}"
readonly INPUT_DIR="${INPUT_DIR:-$ROOT_DIR/data/input/fig6}"
readonly OUTPUT_DIR="${OUTPUT_DIR:-$ROOT_DIR/data/output/fig6}"
readonly PYTHON_BIN="${PYTHON_BIN:-python3}"
readonly REPRODUCE_APP="$BUILD_DIR/bin/reproduce_fig6"
readonly BUILD_SCRIPT="$ROOT_DIR/scripts/run/build.sh"
readonly PLOT_SCRIPT="$ROOT_DIR/scripts/plot_fig6.py"

log() {
    printf '[%s] %s\n' "$1" "$2"
}

die() {
    printf '[error] %s\n' "$1" >&2
    exit 1
}

require_file() {
    local path="$1"
    [[ -f "$path" ]] || die "Required file not found: $path"
}

require_dir() {
    local path="$1"
    [[ -d "$path" ]] || die "Required directory not found: $path"
}

require_executable() {
    local path="$1"
    [[ -x "$path" ]] || die "Required executable not found or not executable: $path"
}

require_command() {
    local name="$1"
    command -v "$name" >/dev/null 2>&1 || die "Required command not found: $name"
}

build_project() {
    require_file "$BUILD_SCRIPT"
    log "build" "Building C++ applications"
    bash "$BUILD_SCRIPT"
}

prepare_output_dir() {
    log "mkdir" "Ensuring output directory exists: $OUTPUT_DIR"
    mkdir -p -- "$OUTPUT_DIR"
}

reproduce_panel() {
    local input_file="$1"
    local panel_name
    panel_name="$(basename "$input_file" .json)"

    local output_json="$OUTPUT_DIR/${panel_name}.json"
    local output_csv="$OUTPUT_DIR/${panel_name}.csv"
    local output_png="$OUTPUT_DIR/${panel_name}.png"

    log "fig6" "$panel_name"
    "$REPRODUCE_APP" "$input_file" "$output_json"

    [[ -f "$output_csv" ]] || die "Expected CSV not generated: $output_csv"

    "$PYTHON_BIN" "$PLOT_SCRIPT" \
        "$output_csv" \
        -o "$output_png" \
        --panel-title "$panel_name"
}

run_all_panels() {
    require_dir "$INPUT_DIR"
    require_executable "$REPRODUCE_APP"
    require_file "$PLOT_SCRIPT"
    require_command "$PYTHON_BIN"

    shopt -s nullglob
    local input_files=("$INPUT_DIR"/*.json)
    shopt -u nullglob

    [[ ${#input_files[@]} -gt 0 ]] || die "No JSON input files found in: $INPUT_DIR"

    local input_file
    for input_file in "${input_files[@]}"; do
        reproduce_panel "$input_file"
    done
}

main() {
    build_project
    prepare_output_dir
    run_all_panels
    log "done" "Figure 6 panel reproductions completed."
}

main "$@"
