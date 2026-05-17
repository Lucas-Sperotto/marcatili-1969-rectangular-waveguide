#!/usr/bin/env python3

"""Run and plot degeneracy checks for symmetric and perturbed square guides."""

from __future__ import annotations

import argparse
import csv
import json
import math
import subprocess
import sys
import tempfile
from dataclasses import dataclass
from pathlib import Path
from typing import Any

import matplotlib.pyplot as plt

from mode_colors import family_linewidth, mode_color

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_SOLVER = ROOT / "build" / "bin" / "solve_single_guide"
DEFAULT_SYMMETRIC_INPUT = ROOT / "data" / "input" / "test_degeneracy.json"
DEFAULT_BROKEN_INPUT = ROOT / "data" / "input" / "test_symmetry_break.json"
DEFAULT_OUTPUT = ROOT / "data" / "output" / "degeneracy_test.png"


@dataclass(frozen=True)
class ModeSpec:
    family: str
    p: int
    q: int


@dataclass(frozen=True)
class Sample:
    case_id: str
    b_over_A4: float
    mode: ModeSpec
    kz: float


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Compare E_y:1:2 and E_x:2:1 degeneracy with solve_single_guide."
    )
    parser.add_argument("--solver", type=Path, default=DEFAULT_SOLVER)
    parser.add_argument("--symmetric-input", type=Path, default=DEFAULT_SYMMETRIC_INPUT)
    parser.add_argument("--broken-input", type=Path, default=DEFAULT_BROKEN_INPUT)
    parser.add_argument("-o", "--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--keep-temp", action="store_true")
    return parser.parse_args()


def resolve_path(path: Path) -> Path:
    return path if path.is_absolute() else ROOT / path


def load_case(path: Path) -> dict[str, Any]:
    with path.open(encoding="utf-8") as handle:
        data = json.load(handle)

    if not isinstance(data, dict):
        raise ValueError(f"{path} must contain a JSON object.")

    return data


def parse_mode(mode_text: str) -> ModeSpec:
    parts = mode_text.split(":")
    if len(parts) != 3:
        raise ValueError(f"Invalid mode specification {mode_text!r}; expected family:p:q.")

    family, p_text, q_text = parts
    return ModeSpec(family=family, p=int(p_text), q=int(q_text))


def compute_A4(case: dict[str, Any]) -> float:
    wavelength = float(case["wavelength"])
    n1 = float(case["n1"])
    n4 = float(case["n4"])

    if not n1 > n4:
        raise ValueError("Expected n1 > n4 so A4 is real.")

    return wavelength / (2.0 * math.sqrt(n1 * n1 - n4 * n4))


def point_input(case: dict[str, Any], mode: ModeSpec, b_over_A4: float) -> dict[str, Any]:
    A4 = compute_A4(case)
    b = b_over_A4 * A4
    a = float(case["a_over_b"]) * b

    return {
        "case_id": f"{case['case_id']}_{mode.family}_{mode.p}_{mode.q}_{b_over_A4:g}",
        "article_target": case.get("article_target", ""),
        "solver_model": case.get("solver_model", "exact"),
        "mode_family": mode.family,
        "mode_indices": {
            "p": mode.p,
            "q": mode.q,
        },
        "geometry": {
            "wavelength": float(case["wavelength"]),
            "a": a,
            "b": b,
        },
        "materials": {
            "n1": float(case["n1"]),
            "n2": float(case["n2"]),
            "n3": float(case["n3"]),
            "n4": float(case["n4"]),
            "n5": float(case["n5"]),
        },
    }


def read_csv_row(csv_path: Path) -> dict[str, str]:
    with csv_path.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle))

    if len(rows) != 1:
        raise ValueError(f"Expected one CSV row in {csv_path}, got {len(rows)}.")

    return rows[0]


def run_point(
    solver: Path,
    temp_dir: Path,
    case: dict[str, Any],
    mode: ModeSpec,
    b_over_A4: float,
) -> Sample:
    stem = f"{case['case_id']}_{mode.family}_{mode.p}_{mode.q}_{b_over_A4:g}"
    input_path = temp_dir / f"{stem}.json"
    output_json = temp_dir / f"{stem}.out.json"
    output_csv = output_json.with_suffix(".csv")

    input_path.write_text(
        json.dumps(point_input(case, mode, b_over_A4), indent=2) + "\n",
        encoding="utf-8",
    )

    result = subprocess.run(
        [str(solver), str(input_path), str(output_json)],
        cwd=ROOT,
        text=True,
        capture_output=True,
        check=False,
    )

    if result.returncode != 0:
        raise RuntimeError(
            "\n".join(
                [
                    f"solve_single_guide failed with exit code {result.returncode}",
                    f"input: {input_path}",
                    f"stdout: {result.stdout.strip()}",
                    f"stderr: {result.stderr.strip()}",
                ]
            )
        )

    row = read_csv_row(output_csv)
    return Sample(
        case_id=str(case["case_id"]),
        b_over_A4=b_over_A4,
        mode=mode,
        kz=float(row["kz"]),
    )


def run_case(solver: Path, temp_dir: Path, case: dict[str, Any]) -> list[Sample]:
    modes = [parse_mode(mode_text) for mode_text in case["modes"]]
    b_values = [float(value) for value in case["b_over_A4_list"]]

    samples: list[Sample] = []
    for b_over_A4 in b_values:
        for mode in modes:
            samples.append(run_point(solver, temp_dir, case, mode, b_over_A4))

    return samples


def mode_label(mode: ModeSpec) -> str:
    superscript = "y" if mode.family == "E_y" else "x"
    return rf"$E^{{{superscript}}}_{{{mode.p}{mode.q}}}$"


def grouped_by_mode(samples: list[Sample]) -> dict[ModeSpec, list[Sample]]:
    grouped: dict[ModeSpec, list[Sample]] = {}
    for sample in samples:
        grouped.setdefault(sample.mode, []).append(sample)

    for rows in grouped.values():
        rows.sort(key=lambda sample: sample.b_over_A4)

    return grouped


def plot_panel(axis: plt.Axes, samples: list[Sample], title: str) -> None:
    for mode, rows in grouped_by_mode(samples).items():
        linestyle = "-" if mode.family == "E_y" else "--"
        axis.plot(
            [sample.b_over_A4 for sample in rows],
            [sample.kz for sample in rows],
            color=mode_color(mode.p, mode.q),
            linewidth=family_linewidth(mode.family),
            linestyle=linestyle,
            marker="o",
            markersize=4,
            label=mode_label(mode),
        )

    axis.set_title(title)
    axis.set_xlabel(r"$b/A_4$")
    axis.set_ylabel(r"$k_z$ (rad/m)")
    axis.grid(True, which="major", color="#d0d0d0", linewidth=0.8)
    axis.legend()


def max_pair_delta(samples: list[Sample]) -> float:
    grouped = grouped_by_mode(samples)
    if len(grouped) != 2:
        return math.nan

    first, second = grouped.values()
    deltas = [
        abs(left.kz - right.kz)
        for left, right in zip(first, second)
        if left.b_over_A4 == right.b_over_A4
    ]
    return max(deltas) if deltas else math.nan


def build_plot(
    symmetric_samples: list[Sample],
    broken_samples: list[Sample],
    output_path: Path,
) -> None:
    plt.style.use("seaborn-v0_8-whitegrid")
    figure, axes = plt.subplots(1, 2, figsize=(12.0, 5.0), sharey=True)

    plot_panel(axes[0], symmetric_samples, "Symmetric guide")
    plot_panel(axes[1], broken_samples, "Broken symmetry")

    output_path.parent.mkdir(parents=True, exist_ok=True)
    figure.tight_layout()
    figure.savefig(output_path, dpi=200)
    plt.close(figure)


def main() -> int:
    args = parse_args()
    solver = resolve_path(args.solver)
    symmetric_input = resolve_path(args.symmetric_input)
    broken_input = resolve_path(args.broken_input)
    output_path = resolve_path(args.output)

    if not solver.is_file():
        print(f"solve_single_guide not found: {solver}", file=sys.stderr)
        return 1

    symmetric_case = load_case(symmetric_input)
    broken_case = load_case(broken_input)

    if args.keep_temp:
        temp_dir = Path(tempfile.mkdtemp(prefix="marcatili_degeneracy_"))
        print(f"Temporary files: {temp_dir}")
        symmetric_samples = run_case(solver, temp_dir, symmetric_case)
        broken_samples = run_case(solver, temp_dir, broken_case)
    else:
        with tempfile.TemporaryDirectory(prefix="marcatili_degeneracy_") as directory:
            temp_dir = Path(directory)
            symmetric_samples = run_case(solver, temp_dir, symmetric_case)
            broken_samples = run_case(solver, temp_dir, broken_case)

    build_plot(symmetric_samples, broken_samples, output_path)

    print(f"Wrote plot to {output_path}")
    print(f"Max |delta kz| symmetric: {max_pair_delta(symmetric_samples):.6e}")
    print(f"Max |delta kz| broken: {max_pair_delta(broken_samples):.6e}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
