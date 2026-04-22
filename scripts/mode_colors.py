"""Shared visual encoding for waveguide modes across all plot scripts.

Color is fixed by (p, q) mode order — same mode always gets the same color
regardless of which figure it appears in. Line style encodes the solver model;
line width encodes the mode family.
"""

from __future__ import annotations

# Fixed color per (p, q) pair — tab10 palette, deterministic assignment.
MODE_COLORS: dict[tuple[int, int], str] = {
    (1, 1): "#1f77b4",  # blue
    (2, 1): "#ff7f0e",  # orange
    (1, 2): "#2ca02c",  # green
    (2, 2): "#d62728",  # red
    (3, 1): "#9467bd",  # purple
    (1, 3): "#8c564b",  # brown
    (2, 3): "#e377c2",  # pink
    (3, 2): "#7f7f7f",  # gray
    (3, 3): "#bcbd22",  # yellow-green
    (4, 1): "#17becf",  # teal
    (1, 4): "#aec7e8",  # light blue
    (4, 4): "#ffbb78",  # light orange
}

# Fallback for modes not in the table.
FALLBACK_COLOR = "#333333"

# Solver model → line style.
SOLVER_LINESTYLE: dict[str, str] = {
    "exact": "-",
    "closed_form": "--",
}

# Mode family → line width.
FAMILY_LINEWIDTH: dict[str, float] = {
    "E_y": 2.0,
    "E_x": 1.4,
}

# Sequential colors for coupler parameter sweeps (a/A5 or n1/n5 families).
# 7 distinct values — same order as the a/A5 sweep [0.5, 0.75, 1, 1.5, 2, 3, 4].
SWEEP_COLORS: list[str] = [
    "#1f77b4",  # blue      (0.5)
    "#ff7f0e",  # orange    (0.75)
    "#2ca02c",  # green     (1)
    "#d62728",  # red       (1.5)
    "#9467bd",  # purple    (2)
    "#8c564b",  # brown     (3)
    "#e377c2",  # pink      (4)
]


def mode_color(p: int, q: int) -> str:
    return MODE_COLORS.get((p, q), FALLBACK_COLOR)


def solver_linestyle(solver_model: str) -> str:
    return SOLVER_LINESTYLE.get(solver_model, "-")


def family_linewidth(mode_family: str) -> float:
    return FAMILY_LINEWIDTH.get(mode_family, 1.6)


def sweep_color(index: int) -> str:
    return SWEEP_COLORS[index % len(SWEEP_COLORS)]
