#!/usr/bin/env python3

"""Plot closed-form error against the exact Figure 6 solver output."""

from __future__ import annotations

import argparse
from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd

import mode_colors

PROJECT_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_INPUT = PROJECT_ROOT / "data" / "output" / "fig6"
DEFAULT_OUTPUT = PROJECT_ROOT / "data" / "output" / "error_closed_form.png"

BASE_INDEX_COLUMNS = ["b_over_A4", "mode_family", "p", "q"]
OPTIONAL_ID_COLUMNS = ["panel_id", "variant_id", "curve_id"]
REQUIRED_COLUMNS = set(BASE_INDEX_COLUMNS + ["kz"])
SOLVER_COLUMN_CANDIDATES = ("solver", "solver_model")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Plot exact-vs-closed-form kz error from reproduce_fig6 CSV output."
    )
    parser.add_argument(
        "input",
        nargs="?",
        type=Path,
        default=DEFAULT_INPUT,
        help=(
            "CSV file or directory containing reproduce_fig6 CSV files. "
            "Defaults to data/output/fig6."
        ),
    )
    parser.add_argument(
        "-o",
        "--output",
        type=Path,
        default=DEFAULT_OUTPUT,
        help="Output image path. Defaults to data/output/error_closed_form.png.",
    )
    parser.add_argument(
        "--title",
        help="Optional figure title.",
    )
    return parser.parse_args()


def resolve_path(path: Path) -> Path:
    return path if path.is_absolute() else PROJECT_ROOT / path


def csv_paths(input_path: Path) -> list[Path]:
    if input_path.is_file():
        return [input_path]
    if input_path.is_dir():
        return sorted(input_path.glob("*.csv"))
    raise FileNotFoundError(f"Input CSV path not found: {input_path}")


def solver_column(frame: pd.DataFrame, path: Path) -> str:
    for candidate in SOLVER_COLUMN_CANDIDATES:
        if candidate in frame.columns:
            return candidate
    raise ValueError(
        f"{path} is missing solver column. Expected one of: "
        + ", ".join(SOLVER_COLUMN_CANDIDATES)
    )


def validate_columns(frame: pd.DataFrame, path: Path) -> str:
    missing = REQUIRED_COLUMNS.difference(frame.columns)
    if missing:
        missing_text = ", ".join(sorted(missing))
        raise ValueError(f"{path} is missing required columns: {missing_text}")

    return solver_column(frame, path)


def read_fig6_csvs(input_path: Path) -> pd.DataFrame:
    paths = csv_paths(input_path)
    if not paths:
        raise ValueError(f"No CSV files found in {input_path}")

    frames: list[pd.DataFrame] = []
    for path in paths:
        frame = pd.read_csv(path)
        solver_name = validate_columns(frame, path)
        if solver_name != "solver":
            frame = frame.rename(columns={solver_name: "solver"})

        frame["source_csv"] = path.stem
        frames.append(frame)

    return pd.concat(frames, ignore_index=True)


def index_columns(frame: pd.DataFrame) -> list[str]:
    columns = ["source_csv"]
    columns.extend(column for column in OPTIONAL_ID_COLUMNS if column in frame.columns)
    columns.extend(BASE_INDEX_COLUMNS)
    return columns


def compute_errors(frame: pd.DataFrame) -> pd.DataFrame:
    working = frame.copy()
    working["solver"] = working["solver"].astype(str)
    working["kz"] = pd.to_numeric(working["kz"], errors="coerce")
    working["b_over_A4"] = pd.to_numeric(working["b_over_A4"], errors="coerce")
    working["p"] = pd.to_numeric(working["p"], errors="raise").astype(int)
    working["q"] = pd.to_numeric(working["q"], errors="raise").astype(int)

    working = working[working["solver"].isin(["exact", "closed_form"])]
    if working.empty:
        raise ValueError("No rows with solver exact or closed_form were found.")

    pivot_index = index_columns(working)
    pivot = (
        working.pivot_table(
            index=pivot_index,
            columns="solver",
            values="kz",
            aggfunc="first",
        )
        .reset_index()
        .rename_axis(None, axis=1)
    )

    if "exact" not in pivot.columns or "closed_form" not in pivot.columns:
        raise ValueError("CSV data must contain both exact and closed_form solver rows.")

    errors = pivot.dropna(subset=["exact", "closed_form"]).copy()
    if errors.empty:
        raise ValueError("No exact/closed_form pairs share the same mode and b/A4 point.")

    errors["abs_error"] = (errors["exact"] - errors["closed_form"]).abs()
    errors["rel_error"] = errors["abs_error"] / errors["exact"]
    return errors.sort_values(index_columns(errors))


def mode_label(family: str, p: int, q: int) -> str:
    superscript = "y" if family == "E_y" else "x"
    return rf"$E^{{{superscript}}}_{{{p}{q}}}$"


def line_label(row: pd.Series, include_source: bool) -> str:
    label = mode_label(str(row["mode_family"]), int(row["p"]), int(row["q"]))
    parts: list[str] = []
    for column in ["panel_id", "variant_id"]:
        if column in row.index and str(row[column]) not in {"", "default", "nan"}:
            parts.append(str(row[column]))

    if include_source and "source_csv" in row.index:
        parts.insert(0, str(row["source_csv"]))

    if parts:
        return f"{label} ({', '.join(parts)})"
    return label


def mode_color(family: str, p: int, q: int) -> str:
    try:
        return mode_colors.get_mode_color(family, p, q)
    except AttributeError:
        return mode_colors.mode_color(p, q)


def plot_errors(errors: pd.DataFrame, output_path: Path, title: str | None) -> None:
    plt.style.use("seaborn-v0_8-whitegrid")
    figure, (abs_axis, rel_axis) = plt.subplots(2, 1, figsize=(9.0, 7.2), sharex=True)

    group_columns = [
        column
        for column in ["source_csv", "panel_id", "variant_id", "curve_id", "mode_family", "p", "q"]
        if column in errors.columns
    ]
    include_source = errors["source_csv"].nunique() > 1 if "source_csv" in errors.columns else False

    for _, rows in errors.groupby(group_columns, sort=True):
        rows = rows.sort_values("b_over_A4")
        first = rows.iloc[0]
        family = str(first["mode_family"])
        p = int(first["p"])
        q = int(first["q"])
        color = mode_color(family, p, q)
        label = line_label(first, include_source)

        abs_axis.plot(
            rows["b_over_A4"],
            rows["abs_error"],
            color=color,
            linewidth=1.7,
            label=label,
        )
        rel_axis.plot(
            rows["b_over_A4"],
            rows["rel_error"],
            color=color,
            linewidth=1.7,
            label=label,
        )

    abs_axis.set_ylabel(r"$|k_{z,\mathrm{exact}}-k_{z,\mathrm{closed}}|$")
    rel_axis.set_ylabel(r"$|k_{z,\mathrm{exact}}-k_{z,\mathrm{closed}}|/|k_{z,\mathrm{exact}}|$")
    rel_axis.set_xlabel(r"$b/A_4$")

    if title:
        abs_axis.set_title(title)

    for axis in (abs_axis, rel_axis):
        axis.grid(True, which="major", color="#d0d0d0", linewidth=0.8)

    handles, labels = abs_axis.get_legend_handles_labels()
    if handles:
        figure.legend(
            handles,
            labels,
            loc="center left",
            bbox_to_anchor=(1.0, 0.5),
            fontsize=8,
        )

    output_path.parent.mkdir(parents=True, exist_ok=True)
    figure.tight_layout()
    figure.savefig(output_path, dpi=200, bbox_inches="tight")
    plt.close(figure)


def main() -> int:
    args = parse_args()
    input_path = resolve_path(args.input)
    output_path = resolve_path(args.output)

    frame = read_fig6_csvs(input_path)
    errors = compute_errors(frame)
    plot_errors(errors, output_path, args.title)

    print(f"Wrote plot to {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
