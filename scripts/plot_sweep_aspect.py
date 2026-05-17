#!/usr/bin/env python3

"""Plot kz vs a/b aspect-ratio sweep from reproduce_sweep_aspect CSV output."""

from __future__ import annotations

import argparse
import csv
from collections import defaultdict
from pathlib import Path

import matplotlib.pyplot as plt

import mode_colors


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Plot kz vs a/b from reproduce_sweep_aspect CSV output."
    )
    parser.add_argument(
        "csv_file",
        nargs="?",
        type=Path,
        default=Path("data/output/sweep_aspect.csv"),
        help="CSV produced by reproduce_sweep_aspect (default: data/output/sweep_aspect.csv).",
    )
    parser.add_argument(
        "-o",
        "--output",
        type=Path,
        help="Output image path. Defaults to the CSV path with .png extension.",
    )
    return parser.parse_args()


def load_sweep_data(
    csv_file: Path,
) -> dict[tuple[str, int, int], tuple[list[float], list[float]]]:
    if not csv_file.is_file():
        raise FileNotFoundError(f"CSV not found: {csv_file}")

    series: dict[tuple[str, int, int], tuple[list[float], list[float]]] = defaultdict(
        lambda: ([], [])
    )

    with csv_file.open(newline="", encoding="utf-8") as handle:
        reader = csv.DictReader(handle)
        required = {"a_over_b", "mode_family", "p", "q", "kz"}
        missing = required - set(reader.fieldnames or [])
        if missing:
            raise ValueError(f"CSV missing columns: {', '.join(sorted(missing))}")

        for row in reader:
            key = (row["mode_family"], int(row["p"]), int(row["q"]))
            xs, ys = series[key]
            xs.append(float(row["a_over_b"]))
            ys.append(float(row["kz"]))

    for xs, ys in series.values():
        order = sorted(range(len(xs)), key=lambda i: xs[i])
        xs[:] = [xs[i] for i in order]
        ys[:] = [ys[i] for i in order]

    return series


def mode_label(family: str, p: int, q: int) -> str:
    superscript = "y" if family == "E_y" else "x"
    return rf"$E^{{{superscript}}}_{{{p}{q}}}$"


def build_plot(
    series: dict[tuple[str, int, int], tuple[list[float], list[float]]],
    output_path: Path,
) -> None:
    plt.style.use("seaborn-v0_8-whitegrid")
    figure, axis = plt.subplots(figsize=(8.0, 5.5))

    for (family, p, q), (xs, ys) in sorted(series.items()):
        color = mode_colors.mode_color(p, q)
        lw = mode_colors.family_linewidth(family)
        label = mode_label(family, p, q)
        axis.plot(xs, ys, color=color, linewidth=lw, linestyle="-", label=label)

    axis.set_xlabel(r"$a/b$")
    axis.set_ylabel(r"$k_z$ (rad/m)")
    axis.legend(loc="lower right", framealpha=0.9)
    axis.grid(True, which="major", color="#d0d0d0", linewidth=0.8)

    output_path.parent.mkdir(parents=True, exist_ok=True)
    figure.tight_layout()
    figure.savefig(output_path, dpi=200)
    plt.close(figure)
    print(f"Wrote plot to {output_path}")


def main() -> int:
    args = parse_args()
    output_path = args.output if args.output else args.csv_file.with_suffix(".png")
    series = load_sweep_data(args.csv_file)
    if not series:
        raise SystemExit("No data found in CSV.")
    build_plot(series, output_path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
