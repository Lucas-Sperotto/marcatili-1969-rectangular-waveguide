#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import json
import math
import subprocess
from dataclasses import dataclass
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
DEFAULT_V_VALUES = [0.3, 0.7, 1.1, 1.5]


@dataclass(frozen=True)
class Mode:
    label: str
    family: str
    p: int
    q: int


@dataclass(frozen=True)
class Case:
    case_id: str
    n_core: float
    n_cladding: float
    modes: tuple[Mode, ...]


CASES = (
    Case(
        case_id="fig5a_rect_ex_low_contrast",
        n_core=1.05,
        n_cladding=1.0,
        modes=(
            Mode("Ex11", "E_x", 1, 1),
            Mode("Ex21", "E_x", 2, 1),
        ),
    ),
    Case(
        case_id="fig5b_rect_ey_low_contrast",
        n_core=1.05,
        n_cladding=1.0,
        modes=(
            Mode("Ey11", "E_y", 1, 1),
            Mode("Ey21", "E_y", 2, 1),
        ),
    ),
    Case(
        case_id="fig5c_rect_ex_ey_high_contrast",
        n_core=1.5,
        n_cladding=1.0,
        modes=(
            Mode("Ex11", "E_x", 1, 1),
            Mode("Ey11", "E_y", 1, 1),
        ),
    ),
)


FIELDNAMES = [
    "case_id",
    "curve_id",
    "x",
    "y",
    "x_quantity",
    "y_quantity",
    "mode_label",
    "field_kind",
    "status",
    "source",
    "solver_model",
    "p",
    "q",
    "v_number",
    "wavelength",
    "a",
    "b",
    "a_over_b",
    "n_core",
    "n_cladding",
    "kz",
    "source_status",
    "guided",
]


def parse_csv_floats(raw: str) -> list[float]:
    values = [float(item.strip()) for item in raw.split(",") if item.strip()]
    if not values:
        raise argparse.ArgumentTypeError("expected at least one comma-separated value")
    return values


def parse_csv_strings(raw: str) -> list[str]:
    values = [item.strip() for item in raw.split(",") if item.strip()]
    if not values:
        raise argparse.ArgumentTypeError("expected at least one comma-separated value")
    return values


def fmt(value: float) -> str:
    if not math.isfinite(value):
        return "nan"
    return f"{value:.12g}"


def as_float(value: object, default: float = math.nan) -> float:
    if value is None:
        return default
    try:
        return float(value)
    except (TypeError, ValueError):
        return default


def wavelength_from_v(v_number: float, t_m: float, n_core: float, n_cladding: float) -> float:
    contrast = n_core * n_core - n_cladding * n_cladding
    if contrast <= 0.0:
        raise ValueError("n_core must be greater than n_cladding")
    return 2.0 * t_m * math.sqrt(contrast) / v_number


def build_input(
    *,
    case: Case,
    mode: Mode,
    solver_model: str,
    v_number: float,
    t_m: float,
    a_over_b: float,
    csv_path: Path,
) -> dict[str, object]:
    semi_height = 0.5 * t_m
    semi_width = a_over_b * semi_height
    wavelength = wavelength_from_v(v_number, t_m, case.n_core, case.n_cladding)

    return {
        "case_id": f"{case.case_id}_{mode.label}_{solver_model}_v{v_number:g}",
        "article_target": (
            "Koshiba and Inoue 1992 Fig. 5 auxiliary Marcatili comparison; "
            "not an article-digitized reference."
        ),
        "csv_file": str(csv_path),
        "solver_model": solver_model,
        "mode_family": mode.family,
        "mode_indices": {"p": mode.p, "q": mode.q},
        "geometry": {
            "wavelength": wavelength,
            "a": semi_width,
            "b": semi_height,
        },
        "materials": {
            "n1": case.n_core,
            "n2": case.n_cladding,
            "n3": case.n_cladding,
            "n4": case.n_cladding,
            "n5": case.n_cladding,
        },
        "boundary_conditions": {
            "guide_limit": "c_infinity",
            "koshiba_fig5_mapping": "W=2t, core height=t, so Marcatili a/b=2 by default",
        },
    }


def run_solver(bin_path: Path, input_path: Path, output_json: Path) -> dict[str, object]:
    subprocess.run(
        [str(bin_path), str(input_path), str(output_json)],
        cwd=ROOT,
        check=True,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    return json.loads(output_json.read_text(encoding="utf-8"))


def row_from_result(
    *,
    case: Case,
    mode: Mode,
    solver_model: str,
    v_number: float,
    a_over_b: float,
    result: dict[str, object],
) -> dict[str, str]:
    derived = result.get("derived", {})
    geometry = result.get("geometry", {})

    source_status = str(result.get("status", "unknown"))
    guided = bool(result.get("guided", False))
    b_number = as_float(derived.get("kz_normalized_against_n4"))
    if guided and math.isfinite(b_number) and 0.0 <= b_number <= 1.0:
        status = "propagating"
    elif source_status == "below_cutoff":
        status = "below_cutoff"
    elif math.isfinite(b_number):
        status = "outside_guided_range"
    else:
        status = source_status

    curve_id = f"{mode.label}_marcatili_{solver_model}"
    return {
        "case_id": case.case_id,
        "curve_id": curve_id,
        "x": fmt(v_number),
        "y": fmt(b_number),
        "x_quantity": "v_number",
        "y_quantity": "b_number",
        "mode_label": mode.label,
        "field_kind": "E",
        "status": status,
        "source": "marcatili_1969_single_guide",
        "solver_model": solver_model,
        "p": str(mode.p),
        "q": str(mode.q),
        "v_number": fmt(v_number),
        "wavelength": fmt(as_float(geometry.get("wavelength"))),
        "a": fmt(as_float(geometry.get("a"))),
        "b": fmt(as_float(geometry.get("b"))),
        "a_over_b": fmt(a_over_b),
        "n_core": fmt(case.n_core),
        "n_cladding": fmt(case.n_cladding),
        "kz": fmt(as_float(derived.get("kz"))),
        "source_status": source_status,
        "guided": "1" if guided else "0",
    }


def write_csv(path: Path, rows: list[dict[str, str]]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=FIELDNAMES)
        writer.writeheader()
        writer.writerows(rows)


def main() -> int:
    parser = argparse.ArgumentParser(
        description=(
            "Run Marcatili single-guide auxiliary curves for Koshiba Fig. 5. "
            "The output is a computed comparison dataset, not digitized article data."
        )
    )
    parser.add_argument("--v-values", type=parse_csv_floats, default=DEFAULT_V_VALUES)
    parser.add_argument("--solver-models", type=parse_csv_strings, default=["exact", "closed_form"])
    parser.add_argument("--t-m", type=float, default=1.0)
    parser.add_argument(
        "--a-over-b",
        type=float,
        default=2.0,
        help="Core full-width/full-height ratio. Koshiba Fig. 5 uses W=2t, so default is 2.",
    )
    parser.add_argument("--output-dir", type=Path, default=ROOT / "data/output/koshiba_fig5")
    parser.add_argument("--work-dir", type=Path, default=ROOT / "build/koshiba_fig5_work")
    parser.add_argument("--bin", type=Path, default=ROOT / "build/bin/solve_single_guide")
    args = parser.parse_args()

    if args.t_m <= 0.0:
        raise SystemExit("--t-m must be positive")
    if args.a_over_b <= 0.0:
        raise SystemExit("--a-over-b must be positive")
    if not args.bin.exists():
        raise SystemExit(f"missing solver binary: {args.bin}. Run ./scripts/run.sh build first.")

    args.work_dir.mkdir(parents=True, exist_ok=True)

    rows: list[dict[str, str]] = []
    for case in CASES:
        for mode in case.modes:
            for solver_model in args.solver_models:
                if solver_model not in {"exact", "closed_form"}:
                    raise SystemExit("--solver-models supports exact and closed_form")
                for v_number in args.v_values:
                    safe_v = str(v_number).replace(".", "p").replace("-", "m")
                    stem = f"{case.case_id}_{mode.label}_{solver_model}_v{safe_v}"
                    csv_path = args.work_dir / f"{stem}.csv"
                    input_path = args.work_dir / f"{stem}.json"
                    output_json = args.work_dir / f"{stem}.out.json"
                    payload = build_input(
                        case=case,
                        mode=mode,
                        solver_model=solver_model,
                        v_number=v_number,
                        t_m=args.t_m,
                        a_over_b=args.a_over_b,
                        csv_path=csv_path,
                    )
                    input_path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
                    result = run_solver(args.bin, input_path, output_json)
                    rows.append(
                        row_from_result(
                            case=case,
                            mode=mode,
                            solver_model=solver_model,
                            v_number=v_number,
                            a_over_b=args.a_over_b,
                            result=result,
                        )
                    )

    combined = args.output_dir / "marcatili_fig5_reference.csv"
    write_csv(combined, rows)
    for case in CASES:
        write_csv(
            args.output_dir / f"marcatili_{case.case_id}.csv",
            [row for row in rows if row["case_id"] == case.case_id],
        )

    print(f"Wrote {combined}")
    for case in CASES:
        print(f"Wrote {args.output_dir / f'marcatili_{case.case_id}.csv'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
