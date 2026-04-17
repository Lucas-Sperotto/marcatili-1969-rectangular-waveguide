#!/usr/bin/env bash

set -Eeuo pipefail

readonly SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
readonly ROOT_DIR="$(cd -- "$SCRIPT_DIR/../.." && pwd)"
readonly BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build}"
readonly OUTPUT_DIR="${OUTPUT_DIR:-$ROOT_DIR/data/output}"
readonly SKIP_BUILD="${SKIP_BUILD:-0}"
readonly PYTHON_BIN="${PYTHON_BIN:-python3}"

readonly -a APPS=(
    solve_single_guide
    solve_coupler
    reproduce_fig6
    reproduce_fig7
    reproduce_fig8
    reproduce_fig10
    reproduce_fig11
    reproduce_table1
)

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

require_executable() {
    local path="$1"
    [[ -x "$path" ]] || die "Required executable not found or not executable: $path"
}

require_command() {
    local name="$1"
    command -v "$name" >/dev/null 2>&1 || die "Required command not found: $name"
}

build_if_needed() {
    [[ "$SKIP_BUILD" == "1" ]] && return 0

    local build_script="$ROOT_DIR/scripts/run/build.sh"
    require_file "$build_script"

    log "build" "Building C++ applications"
    bash "$build_script"
}

prepare_output_dir() {
    log "mkdir" "Ensuring output directory exists: $OUTPUT_DIR"
    mkdir -p -- "$OUTPUT_DIR"
}

run_app() {
    local app="$1"
    local input_file="$ROOT_DIR/data/input/${app}.json"
    local output_file="$OUTPUT_DIR/${app}.json"
    local executable="$BUILD_DIR/bin/$app"

    require_file "$input_file"
    require_executable "$executable"

    log "run" "$app"
    "$executable" "$input_file" "$output_file"
}

run_all_apps() {
    local app
    for app in "${APPS[@]}"; do
        run_app "$app"
    done
}

run_fig6_panels() {
    local script="$ROOT_DIR/scripts/run/reproduce_fig6_panels.sh"
    require_file "$script"

    log "plot" "Generating Figure 6 panel reproductions"
    bash "$script"
}

plot_with_python_if_csv_exists() {
    local csv_file="$1"
    local plot_script="$2"
    local description="$3"
    shift 3

    [[ -f "$csv_file" ]] || return 0

    require_command "$PYTHON_BIN"
    require_file "$plot_script"

    log "plot" "$description"
    "$PYTHON_BIN" "$plot_script" "$csv_file" "$@"
}

main() {
    build_if_needed
    prepare_output_dir
    run_all_apps
    run_fig6_panels

    plot_with_python_if_csv_exists \
        "$OUTPUT_DIR/reproduce_fig7.lines.csv" \
        "$ROOT_DIR/scripts/plot_fig7.py" \
        "Rendering Figure 7" \
        --intersections-csv "$OUTPUT_DIR/reproduce_fig7.intersections.csv" \
        -o "$OUTPUT_DIR/reproduce_fig7.png"

    plot_with_python_if_csv_exists \
        "$OUTPUT_DIR/reproduce_fig8.csv" \
        "$ROOT_DIR/scripts/plot_fig8.py" \
        "Rendering Figure 8" \
        -o "$OUTPUT_DIR/reproduce_fig8.png"

    plot_with_python_if_csv_exists \
        "$OUTPUT_DIR/reproduce_fig10.csv" \
        "$ROOT_DIR/scripts/plot_fig10.py" \
        "Rendering Figure 10" \
        -o "$OUTPUT_DIR/reproduce_fig10.png"

    plot_with_python_if_csv_exists \
        "$OUTPUT_DIR/reproduce_fig11.csv" \
        "$ROOT_DIR/scripts/plot_fig11.py" \
        "Rendering Figure 11" \
        -o "$OUTPUT_DIR/reproduce_fig11.png"

    log "done" "All configured cases and figures reproduced."
}

main "$@"
