#!/usr/bin/env python3

"""Compare the rectangular single-guide solver against the slab-limit solver.

The script builds temporary point-input JSON files from the first
data/input/reproduce_fig7.json case, runs the compiled executables in build/bin,
reads their CSV outputs, and prints a compact numerical comparison table.
"""

from __future__ import annotations

import argparse
import json
import math
import subprocess
import sys
import tempfile
from pathlib import Path
from typing import Any

import numpy as np
import pandas as pd

PROJECT_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_INPUT = PROJECT_ROOT / "data" / "input" / "reproduce_fig7.json"
DEFAULT_SINGLE_BIN = PROJECT_ROOT / "build" / "bin" / "solve_single_guide"
DEFAULT_SLAB_BIN = PROJECT_ROOT / "build" / "bin" / "solve_slab_guide"
DEFAULT_A_OVER_B = 100.0
DEFAULT_SOLVER_MODEL = "exact"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Verify the a/b -> infinity limit by comparing solve_single_guide "
            "at a_over_b=100 against solve_slab_guide."
        )
    )
    parser.add_argument(
        "--input",
        type=Path,
        default=DEFAULT_INPUT,
        help="Figure 7 input JSON. Defaults to data/input/reproduce_fig7.json.",
    )
    parser.add_argument(
        "--single-bin",
        type=Path,
        default=DEFAULT_SINGLE_BIN,
        help="Path to solve_single_guide. Defaults to build/bin/solve_single_guide.",
    )
    parser.add_argument(
        "--slab-bin",
        type=Path,
        default=DEFAULT_SLAB_BIN,
        help="Path to solve_slab_guide. Defaults to build/bin/solve_slab_guide.",
    )
    parser.add_argument(
        "--solver-model",
        choices=("exact", "closed_form"),
        default=DEFAULT_SOLVER_MODEL,
        help="Solver model to compare. Defaults to exact.",
    )
    parser.add_argument(
        "--a-over-b",
        type=float,
        default=DEFAULT_A_OVER_B,
        help="Aspect ratio used for solve_single_guide. Defaults to 100.0.",
    )
    parser.add_argument(
        "--b-over-a4",
        type=float,
        nargs="+",
        help=(
            "Optional b/A4 points to test. If omitted, the script uses the "
            "single b value from the first Figure 7 case."
        ),
    )
    parser.add_argument(
        "--keep-temp",
        action="store_true",
        help="Keep temporary JSON/CSV files and print their directory.",
    )
    return parser.parse_args()


def load_json(path: Path) -> Any:
    with path.open(encoding="utf-8") as handle:
        return json.load(handle)


def first_case(data: Any) -> dict[str, Any]:
    if isinstance(data, list) and data:
        case = data[0]
    elif isinstance(data, dict) and isinstance(data.get("cases"), list) and data["cases"]:
        case = data["cases"][0]
    elif isinstance(data, dict):
        case = data
    else:
        raise ValueError("input JSON does not contain a case object")

    if not isinstance(case, dict):
        raise ValueError("first case is not a JSON object")

    return case


def nested_value(case: dict[str, Any], dotted_key: str, flat_key: str) -> Any:
    current: Any = case
    for part in dotted_key.split("."):
        if not isinstance(current, dict) or part not in current:
            current = None
            break
        current = current[part]

    if current is not None:
        return current

    if flat_key in case:
        return case[flat_key]

    raise KeyError(f"missing required key: {dotted_key} or {flat_key}")


def numeric_value(case: dict[str, Any], dotted_key: str, flat_key: str) -> float:
    value = nested_value(case, dotted_key, flat_key)
    try:
        number = float(value)
    except (TypeError, ValueError) as exc:
        raise ValueError(f"{dotted_key} must be numeric, got {value!r}") from exc

    if not math.isfinite(number):
        raise ValueError(f"{dotted_key} must be finite, got {value!r}")

    return number


def compute_a4(wavelength: float, n1: float, n4: float) -> float:
    if not n1 > n4:
        raise ValueError("n1 must be greater than n4 to compute A4")
    return wavelength / (2.0 * math.sqrt(n1 * n1 - n4 * n4))


def parse_mode(mode: Any) -> tuple[str, int, int, str]:
    if isinstance(mode, str):
        parts = mode.split(":")
        if len(parts) != 3:
            raise ValueError(f"invalid mode string {mode!r}; expected family:p:q")
        family, p_text, q_text = parts
        p = int(p_text)
        q = int(q_text)
    elif isinstance(mode, dict):
        family = str(mode.get("mode_family", mode.get("family", "")))
        p = int(mode.get("p", mode.get("mode_p", 0)))
        q = int(mode.get("q", mode.get("mode_q", 0)))
    else:
        raise ValueError(f"unsupported mode entry {mode!r}")

    if family not in {"E_y", "E_x"} or p <= 0 or q <= 0:
        raise ValueError(f"invalid mode specification {mode!r}")

    return family, p, q, f"{family}:{p}:{q}"


def modes_from_case(case: dict[str, Any]) -> list[tuple[str, int, int, str]]:
    raw_modes = case.get("modes")
    if not isinstance(raw_modes, list) or not raw_modes:
        raise ValueError("first case must provide a non-empty modes array")
    return [parse_mode(mode) for mode in raw_modes]


def require_executable(path: Path, label: str) -> Path:
    resolved = path if path.is_absolute() else PROJECT_ROOT / path
    if not resolved.is_file():
        raise FileNotFoundError(f"{label} not found: {resolved}")
    if (resolved.stat().st_mode & 0o111) == 0:
        raise PermissionError(f"{label} is not executable: {resolved}")
    return resolved


def build_input_payload(
    *,
    source_case: dict[str, Any],
    mode_family: str,
    p: int,
    q: int,
    b: float,
    a_over_b: float,
    solver_model: str,
    solver_kind: str,
    wavelength: float,
    materials: dict[str, float],
) -> dict[str, Any]:
    source_case_id = str(source_case.get("case_id", "reproduce_fig7_first_case"))
    a = a_over_b * b

    return {
        "case_id": f"VERIFY-SLAB-{solver_kind}-{mode_family}-{p}-{q}",
        "article_target": (
            "Temporary slab-limit verification case derived from "
            f"{source_case_id}"
        ),
        "source_case_id": source_case_id,
        "solver_model": solver_model,
        "mode_family": mode_family,
        "mode_indices": {
            "p": p,
            "q": q,
        },
        "a_over_b": a_over_b,
        "geometry": {
            "wavelength": wavelength,
            "a": a,
            "b": b,
        },
        "materials": materials,
    }


def write_json(path: Path, payload: dict[str, Any]) -> None:
    path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")


def run_solver(binary: Path, input_json: Path, output_json: Path) -> Path:
    result = subprocess.run(
        [str(binary), str(input_json), str(output_json)],
        cwd=PROJECT_ROOT,
        text=True,
        capture_output=True,
        check=False,
    )

    if result.returncode != 0:
        raise RuntimeError(
            "\n".join(
                [
                    f"{binary.name} failed with exit code {result.returncode}",
                    f"input: {input_json}",
                    f"stdout: {result.stdout.strip()}",
                    f"stderr: {result.stderr.strip()}",
                ]
            )
        )

    output_csv = output_json.with_suffix(".csv")
    if not output_csv.is_file():
        raise FileNotFoundError(f"expected CSV output was not written: {output_csv}")

    return output_csv


def read_single_row(csv_path: Path) -> pd.Series:
    frame = pd.read_csv(csv_path)
    if len(frame.index) != 1:
        raise ValueError(f"expected exactly one CSV row in {csv_path}, got {len(frame.index)}")

    required_columns = {"ky", "kz", "b_over_A4", "status", "guided", "domain_valid"}
    missing = sorted(required_columns.difference(frame.columns))
    if missing:
        raise ValueError(f"{csv_path} is missing required columns: {', '.join(missing)}")

    return frame.iloc[0]


def relative_error(value: float, reference: float) -> float:
    if not np.isfinite(value) or not np.isfinite(reference):
        return np.nan
    if reference == 0.0:
        return 0.0 if value == 0.0 else np.inf
    return abs(value - reference) / abs(reference)


def validate_positive_finite(value: float, label: str) -> float:
    if not math.isfinite(value) or value <= 0.0:
        raise ValueError(f"{label} must be a finite positive number, got {value!r}")
    return value


def run_comparison(
    *,
    single_bin: Path,
    slab_bin: Path,
    temp_dir: Path,
    source_case: dict[str, Any],
    modes: list[tuple[str, int, int, str]],
    b_points: list[tuple[float, float]],
    a_over_b: float,
    solver_model: str,
    wavelength: float,
    materials: dict[str, float],
) -> pd.DataFrame:
    rows: list[dict[str, Any]] = []

    for point_index, (b_over_a4, b) in enumerate(b_points):
        for mode_family, p, q, mode_label in modes:
            common_payload_args = {
                "source_case": source_case,
                "mode_family": mode_family,
                "p": p,
                "q": q,
                "b": b,
                "a_over_b": a_over_b,
                "solver_model": solver_model,
                "wavelength": wavelength,
                "materials": materials,
            }

            stem = f"point{point_index:03d}_{mode_family}_{p}_{q}"
            single_input = temp_dir / f"{stem}_single.json"
            slab_input = temp_dir / f"{stem}_slab.json"
            single_output = temp_dir / f"{stem}_single.out.json"
            slab_output = temp_dir / f"{stem}_slab.out.json"

            write_json(
                single_input,
                build_input_payload(solver_kind="single", **common_payload_args),
            )
            write_json(
                slab_input,
                build_input_payload(solver_kind="slab", **common_payload_args),
            )

            single_row = read_single_row(run_solver(single_bin, single_input, single_output))
            slab_row = read_single_row(run_solver(slab_bin, slab_input, slab_output))

            ky_single = float(single_row["ky"])
            ky_slab = float(slab_row["ky"])
            kz_single = float(single_row["kz"])
            kz_slab = float(slab_row["kz"])

            rows.append(
                {
                    "mode": mode_label,
                    "b_over_A4": b_over_a4,
                    "ky_single_guide": ky_single,
                    "ky_slab": ky_slab,
                    "ky_relative_error": relative_error(ky_single, ky_slab),
                    "kz_single": kz_single,
                    "kz_slab": kz_slab,
                    "kz_relative_error": relative_error(kz_single, kz_slab),
                    "single_status": single_row["status"],
                    "slab_status": slab_row["status"],
                    "single_guided": int(single_row["guided"]),
                    "slab_guided": int(slab_row["guided"]),
                }
            )

    return pd.DataFrame(rows)


def format_float(value: Any) -> str:
    try:
        number = float(value)
    except (TypeError, ValueError):
        return str(value)

    if np.isnan(number):
        return "nan"
    if np.isposinf(number):
        return "inf"
    if np.isneginf(number):
        return "-inf"
    return f"{number:.8e}"


def print_table(table: pd.DataFrame) -> None:
    formatters = {
        "b_over_A4": format_float,
        "ky_single_guide": format_float,
        "ky_slab": format_float,
        "ky_relative_error": format_float,
        "kz_single": format_float,
        "kz_slab": format_float,
        "kz_relative_error": format_float,
    }

    with pd.option_context(
        "display.max_columns",
        None,
        "display.max_rows",
        None,
        "display.width",
        240,
    ):
        print(table.to_string(index=False, formatters=formatters))


def main() -> int:
    args = parse_args()

    try:
        input_path = args.input if args.input.is_absolute() else PROJECT_ROOT / args.input
        single_bin = require_executable(args.single_bin, "solve_single_guide")
        slab_bin = require_executable(args.slab_bin, "solve_slab_guide")

        if args.a_over_b <= 0.0 or not math.isfinite(args.a_over_b):
            raise ValueError("--a-over-b must be a finite positive number")

        source_case = first_case(load_json(input_path))
        wavelength = numeric_value(source_case, "geometry.wavelength", "wavelength")
        base_b = numeric_value(source_case, "geometry.b", "b")
        materials = {
            key: numeric_value(source_case, f"materials.{key}", key)
            for key in ("n1", "n2", "n3", "n4", "n5")
        }

        a4 = compute_a4(wavelength, materials["n1"], materials["n4"])
        if args.b_over_a4:
            b_over_a4_values = [
                validate_positive_finite(float(value), "--b-over-a4")
                for value in args.b_over_a4
            ]
            b_points = [(value, value * a4) for value in b_over_a4_values]
        else:
            b_points = [(base_b / a4, base_b)]

        modes = modes_from_case(source_case)

        if args.keep_temp:
            temp_dir = Path(tempfile.mkdtemp(prefix="marcatili_slab_limit_"))
            print(f"Temporary files: {temp_dir}")
            table = run_comparison(
                single_bin=single_bin,
                slab_bin=slab_bin,
                temp_dir=temp_dir,
                source_case=source_case,
                modes=modes,
                b_points=b_points,
                a_over_b=args.a_over_b,
                solver_model=args.solver_model,
                wavelength=wavelength,
                materials=materials,
            )
        else:
            with tempfile.TemporaryDirectory(prefix="marcatili_slab_limit_") as directory:
                table = run_comparison(
                    single_bin=single_bin,
                    slab_bin=slab_bin,
                    temp_dir=Path(directory),
                    source_case=source_case,
                    modes=modes,
                    b_points=b_points,
                    a_over_b=args.a_over_b,
                    solver_model=args.solver_model,
                    wavelength=wavelength,
                    materials=materials,
                )

        print_table(table)
    except Exception as exc:
        print(f"verify_slab_limit failed: {exc}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
