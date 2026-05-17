#!/usr/bin/env python3

"""Compare Figure 10 baseline, a/A5 = 1.6, and Eq. (20) hypotheses."""

from __future__ import annotations

import argparse
import csv
import math
from collections import defaultdict
from dataclasses import dataclass
from pathlib import Path

import matplotlib.image as mpimg
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.lines import Line2D

from mode_colors import SOLVER_LINESTYLE, sweep_color

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_BASELINE_CSV = ROOT / "data" / "output" / "reproduce_fig10.csv"
DEFAULT_TEST16_CSV = ROOT / "data" / "output" / "reproduce_fig10_test16.csv"
DEFAULT_EQ20_CSV = ROOT / "data" / "output" / "reproduce_fig10_eq20.csv"
DEFAULT_ARTICLE_IMAGE = ROOT / "docs" / "img" / "fig_10.png"
DEFAULT_OUTPUT = ROOT / "data" / "output" / "reproduce_fig10_compare_test16_eq20.png"

REQUIRED_COLUMNS = {
    "curve_label",
    "solver_model",
    "a_over_A5",
    "c_over_a",
    "normalized_coupling",
}

# Pixel bounds of the plotted frame in docs/img/fig_10.png, calibrated from the scan.
# Coordinates use the image convention: x grows rightward and y grows downward.
ARTICLE_FRAME_LEFT = 86
ARTICLE_FRAME_RIGHT = 438
ARTICLE_FRAME_TOP = 15
ARTICLE_FRAME_BOTTOM = 385

X_MIN = 0.0
X_MAX = 3.0
Y_MIN = 1.0e-4
Y_MAX = 10.0


@dataclass(frozen=True)
class CurvePoint:
    dataset: str
    curve_label: str
    solver_model: str
    a_over_A5: float
    c_over_a: float
    normalized_coupling: float


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Overlay the current Figure 10 CSV, the a/A5 = 1.6 alternative, "
            "and the Eq. (20) alternative with reference dots extracted from "
            "docs/img/fig_10.png."
        )
    )
    parser.add_argument("--baseline-csv", type=Path, default=DEFAULT_BASELINE_CSV)
    parser.add_argument("--test16-csv", type=Path, default=DEFAULT_TEST16_CSV)
    parser.add_argument("--eq20-csv", type=Path, default=DEFAULT_EQ20_CSV)
    parser.add_argument("--article-image", type=Path, default=DEFAULT_ARTICLE_IMAGE)
    parser.add_argument("-o", "--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument(
        "--solver-model",
        choices=["exact", "closed_form", "all"],
        default="all",
        help="Solver model to plot. The default keeps both Figure 10 line styles.",
    )
    parser.add_argument(
        "--max-article-dots",
        type=int,
        default=3500,
        help="Maximum number of scan pixels rendered as reference dots.",
    )
    return parser.parse_args()


def resolve_path(path: Path) -> Path:
    return path if path.is_absolute() else ROOT / path


def validate_columns(fieldnames: list[str] | None, path: Path) -> None:
    if not fieldnames:
        raise ValueError(f"{path} is empty or missing a header row.")

    missing = REQUIRED_COLUMNS.difference(fieldnames)
    if missing:
        missing_text = ", ".join(sorted(missing))
        raise ValueError(f"{path} is missing required columns: {missing_text}")


def load_points(path: Path, dataset: str, solver_filter: str) -> list[CurvePoint]:
    if not path.is_file():
        raise FileNotFoundError(f"CSV not found: {path}")

    points: list[CurvePoint] = []
    with path.open(newline="", encoding="utf-8") as handle:
        reader = csv.DictReader(handle)
        validate_columns(reader.fieldnames, path)

        for row in reader:
            solver_model = row["solver_model"]
            if solver_filter != "all" and solver_model != solver_filter:
                continue

            y_value = float(row["normalized_coupling"])
            if math.isnan(y_value) or y_value <= 0.0:
                continue

            points.append(
                CurvePoint(
                    dataset=dataset,
                    curve_label=row["curve_label"],
                    solver_model=solver_model,
                    a_over_A5=float(row["a_over_A5"]),
                    c_over_a=float(row["c_over_a"]),
                    normalized_coupling=y_value,
                )
            )

    return points


def grouped_curves(points: list[CurvePoint]) -> dict[tuple[str, str], list[CurvePoint]]:
    grouped: dict[tuple[str, str], list[CurvePoint]] = defaultdict(list)
    for point in points:
        grouped[(point.curve_label, point.solver_model)].append(point)

    for rows in grouped.values():
        rows.sort(key=lambda point: point.c_over_a)

    return grouped


def is_highlight_curve(point: CurvePoint) -> bool:
    return (
        (point.dataset == "baseline" and math.isclose(point.a_over_A5, 1.0))
        or (point.dataset == "test16" and math.isclose(point.a_over_A5, 1.6))
        or point.dataset == "eq20"
    )


def plot_dataset(axis: plt.Axes, points: list[CurvePoint], dataset: str) -> None:
    grouped = grouped_curves(points)
    labels = sorted({rows[0].curve_label for rows in grouped.values()}, key=float)
    label_to_color = {label: sweep_color(index) for index, label in enumerate(labels)}

    for rows in sorted(grouped.values(), key=lambda values: values[0].a_over_A5):
        first = rows[0]
        highlight = is_highlight_curve(first)

        if dataset == "baseline":
            color = "#3766a6" if highlight else "#4f5965"
            alpha = 0.98 if highlight else 0.22
            linewidth = 2.2 if highlight else 1.1
            zorder = 4 if highlight else 2
        elif dataset == "test16":
            color = "#c24a35" if highlight else label_to_color[first.curve_label]
            alpha = 0.98 if highlight else 0.22
            linewidth = 2.4 if highlight else 1.1
            zorder = 5 if highlight else 3
        else:
            color = "#6f49a8"
            alpha = 0.56
            linewidth = 1.7
            zorder = 6

        axis.semilogy(
            [point.c_over_a for point in rows],
            [point.normalized_coupling for point in rows],
            color=color,
            linestyle=SOLVER_LINESTYLE.get(first.solver_model, "-"),
            linewidth=linewidth,
            alpha=alpha,
            zorder=zorder,
        )


def article_pixel_dots(
    image_path: Path,
    max_dots: int,
) -> tuple[np.ndarray, np.ndarray]:
    if not image_path.is_file():
        raise FileNotFoundError(f"Article image not found: {image_path}")

    image = mpimg.imread(image_path)
    rgb = image[:, :, :3]
    gray = rgb.mean(axis=2)

    crop = gray[
        ARTICLE_FRAME_TOP : ARTICLE_FRAME_BOTTOM + 1,
        ARTICLE_FRAME_LEFT : ARTICLE_FRAME_RIGHT + 1,
    ]
    dark = crop < 0.42

    y_pixels, x_pixels = np.nonzero(dark)
    x_fraction = x_pixels / (ARTICLE_FRAME_RIGHT - ARTICLE_FRAME_LEFT)
    y_fraction = y_pixels / (ARTICLE_FRAME_BOTTOM - ARTICLE_FRAME_TOP)

    x_values = X_MIN + x_fraction * (X_MAX - X_MIN)
    log_y = math.log10(Y_MAX) + y_fraction * (
        math.log10(Y_MIN) - math.log10(Y_MAX)
    )
    y_values = 10.0 ** log_y

    # Remove most frame/grid pixels so the remaining dots emphasize printed curves.
    keep = np.ones_like(x_values, dtype=bool)
    for x_grid in np.arange(X_MIN, X_MAX + 0.001, 0.5):
        keep &= np.abs(x_values - x_grid) > 0.025

    for log_grid in np.arange(math.log10(Y_MIN), math.log10(Y_MAX) + 0.001, 1.0):
        keep &= np.abs(log_y - log_grid) > 0.075

    # The upper-right inset is part of the article figure, but it is not a data curve.
    keep &= ~((x_values > 1.55) & (log_y > -0.55))
    keep &= log_y <= 0.0

    x_values = x_values[keep]
    y_values = y_values[keep]

    if max_dots > 0 and len(x_values) > max_dots:
        stride = math.ceil(len(x_values) / max_dots)
        x_values = x_values[::stride]
        y_values = y_values[::stride]

    return x_values, y_values


def build_plot(
    baseline_points: list[CurvePoint],
    test16_points: list[CurvePoint],
    eq20_points: list[CurvePoint],
    article_x: np.ndarray,
    article_y: np.ndarray,
    output_path: Path,
) -> None:
    plt.style.use("seaborn-v0_8-whitegrid")
    figure, axis = plt.subplots(figsize=(8.4, 8.4))
    axis.set_box_aspect(1.0)

    axis.scatter(
        article_x,
        article_y,
        s=4,
        color="#111111",
        alpha=0.18,
        linewidths=0,
        label="scan pixels from fig_10.png",
        zorder=1,
    )

    plot_dataset(axis, baseline_points, "baseline")
    plot_dataset(axis, test16_points, "test16")
    plot_dataset(axis, eq20_points, "eq20")

    axis.set_xlim(X_MIN, X_MAX)
    axis.set_ylim(Y_MIN, Y_MAX)
    axis.set_xticks([index / 2 for index in range(7)])
    axis.set_xlabel(r"$c/a$")
    axis.set_ylabel(
        r"$|K|a/\left(\left[1-\left(n_5/n_1\right)^2\right]^{1/2}k_z\right)$"
    )
    axis.grid(True, which="major", color="#c8c8c8", linewidth=0.8)
    axis.grid(True, which="minor", color="#e6e6e6", linewidth=0.5, alpha=0.6)

    legend_handles = [
        Line2D(
            [0],
            [0],
            marker="o",
            linestyle="",
            color="#111111",
            alpha=0.28,
            markersize=5,
            label="Fig. 10 scan dots",
        ),
        Line2D(
            [0],
            [0],
            color="#3766a6",
            linestyle="-",
            linewidth=2.2,
            label="baseline a/A5 = 1.0",
        ),
        Line2D(
            [0],
            [0],
            color="#c24a35",
            linestyle="-",
            linewidth=2.4,
            label="alternative a/A5 = 1.6",
        ),
        Line2D(
            [0],
            [0],
            color="#6f49a8",
            linestyle="-",
            linewidth=1.9,
            label="alternative Eq. (20)",
        ),
        Line2D(
            [0],
            [0],
            color="black",
            linestyle=SOLVER_LINESTYLE.get("exact", "-"),
            linewidth=1.6,
            label="exact",
        ),
        Line2D(
            [0],
            [0],
            color="black",
            linestyle=SOLVER_LINESTYLE.get("closed_form", "--"),
            linewidth=1.6,
            label="closed_form",
        ),
    ]
    axis.legend(handles=legend_handles, loc="upper right", frameon=True, framealpha=0.95)

    output_path.parent.mkdir(parents=True, exist_ok=True)
    figure.tight_layout()
    figure.savefig(output_path, dpi=220)
    plt.close(figure)


def main() -> int:
    args = parse_args()

    baseline_csv = resolve_path(args.baseline_csv)
    test16_csv = resolve_path(args.test16_csv)
    eq20_csv = resolve_path(args.eq20_csv)
    article_image = resolve_path(args.article_image)
    output_path = resolve_path(args.output)

    baseline_points = load_points(baseline_csv, "baseline", args.solver_model)
    test16_points = load_points(test16_csv, "test16", args.solver_model)
    eq20_points = load_points(eq20_csv, "eq20", args.solver_model)
    article_x, article_y = article_pixel_dots(article_image, args.max_article_dots)

    if not baseline_points:
        raise SystemExit(f"No plottable rows found in {baseline_csv}")
    if not test16_points:
        raise SystemExit(f"No plottable rows found in {test16_csv}")
    if not eq20_points:
        raise SystemExit(f"No plottable rows found in {eq20_csv}")
    if len(article_x) == 0:
        raise SystemExit(f"No scan dots extracted from {article_image}")

    build_plot(
        baseline_points,
        test16_points,
        eq20_points,
        article_x,
        article_y,
        output_path,
    )

    print(f"Wrote comparison plot to {output_path}")
    print(f"Article scan dots: {len(article_x)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
