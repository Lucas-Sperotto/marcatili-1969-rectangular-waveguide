#!/usr/bin/env python3

"""Build a side-by-side comparison between the Figure 11 scan and the current reproduction."""

from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image, ImageChops, ImageDraw


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Create a side-by-side comparison between the Figure 11 article scan and the current reproduction."
    )
    parser.add_argument(
        "plot_png",
        type=Path,
        help="Generated PNG from scripts/plot_fig11.py.",
    )
    parser.add_argument("-o", "--output", required=True, type=Path, help="Output PNG path.")
    return parser.parse_args()


def trim_white_border(image: Image.Image) -> Image.Image:
    background = Image.new(image.mode, image.size, "white")
    diff = ImageChops.difference(image, background)
    bbox = diff.getbbox()
    if bbox is None:
        return image
    return image.crop(bbox)


def fit_height(image: Image.Image, height: int) -> Image.Image:
    width = max(1, round(image.width * height / image.height))
    return image.resize((width, height), Image.Resampling.LANCZOS)


def build_canvas(article: Image.Image, plot: Image.Image) -> Image.Image:
    target_height = max(article.height, plot.height)
    article = fit_height(article, target_height)
    plot = fit_height(plot, target_height)

    margin = 32
    gap = 24
    header = 40
    footer = 16
    width = article.width + plot.width + gap + 2 * margin
    height = target_height + header + footer + 2 * margin

    canvas = Image.new("RGB", (width, height), "white")
    draw = ImageDraw.Draw(canvas)

    draw.text((margin, margin), "Figure 11 article scan", fill="black")
    draw.text((margin + article.width + gap, margin), "Figure 11 current reproduction", fill="black")

    y0 = margin + header
    canvas.paste(article, (margin, y0))
    canvas.paste(plot, (margin + article.width + gap, y0))

    draw.rectangle((margin - 1, y0 - 1, margin + article.width, y0 + article.height), outline="black", width=1)
    plot_x = margin + article.width + gap
    draw.rectangle((plot_x - 1, y0 - 1, plot_x + plot.width, y0 + plot.height), outline="black", width=1)
    return canvas


def main() -> None:
    args = parse_args()
    repo_root = Path(__file__).resolve().parent.parent
    article = trim_white_border(Image.open(repo_root / "docs/img/fig_11.png").convert("RGB"))
    plot = trim_white_border(Image.open(args.plot_png).convert("RGB"))

    args.output.parent.mkdir(parents=True, exist_ok=True)
    comparison = build_canvas(article, plot)
    comparison.save(args.output)
    print(f"Wrote comparison to {args.output}")


if __name__ == "__main__":
    main()
