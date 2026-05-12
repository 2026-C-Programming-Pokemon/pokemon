#!/usr/bin/env python3
"""Convert sprite images into block/ASCII text and C-friendly snippets."""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

from PIL import Image

INPUT_DIR = Path(__file__).parent / "input"
OUTPUT_DIR = Path(__file__).parent / "output"
GENERATED_DIR = Path(__file__).parent / "generated"

DEFAULT_ASCII_CHARS = " .:-=+*#%@"
DEFAULT_BLOCK_THRESHOLDS = (64, 128, 192)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Convert one sprite image into text art and a C string snippet.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument("image", type=Path, help="Input sprite image path (input/ folder checked first).")
    parser.add_argument("-w", "--width", type=int, default=32, help="Output width in characters.")
    parser.add_argument(
        "--mode",
        choices=("ascii", "blocks"),
        default="blocks",
        help="Rendering mode. blocks is recommended for pixel-art sprites.",
    )
    parser.add_argument("--chars", default=DEFAULT_ASCII_CHARS, help="ASCII characters from darkest to brightest.")
    parser.add_argument("--invert", action="store_true", help="Reverse brightness mapping.")
    parser.add_argument("--output", type=Path, help="Human-readable text output path.")
    parser.add_argument("--generated", type=Path, help="C-friendly generated snippet output path.")
    parser.add_argument("--c-name", help="C variable name for generated snippet.")
    return parser.parse_args()


def resolve_input(path: Path) -> Path:
    if path.exists():
        return path
    candidate = INPUT_DIR / path
    if candidate.exists():
        return candidate
    raise FileNotFoundError(f"input image not found: {path}")


def safe_c_identifier(name: str) -> str:
    identifier = re.sub(r"\W+", "_", name.strip().lower()).strip("_") or "sprite"
    if identifier[0].isdigit():
        identifier = f"sprite_{identifier}"
    return identifier


def load_image(path: Path) -> Image.Image:
    return Image.open(path).convert("RGBA")


def resize_for_ascii(image: Image.Image, width: int) -> Image.Image:
    if width <= 0:
        raise ValueError("width must be greater than 0")
    source_width, source_height = image.size
    if source_width <= 0 or source_height <= 0:
        raise ValueError("image has invalid dimensions")
    aspect_ratio = source_height / source_width
    height = max(1, round(width * aspect_ratio * 0.5))
    resample = getattr(Image, "Resampling", Image).LANCZOS
    return image.resize((width, height), resample)


def resize_for_blocks(image: Image.Image, width: int) -> Image.Image:
    if width <= 0:
        raise ValueError("width must be greater than 0")
    source_width, source_height = image.size
    if source_width <= 0 or source_height <= 0:
        raise ValueError("image has invalid dimensions")
    aspect_ratio = source_height / source_width
    height = max(2, round(width * aspect_ratio))
    if height % 2 == 1:
        height += 1
    resample = getattr(Image, "Resampling", Image).NEAREST
    return image.resize((width, height), resample)


def pixel_brightness(pixel: tuple[int, int, int, int]) -> float:
    red, green, blue, alpha = pixel
    if alpha < 20:
        return 255.0
    return (0.299 * red) + (0.587 * green) + (0.114 * blue)


def brightness_to_ascii(brightness: float, chars: str, invert: bool) -> str:
    palette = chars[::-1] if invert else chars
    index = round((brightness / 255.0) * (len(palette) - 1))
    return palette[index]


def brightness_to_block(brightness: float, invert: bool) -> str:
    low, mid, high = DEFAULT_BLOCK_THRESHOLDS
    if invert:
        if brightness < low:
            return " "
        if brightness < mid:
            return "░"
        if brightness < high:
            return "▒"
        return "█"
    if brightness < low:
        return "█"
    if brightness < mid:
        return "▓"
    if brightness < high:
        return "▒"
    return " "


def image_to_ascii_rows(image: Image.Image, width: int, chars: str, invert: bool) -> list[str]:
    if len(chars) < 2:
        raise ValueError("chars must contain at least two characters")
    resized = resize_for_ascii(image, width)
    rows: list[str] = []
    for y in range(resized.height):
        row = []
        for x in range(resized.width):
            brightness = pixel_brightness(resized.getpixel((x, y)))
            row.append(brightness_to_ascii(brightness, chars, invert))
        rows.append("".join(row).rstrip())
    return rows


def image_to_block_rows(image: Image.Image, width: int, invert: bool) -> list[str]:
    resized = resize_for_blocks(image, width)
    rows: list[str] = []
    for y in range(0, resized.height, 2):
        row = []
        for x in range(resized.width):
            top = pixel_brightness(resized.getpixel((x, y)))
            bottom = pixel_brightness(resized.getpixel((x, y + 1)))
            top_char = brightness_to_block(top, invert)
            bottom_char = brightness_to_block(bottom, invert)
            if top_char == bottom_char:
                row.append(top_char)
            elif top_char == " " and bottom_char != " ":
                row.append("▄")
            elif top_char != " " and bottom_char == " ":
                row.append("▀")
            else:
                row.append("█")
        rows.append("".join(row).rstrip())
    return rows


def render_rows(image: Image.Image, width: int, mode: str, chars: str, invert: bool) -> list[str]:
    if mode == "ascii":
        return image_to_ascii_rows(image, width, chars, invert)
    return image_to_block_rows(image, width, invert)


def write_text_output(path: Path, rows: list[str]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(rows) + "\n", encoding="utf-8")


def c_escape(line: str) -> str:
    return line.replace("\\", "\\\\").replace('"', '\\"')


def write_c_output(path: Path, rows: list[str], c_name: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    width = max((len(row) for row in rows), default=0)
    lines = [
        "/* Generated by tools/ascii_converter/convert_sprite.py */",
        f"static const char *{c_name}[] = {{",
    ]
    lines.extend(f'    "{c_escape(row)}",' for row in rows)
    lines.extend([
        "};",
        f"static const int {c_name}_width = {width};",
        f"static const int {c_name}_height = {len(rows)};",
    ])
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def default_output_paths(image_path: Path, mode: str) -> tuple[Path, Path]:
    suffix = f".{mode}"
    return OUTPUT_DIR / f"{image_path.stem}{suffix}.txt", GENERATED_DIR / f"{image_path.stem}{suffix}.inc"


def main() -> None:
    args = parse_args()
    try:
        image_path = resolve_input(args.image)
        text_output, c_output = default_output_paths(image_path, args.mode)
        text_output = args.output or text_output
        c_output = args.generated or c_output
        c_name = safe_c_identifier(args.c_name or f"{image_path.stem}_{args.mode}")

        image = load_image(image_path)
        rows = render_rows(image, args.width, args.mode, args.chars, args.invert)
        write_text_output(text_output, rows)
        write_c_output(c_output, rows, c_name)
    except (FileNotFoundError, OSError, ValueError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        raise SystemExit(1) from exc

    print(f"Mode: {args.mode}")
    print(f"Wrote text art: {text_output}")
    print(f"Wrote C snippet: {c_output}")


if __name__ == "__main__":
    main()
