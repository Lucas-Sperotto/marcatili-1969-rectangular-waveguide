#!/usr/bin/env python3

"""Migrate Figure 6 JSON inputs to the standardized case-array schema.

This one-shot script only scans data/input/fig6/*.json.  Legacy files with a
single JSON object at the root are rewritten as arrays with one object, while
already migrated arrays are left untouched unless their per-case metadata needs
normalization.
"""

from __future__ import annotations

import argparse
import json
import shutil
import sys
from pathlib import Path
from typing import Any

PROJECT_ROOT = Path(__file__).resolve().parents[1]
FIG6_INPUT_DIR = PROJECT_ROOT / "data" / "input" / "fig6"
DEFAULT_SOLVER_MODELS = ["exact", "closed_form"]
LEADING_FIELDS = ("case_id", "article_target")


class MigrationError(RuntimeError):
    """Raised when an input file cannot be safely migrated."""


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Migrate data/input/fig6 JSON inputs to arrays of case objects."
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Report planned changes without writing files or backups.",
    )
    return parser.parse_args()


def dump_json(data: Any) -> str:
    return json.dumps(data, indent=2, ensure_ascii=False) + "\n"


def load_json(path: Path) -> Any:
    try:
        with path.open(encoding="utf-8") as handle:
            return json.load(handle)
    except json.JSONDecodeError as exc:
        raise MigrationError(f"invalid JSON: {exc}") from exc


def normalize_solver_models(value: Any) -> tuple[list[str], bool]:
    if not isinstance(value, list):
        return list(DEFAULT_SOLVER_MODELS), True

    normalized: list[str] = ["exact"]
    for model in value:
        if not isinstance(model, str) or model == "exact" or model in normalized:
            continue
        normalized.append(model)

    return normalized, normalized != value


def fallback_article_target(case_id: str) -> str:
    return f"TODO: verify article_target for {case_id}"


def normalize_case(raw_case: Any, path: Path, index: int) -> tuple[dict[str, Any], list[str]]:
    if not isinstance(raw_case, dict):
        raise MigrationError(f"case {index} is not an object")

    reasons: list[str] = []
    original_keys = list(raw_case.keys())
    if original_keys[:2] != list(LEADING_FIELDS):
        reasons.append(f"case {index}: moved case_id and article_target to the front")

    case_id_value = raw_case.get("case_id")
    if case_id_value is None:
        case_id_value = raw_case.get("panel_id", path.stem)
        reasons.append(f"case {index}: filled missing case_id from panel_id or filename")

    article_target_value = raw_case.get("article_target")
    if article_target_value is None:
        article_target_value = fallback_article_target(str(case_id_value))
        reasons.append(f"case {index}: filled missing article_target with TODO")

    normalized_case: dict[str, Any] = {
        "case_id": case_id_value,
        "article_target": article_target_value,
    }

    if "solver_models" not in raw_case:
        normalized_case["solver_models"] = list(DEFAULT_SOLVER_MODELS)
        reasons.append(f"case {index}: added default solver_models")

    for key, value in raw_case.items():
        if key in LEADING_FIELDS:
            continue

        if key == "solver_models":
            normalized_solver_models, changed = normalize_solver_models(value)
            normalized_case[key] = normalized_solver_models
            if changed:
                reasons.append(f"case {index}: normalized solver_models")
        else:
            normalized_case[key] = value

    return normalized_case, reasons


def normalize_file(path: Path) -> tuple[list[dict[str, Any]], list[str]]:
    data = load_json(path)
    reasons: list[str] = []

    if isinstance(data, dict):
        cases = [data]
        reasons.append("converted root object to single-case array")
    elif isinstance(data, list):
        cases = data
    else:
        raise MigrationError("root JSON value is neither an object nor an array")

    normalized_cases: list[dict[str, Any]] = []
    for index, raw_case in enumerate(cases):
        normalized_case, case_reasons = normalize_case(raw_case, path, index)
        normalized_cases.append(normalized_case)
        reasons.extend(case_reasons)

    return normalized_cases, reasons


def backup_path_for(path: Path) -> Path:
    return path.with_name(f"{path.name}.bak")


def write_migrated_file(path: Path, data: list[dict[str, Any]]) -> tuple[Path, bool]:
    backup_path = backup_path_for(path)
    backup_created = False

    if not backup_path.exists():
        shutil.copy2(path, backup_path)
        backup_created = True

    tmp_path = path.with_name(f"{path.name}.tmp")
    try:
        tmp_path.write_text(dump_json(data), encoding="utf-8")
        tmp_path.replace(path)
    finally:
        if tmp_path.exists():
            tmp_path.unlink()

    return backup_path, backup_created


def print_changed_summary(changed: list[tuple[Path, Path | None, bool, list[str]]]) -> None:
    if not changed:
        print("No files changed.")
        return

    print("Changed files:")
    for path, backup_path, backup_created, reasons in changed:
        backup_text = "dry-run, no backup written"
        if backup_path is not None:
            backup_status = "created" if backup_created else "already existed"
            backup_text = f"backup {backup_status}: {backup_path.relative_to(PROJECT_ROOT)}"
        reason_text = "; ".join(reasons)
        print(f"- {path.relative_to(PROJECT_ROOT)} ({backup_text})")
        print(f"  {reason_text}")


def main() -> int:
    args = parse_args()

    if not FIG6_INPUT_DIR.is_dir():
        print(f"Input directory not found: {FIG6_INPUT_DIR}", file=sys.stderr)
        return 1

    json_paths = sorted(FIG6_INPUT_DIR.glob("*.json"))
    changed: list[tuple[Path, Path | None, bool, list[str]]] = []
    errors: list[tuple[Path, str]] = []

    for path in json_paths:
        try:
            normalized_data, reasons = normalize_file(path)
        except MigrationError as exc:
            errors.append((path, str(exc)))
            continue

        if not reasons:
            continue

        backup_path: Path | None = None
        backup_created = False
        if not args.dry_run:
            backup_path, backup_created = write_migrated_file(path, normalized_data)

        changed.append((path, backup_path, backup_created, reasons))

    print(f"Scanned {len(json_paths)} JSON file(s) in {FIG6_INPUT_DIR.relative_to(PROJECT_ROOT)}.")
    print_changed_summary(changed)

    if errors:
        print("Errors:", file=sys.stderr)
        for path, message in errors:
            print(f"- {path.relative_to(PROJECT_ROOT)}: {message}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
