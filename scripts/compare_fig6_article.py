#!/usr/bin/env python3

"""Build a side-by-side comparison between the article scan and a generated Fig. 6 panel."""

from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image, ImageChops, ImageDraw


PANEL_SOURCES = {
    "SG-006d": ("docs/img/fig_06_c_d.png", "right"),
    "SG-006h": ("docs/img/fig_06_h_i.png", "left"),
    "SG-006i": ("docs/img/fig_06_h_i.png", "right"),
    "SG-006j": ("docs/img/fig_06_j_k.png", "left"),
    "SG-006k": ("docs/img/fig_06_j_k.png", "right"),
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Create a side-by-side comparison between an article panel and the current Fig. 6 reproduction."
    )
    parser.add_argument("panel_id", help="Panel identifier, e.g. SG-006h.")
    parser.add_argument(
        "plot_png",
        nargs="?",
        type=Path,
        help="Generated PNG from scripts/plot_fig6.py. If omitted or missing, a pending placeholder is used.",
    )
    parser.add_argument("-o", "--output", required=True, type=Path, help="Output PNG path.")
    return parser.parse_args()


def crop_article_panel(panel_id: str, repo_root: Path) -> Image.Image:
    try:
        relative_path, side = PANEL_SOURCES[panel_id]
    except KeyError as exc:
        raise SystemExit(f"Unsupported panel_id {panel_id!r}. Supported panels: {', '.join(sorted(PANEL_SOURCES))}.") from exc

    source = Image.open(repo_root / relative_path).convert("RGB")
    width, height = source.size
    midpoint = width // 2

    if side == "left":
        panel = source.crop((0, 0, midpoint, height))
    else:
        panel = source.crop((midpoint, 0, width, height))

    return trim_white_border(panel)


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


def build_pending_panel(reference_size: tuple[int, int], panel_id: str) -> Image.Image:
    width = max(520, reference_size[0])
    height = max(520, reference_size[1])
    canvas = Image.new("RGB", (width, height), "white")
    draw = ImageDraw.Draw(canvas)

    lines = [
        panel_id,
        "",
        "Reproducao numerica pendente",
        "",
        "Caso laminar-limite",
        "Fig. 6d / Fig. 6k",
        "",
        "TODO: implementar solver",
        "de lamina simetrica/assimetrica",
        "sem usar proxy de a -> infinito",
    ]

    y = 40
    for line in lines:
        draw.text((36, y), line, fill="black")
        y += 38 if line else 22

    draw.rectangle((18, 18, width - 18, height - 18), outline="black", width=2)
    return canvas


def build_canvas(article: Image.Image, plot: Image.Image, panel_id: str) -> Image.Image:
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

    draw.text((margin, margin), f"{panel_id} article scan", fill="black")
    draw.text((margin + article.width + gap, margin), f"{panel_id} current reproduction", fill="black")

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
    article = crop_article_panel(args.panel_id, repo_root)

    if args.plot_png and args.plot_png.exists():
        plot = trim_white_border(Image.open(args.plot_png).convert("RGB"))
    else:
        plot = build_pending_panel(article.size, args.panel_id)

    args.output.parent.mkdir(parents=True, exist_ok=True)
    comparison = build_canvas(article, plot, args.panel_id)
    comparison.save(args.output)
    print(f"Wrote comparison to {args.output}")


if __name__ == "__main__":
    main()
