#!/usr/bin/env python3

"""Compare five educational zero-finding methods on the Zeros.c reference case."""

from __future__ import annotations

import math
from dataclasses import dataclass
from pathlib import Path
from typing import Callable

import matplotlib

matplotlib.use("Agg")

import matplotlib.pyplot as plt
import numpy as np


ROOT = Path(__file__).resolve().parents[1]
DEFAULT_OUTPUT = ROOT / "data" / "output" / "zeros_reference_comparison.png"


@dataclass(frozen=True)
class MethodResult:
    name: str
    root: float
    iterations: int
    residual: float


def f(x: float) -> float:
    return x**3 + 4.0 * x**2 - 10.0


def df(x: float) -> float:
    return 3.0 * x**2 + 8.0 * x


def g(x: float) -> float:
    return 0.5 * math.sqrt(10.0 - x**3)


def bisection(
    function: Callable[[float], float],
    a: float,
    b: float,
    tol: float,
    max_iter: int,
) -> tuple[float, int]:
    fa = function(a)
    fb = function(b)
    if fa * fb > 0.0:
        raise ValueError("bisection requires a sign change on [a, b].")

    p = a
    for iteration in range(1, max_iter + 1):
        p = a + (b - a) / 2.0
        fp = function(p)

        if fp == 0.0 or (b - a) / 2.0 < tol:
            return p, iteration

        if fa * fp > 0.0:
            a = p
            fa = fp
        else:
            b = p
            fb = fp

    return p, max_iter


def fixed_point(
    iteration_function: Callable[[float], float],
    x0: float,
    tol: float,
    max_iter: int,
) -> tuple[float, int]:
    for iteration in range(1, max_iter + 1):
        p = iteration_function(x0)
        if abs(p - x0) < tol:
            return p, iteration
        x0 = p

    return x0, max_iter


def newton(
    function: Callable[[float], float],
    derivative: Callable[[float], float],
    x0: float,
    tol: float,
    max_iter: int,
) -> tuple[float, int]:
    for iteration in range(1, max_iter + 1):
        derivative_value = derivative(x0)
        if abs(derivative_value) < 1.0e-15:
            raise ZeroDivisionError("newton encountered a degenerate derivative.")

        p = x0 - function(x0) / derivative_value
        if abs(p - x0) < tol:
            return p, iteration
        x0 = p

    return x0, max_iter


def secant(
    function: Callable[[float], float],
    x0: float,
    x1: float,
    tol: float,
    max_iter: int,
) -> tuple[float, int]:
    q0 = function(x0)
    q1 = function(x1)

    for iteration in range(2, max_iter + 1):
        denominator = q1 - q0
        if abs(denominator) < 1.0e-15:
            raise ZeroDivisionError("secant encountered equal function values.")

        p = x1 - q1 * (x1 - x0) / denominator
        if abs(p - x1) < tol:
            return p, iteration

        x0 = x1
        q0 = q1
        x1 = p
        q1 = function(p)

    return x1, max_iter


def false_position(
    function: Callable[[float], float],
    a: float,
    b: float,
    tol: float,
    max_iter: int,
) -> tuple[float, int]:
    fa = function(a)
    fb = function(b)
    if fa * fb > 0.0:
        raise ValueError("false_position requires a sign change on [a, b].")

    previous_p: float | None = None
    p = a
    for iteration in range(1, max_iter + 1):
        denominator = fb - fa
        if abs(denominator) < 1.0e-15:
            raise ZeroDivisionError("false_position encountered equal function values.")

        p = b - fb * (b - a) / denominator
        fp = function(p)

        if fp == 0.0:
            return p, iteration
        if previous_p is not None and abs(p - previous_p) < tol:
            return p, iteration

        if fa * fp > 0.0:
            a = p
            fa = fp
        else:
            b = p
            fb = fp

        previous_p = p

    return p, max_iter


def run_methods() -> list[MethodResult]:
    a = 1.0
    b = 2.0
    p0 = 1.0
    p1 = 2.0
    tol = 1.0e-5
    max_iter = 10_000

    runs = [
        ("Bisection", bisection(f, a, b, tol, max_iter)),
        ("Fixed point", fixed_point(g, p0, tol, max_iter)),
        ("Newton", newton(f, df, p0, tol, max_iter)),
        ("Secant", secant(f, p0, p1, tol, max_iter)),
        ("False position", false_position(f, a, b, tol, max_iter)),
    ]

    return [
        MethodResult(
            name=name,
            root=root,
            iterations=iterations,
            residual=abs(f(root)),
        )
        for name, (root, iterations) in runs
    ]


def print_table(results: list[MethodResult]) -> None:
    print("Method         | Root            | Iterations | |f(root)|")
    print("--------------|-----------------|------------|----------")
    for result in results:
        print(
            f"{result.name:<14}| "
            f"{result.root:15.11f} | "
            f"{result.iterations:10d} | "
            f"{result.residual:8.2e}"
        )


def plot_iterations(results: list[MethodResult], output_path: Path) -> None:
    plt.style.use("seaborn-v0_8-whitegrid")

    names = [result.name for result in results]
    iterations = np.array([result.iterations for result in results], dtype=float)
    y_positions = np.arange(len(results))
    colors = ["#3766a6", "#c24a35", "#2ca02c", "#9467bd", "#8c564b"]

    figure, axis = plt.subplots(figsize=(8.0, 4.6))
    axis.barh(y_positions, iterations, color=colors, edgecolor="#222222", linewidth=0.6)
    axis.set_yticks(y_positions)
    axis.set_yticklabels(names)
    axis.invert_yaxis()
    axis.set_xlabel("iterations")
    axis.set_title(r"Zeros reference: $f(x)=x^3+4x^2-10$")
    axis.grid(True, axis="x", color="#d0d0d0", linewidth=0.8)

    max_iterations = max(result.iterations for result in results)
    for y_position, result in zip(y_positions, results):
        axis.text(
            result.iterations + max_iterations * 0.02,
            y_position,
            str(result.iterations),
            va="center",
            fontsize=9,
        )

    axis.set_xlim(0, max_iterations * 1.18)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    figure.tight_layout()
    figure.savefig(output_path, dpi=200)
    plt.close(figure)


def main() -> int:
    results = run_methods()
    print_table(results)
    plot_iterations(results, DEFAULT_OUTPUT)
    print(f"\nWrote plot to {DEFAULT_OUTPUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
