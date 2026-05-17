#!/usr/bin/env python3

"""Compare root-method iteration counts in solve_single_guide."""

from __future__ import annotations

import argparse
import json
import subprocess
from dataclasses import dataclass
from pathlib import Path
from typing import Any

import matplotlib.pyplot as plt

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_SOLVER = ROOT / "build" / "bin" / "solve_single_guide"
DEFAULT_BISECTION_INPUT = ROOT / "data" / "input" / "test_root_bisection.json"
DEFAULT_SECANT_INPUT = ROOT / "data" / "input" / "test_root_secant.json"
DEFAULT_NEWTON_INPUT = ROOT / "data" / "input" / "test_root_newton.json"
DEFAULT_FALSE_POSITION_INPUT = ROOT / "data" / "input" / "test_root_false_position.json"
DEFAULT_OUTPUT = ROOT / "data" / "output" / "root_method_comparison.png"
DEFAULT_OUTPUT_DIR = ROOT / "data" / "output"

METHOD_COLORS = {
    "bisection": "#3766a6",
    "secant": "#c24a35",
    "newton": "#2ca02c",
    "false_position": "#9467bd",
}


@dataclass(frozen=True)
class RootResult:
    solver_algorithm: str
    b_over_A4: float
    iter_count_kx: int
    iter_count_ky: int
    status: str
    output_json: Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Run solve_single_guide with each root method and plot iteration counts."
    )
    parser.add_argument("--solver", type=Path, default=DEFAULT_SOLVER)
    parser.add_argument("--bisection-input", type=Path, default=DEFAULT_BISECTION_INPUT)
    parser.add_argument("--secant-input", type=Path, default=DEFAULT_SECANT_INPUT)
    parser.add_argument("--newton-input", type=Path, default=DEFAULT_NEWTON_INPUT)
    parser.add_argument(
        "--false-position-input", type=Path, default=DEFAULT_FALSE_POSITION_INPUT
    )
    parser.add_argument("-o", "--output", type=Path, default=DEFAULT_OUTPUT)
    return parser.parse_args()


def resolve_path(path: Path) -> Path:
    return path if path.is_absolute() else ROOT / path


def load_json(path: Path) -> dict[str, Any]:
    with path.open(encoding="utf-8") as handle:
        data = json.load(handle)

    if not isinstance(data, dict):
        raise ValueError(f"{path} must contain a JSON object.")

    return data


def run_case(solver: Path, input_path: Path, output_dir: Path) -> RootResult:
    input_data = load_json(input_path)
    solver_algorithm = str(input_data.get("solver_algorithm", "bisection"))
    output_json = output_dir / f"root_method_{solver_algorithm}.json"

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

    output_data = load_json(output_json)
    derived = output_data.get("derived")
    if not isinstance(derived, dict):
        raise ValueError(f"{output_json} is missing the derived object.")

    return RootResult(
        solver_algorithm=str(output_data["solver_algorithm"]),
        b_over_A4=float(derived["b_over_A4"]),
        iter_count_kx=int(output_data["iter_count_kx"]),
        iter_count_ky=int(output_data["iter_count_ky"]),
        status=str(output_data["status"]),
        output_json=output_json,
    )


def plot_results(results: list[RootResult], output_path: Path) -> None:
    plt.style.use("seaborn-v0_8-whitegrid")
    figure, axes = plt.subplots(1, 2, figsize=(10.0, 4.2), sharey=True)

    labels = [result.solver_algorithm for result in results]
    x_positions = list(range(len(results)))
    colors = [METHOD_COLORS.get(label, "#333333") for label in labels]

    axes[0].bar(
        x_positions,
        [result.iter_count_kx for result in results],
        color=colors,
        edgecolor="#222222",
        linewidth=0.6,
    )
    axes[1].bar(
        x_positions,
        [result.iter_count_ky for result in results],
        color=colors,
        edgecolor="#222222",
        linewidth=0.6,
    )

    axes[0].set_title(r"$k_x$ root")
    axes[1].set_title(r"$k_y$ root")

    for axis in axes:
        axis.set_xlabel("root method")
        axis.set_ylabel("iteration count")
        axis.set_xticks(x_positions)
        axis.set_xticklabels(labels, rotation=20, ha="right")
        axis.grid(True, which="major", color="#d0d0d0", linewidth=0.8)

    output_path.parent.mkdir(parents=True, exist_ok=True)
    figure.tight_layout()
    figure.savefig(output_path, dpi=200)
    plt.close(figure)


def print_summary(results: list[RootResult]) -> None:
    print("method           b/A4       iter_count_kx  iter_count_ky  status")
    print("---------------  --------  -------------  -------------  ------------")
    for result in results:
        print(
            f"{result.solver_algorithm:<15}  "
            f"{result.b_over_A4:8.5f}  "
            f"{result.iter_count_kx:13d}  "
            f"{result.iter_count_ky:13d}  "
            f"{result.status}"
        )


def main() -> int:
    args = parse_args()
    solver = resolve_path(args.solver)
    bisection_input = resolve_path(args.bisection_input)
    secant_input = resolve_path(args.secant_input)
    newton_input = resolve_path(args.newton_input)
    false_position_input = resolve_path(args.false_position_input)
    output_path = resolve_path(args.output)

    if not solver.is_file():
        raise SystemExit(f"solve_single_guide not found: {solver}")

    output_dir = DEFAULT_OUTPUT_DIR
    output_dir.mkdir(parents=True, exist_ok=True)

    results = [
        run_case(solver, bisection_input, output_dir),
        run_case(solver, secant_input, output_dir),
        run_case(solver, newton_input, output_dir),
        run_case(solver, false_position_input, output_dir),
    ]

    plot_results(results, output_path)

    print(f"Wrote plot to {output_path}")
    print_summary(results)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
