#!/usr/bin/env bash

set -Eeuo pipefail

readonly SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
readonly ROOT_DIR="$(cd -- "$SCRIPT_DIR/.." && pwd)"

readonly BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build}"
readonly BUILD_TYPE="${BUILD_TYPE:-Release}"
readonly OUTPUT_DIR="${OUTPUT_DIR:-$ROOT_DIR/data/output}"
readonly FIG6_INPUT_DIR="${FIG6_INPUT_DIR:-$ROOT_DIR/data/input/fig6}"
readonly FIG6_OUTPUT_DIR="${FIG6_OUTPUT_DIR:-$ROOT_DIR/data/output/fig6}"
readonly PYTHON_BIN="${PYTHON_BIN:-python3}"
readonly RUN_TESTS="${RUN_TESTS:-0}"
readonly CLEAN_TRACKED_OUTPUT="${CLEAN_TRACKED_OUTPUT:-0}"
readonly RUN_PIPELINE="${RUN_PIPELINE:-0}"

readonly -a NON_FIGURE_APPS=(
    solve_single_guide
    solve_coupler
    reproduce_table1
)

log() {
    printf '[%s] %s\n' "$1" "$2"
}

die() {
    printf '[error] %s\n' "$1" >&2
    exit 1
}

on_error() {
    local exit_code=$?
    printf '[error] Command failed with exit code %d at line %d: %s\n' \
        "$exit_code" "${BASH_LINENO[0]}" "$BASH_COMMAND" >&2
    exit "$exit_code"
}

trap on_error ERR

require_command() {
    local name="$1"
    command -v "$name" >/dev/null 2>&1 || die "Required command not found: $name"
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

is_git_repo() {
    git -C "$ROOT_DIR" rev-parse --is-inside-work-tree >/dev/null 2>&1
}

is_tracked_file() {
    local file="$1"
    local relative_path="${file#$ROOT_DIR/}"
    git -C "$ROOT_DIR" ls-files --error-unmatch "$relative_path" >/dev/null 2>&1
}

is_working_ctest() {
    local candidate="$1"

    [[ -x "$candidate" ]] || return 1
    "$candidate" --version >/dev/null 2>&1
}

find_ctest() {
    if [[ -n "${CTEST_BIN:-}" ]]; then
        is_working_ctest "$CTEST_BIN" ||
            die "Configured CTEST_BIN is not executable or not working: $CTEST_BIN"
        printf '%s\n' "$CTEST_BIN"
        return 0
    fi

    local cmake_path
    cmake_path="$(command -v cmake)" || die "Required command not found: cmake"

    local cmake_bin_dir
    cmake_bin_dir="$(dirname "$cmake_path")"

    local detected_ctest="$cmake_bin_dir/ctest"
    if is_working_ctest "$detected_ctest"; then
        printf '%s\n' "$detected_ctest"
        return 0
    fi

    local path_ctest
    path_ctest="$(command -v ctest)" || die "Unable to find a working ctest binary."

    if is_working_ctest "$path_ctest"; then
        printf '%s\n' "$path_ctest"
        return 0
    fi

    if is_working_ctest "/usr/bin/ctest"; then
        printf '%s\n' "/usr/bin/ctest"
        return 0
    fi

    die "Unable to find a working ctest binary."
}

clean_build_dir() {
    if [[ -d "$BUILD_DIR" ]]; then
        log "clean" "Removing build directory: $BUILD_DIR"
        rm -rf -- "$BUILD_DIR"
    fi
}

clean_output_files() {
    [[ -d "$OUTPUT_DIR" ]] || return 0

    local file
    local removed_count=0
    local skipped_count=0

    while IFS= read -r -d '' file; do
        if [[ "$CLEAN_TRACKED_OUTPUT" != "1" ]] && is_git_repo && is_tracked_file "$file"; then
            ((skipped_count += 1))
            continue
        fi

        rm -f -- "$file"
        ((removed_count += 1))
    done < <(
        find "$OUTPUT_DIR" -type f \
            \( -name '*.json' -o -name '*.csv' -o -name '*.png' \) \
            -print0
    )

    log "clean" "Output files removed: $removed_count"
    if [[ "$CLEAN_TRACKED_OUTPUT" != "1" ]]; then
        log "clean" "Tracked output files preserved: $skipped_count"
    fi
}

clean_all() {
    clean_build_dir
    clean_output_files
}

configure_project() {
    require_command cmake
    log "build" "Configuring project with CMake ($BUILD_TYPE)"
    cmake -S "$ROOT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
}

compile_project() {
    log "build" "Building project"
    if [[ -n "${BUILD_JOBS:-}" ]]; then
        cmake --build "$BUILD_DIR" --parallel "$BUILD_JOBS"
    else
        cmake --build "$BUILD_DIR"
    fi
}

run_tests_if_requested() {
    [[ "$RUN_TESTS" == "1" ]] || return 0

    local ctest_bin
    ctest_bin="$(find_ctest)"

    log "test" "Running test suite"
    "$ctest_bin" --test-dir "$BUILD_DIR" --output-on-failure
}

build_project() {
    configure_project
    compile_project
    run_tests_if_requested
}

prepare_output_dir() {
    mkdir -p -- "$OUTPUT_DIR"
}

prepare_fig6_output_dir() {
    mkdir -p -- "$FIG6_OUTPUT_DIR"
}

run_app() {
    local app="$1"
    local input_file="$2"
    local output_file="$3"
    local executable="$BUILD_DIR/bin/$app"

    require_file "$input_file"
    require_executable "$executable"

    log "run" "$app"
    "$executable" "$input_file" "$output_file"
}

run_non_figure_apps() {
    local app
    for app in "${NON_FIGURE_APPS[@]}"; do
        run_app \
            "$app" \
            "$ROOT_DIR/data/input/${app}.json" \
            "$OUTPUT_DIR/${app}.json"
    done
}

run_plot_script() {
    local description="$1"
    local plot_script="$2"
    shift 2

    require_command "$PYTHON_BIN"
    require_file "$plot_script"

    log "plot" "$description"
    "$PYTHON_BIN" "$plot_script" "$@"
}

panel_label_from_name() {
    local panel_name="$1"

    if [[ "$panel_name" =~ ([A-Za-z])$ ]]; then
        printf '(%s)\n' "${BASH_REMATCH[1],,}"
        return 0
    fi

    printf '%s\n' "$panel_name"
}

render_fig6_panels() {
    require_dir "$FIG6_INPUT_DIR"
    require_executable "$BUILD_DIR/bin/reproduce_fig6"

    prepare_fig6_output_dir

    shopt -s nullglob
    local input_files=("$FIG6_INPUT_DIR"/*.json)
    shopt -u nullglob

    [[ ${#input_files[@]} -gt 0 ]] || die "No JSON input files found in: $FIG6_INPUT_DIR"

    local input_file
    for input_file in "${input_files[@]}"; do
        local panel_name
        panel_name="$(basename "$input_file" .json)"

        local output_json="$FIG6_OUTPUT_DIR/${panel_name}.json"
        local output_csv="$FIG6_OUTPUT_DIR/${panel_name}.csv"
        local output_png="$FIG6_OUTPUT_DIR/${panel_name}.png"
        local panel_label
        panel_label="$(panel_label_from_name "$panel_name")"

        log "fig6" "$panel_name"
        "$BUILD_DIR/bin/reproduce_fig6" "$input_file" "$output_json"

        [[ -f "$output_csv" ]] || die "Expected CSV not generated: $output_csv"

        run_plot_script \
            "Rendering Figure 6 panel $panel_name" \
            "$ROOT_DIR/scripts/plot_fig6.py" \
            "$output_csv" \
            -o "$output_png" \
            --panel-title "$panel_label"
    done
}

render_figure7() {
    local output_json="$OUTPUT_DIR/reproduce_fig7.json"
    local lines_csv="$OUTPUT_DIR/reproduce_fig7.lines.csv"
    local intersections_csv="$OUTPUT_DIR/reproduce_fig7.intersections.csv"
    local output_png="$OUTPUT_DIR/reproduce_fig7.png"

    run_app "reproduce_fig7" "$ROOT_DIR/data/input/reproduce_fig7.json" "$output_json"
    require_file "$lines_csv"
    require_file "$intersections_csv"

    run_plot_script \
        "Rendering Figure 7" \
        "$ROOT_DIR/scripts/plot_fig7.py" \
        "$lines_csv" \
        --intersections-csv "$intersections_csv" \
        --no-title \
        -o "$output_png"
}

render_figure8() {
    local output_json="$OUTPUT_DIR/reproduce_fig8.json"
    local output_csv="$OUTPUT_DIR/reproduce_fig8.csv"
    local output_png="$OUTPUT_DIR/reproduce_fig8.png"

    run_app "reproduce_fig8" "$ROOT_DIR/data/input/reproduce_fig8.json" "$output_json"
    require_file "$output_csv"

    run_plot_script \
        "Rendering Figure 8" \
        "$ROOT_DIR/scripts/plot_fig8.py" \
        "$output_csv" \
        --no-title \
        -o "$output_png"
}

render_figure10() {
    local output_json="$OUTPUT_DIR/reproduce_fig10.json"
    local output_csv="$OUTPUT_DIR/reproduce_fig10.csv"
    local output_png="$OUTPUT_DIR/reproduce_fig10.png"

    run_app "reproduce_fig10" "$ROOT_DIR/data/input/reproduce_fig10.json" "$output_json"
    require_file "$output_csv"

    run_plot_script \
        "Rendering Figure 10" \
        "$ROOT_DIR/scripts/plot_fig10.py" \
        "$output_csv" \
        --no-title \
        -o "$output_png"
}

render_figure11() {
    local output_json="$OUTPUT_DIR/reproduce_fig11.json"
    local output_csv="$OUTPUT_DIR/reproduce_fig11.csv"
    local output_png="$OUTPUT_DIR/reproduce_fig11.png"

    run_app "reproduce_fig11" "$ROOT_DIR/data/input/reproduce_fig11.json" "$output_json"
    require_file "$output_csv"

    run_plot_script \
        "Rendering Figure 11" \
        "$ROOT_DIR/scripts/plot_fig11.py" \
        "$output_csv" \
        --no-title \
        -o "$output_png"
}

reproduce_outputs() {
    prepare_output_dir
    run_non_figure_apps
    render_fig6_panels
    render_figure7
    render_figure8
    render_figure10
    render_figure11
}

check_reproduction() {
    if [[ "$RUN_PIPELINE" == "1" ]]; then
        clean_all
        build_project
        reproduce_outputs
    fi

    local required_files=(
        "$OUTPUT_DIR/solve_single_guide.json"
        "$OUTPUT_DIR/solve_single_guide.csv"
        "$OUTPUT_DIR/solve_coupler.json"
        "$OUTPUT_DIR/solve_coupler.csv"
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

    require_dir "$FIG6_INPUT_DIR"

    shopt -s nullglob
    local input_files=("$FIG6_INPUT_DIR"/*.json)
    shopt -u nullglob

    [[ ${#input_files[@]} -gt 0 ]] || die "No JSON input files found in: $FIG6_INPUT_DIR"

    local input_file
    for input_file in "${input_files[@]}"; do
        local panel_name
        panel_name="$(basename "$input_file" .json)"

        required_files+=(
            "$FIG6_OUTPUT_DIR/${panel_name}.json"
            "$FIG6_OUTPUT_DIR/${panel_name}.csv"
            "$FIG6_OUTPUT_DIR/${panel_name}.png"
        )
    done

    local missing_count=0
    local file
    for file in "${required_files[@]}"; do
        if [[ -f "$file" ]]; then
            printf '[ok] %s\n' "$file"
        else
            printf '[missing] %s\n' "$file"
            ((missing_count += 1))
        fi
    done

    if [[ "$missing_count" -ne 0 ]]; then
        die "Reproduction check failed: $missing_count required artifact(s) missing."
    fi

    log "done" "Reproduction checklist passed."
}

usage() {
    cat <<EOF
Usage:
  $(basename "$0") <command>

Commands:
  clean       Remove build directory and generated output files
  build       Configure, build, and optionally run tests
  fig6        Build (unless SKIP_BUILD=1) and render Figure 6 panels
  fig7        Build (unless SKIP_BUILD=1) and render Figure 7
  fig8        Build (unless SKIP_BUILD=1) and render Figure 8
  fig10       Build (unless SKIP_BUILD=1) and render Figure 10
  fig11       Build (unless SKIP_BUILD=1) and render Figure 11
  reproduce   Build (unless SKIP_BUILD=1) and reproduce all configured outputs
  check       Verify the required reproduction artifacts
  full        Clean, build, reproduce, and verify everything
  help        Show this help

Environment variables:
  BUILD_DIR
  BUILD_TYPE
  BUILD_JOBS
  OUTPUT_DIR
  FIG6_INPUT_DIR
  FIG6_OUTPUT_DIR
  PYTHON_BIN
  RUN_TESTS=1
  RUN_PIPELINE=1
  SKIP_BUILD=1
  CLEAN_TRACKED_OUTPUT=1
  CTEST_BIN
EOF
}

build_if_needed() {
    if [[ "${SKIP_BUILD:-0}" != "1" ]]; then
        build_project
    fi
}

main() {
    local command="${1:-help}"

    case "$command" in
        clean)
            clean_all
            ;;
        build)
            build_project
            ;;
        fig6)
            build_if_needed
            render_fig6_panels
            ;;
        fig7)
            build_if_needed
            prepare_output_dir
            render_figure7
            ;;
        fig8)
            build_if_needed
            prepare_output_dir
            render_figure8
            ;;
        fig10)
            build_if_needed
            prepare_output_dir
            render_figure10
            ;;
        fig11)
            build_if_needed
            prepare_output_dir
            render_figure11
            ;;
        reproduce)
            build_if_needed
            reproduce_outputs
            ;;
        check)
            check_reproduction
            ;;
        full)
            clean_all
            build_project
            reproduce_outputs
            check_reproduction
            ;;
        help|-h|--help)
            usage
            ;;
        *)
            usage
            die "Unknown command: $command"
            ;;
    esac
}

main "$@"
