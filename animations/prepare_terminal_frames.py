"""Turn donut-animate's captured ANSI stream into JSON without re-rendering it."""

import json
import re
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ANSI_ESCAPE = re.compile(r"\x1b\[[0-?]*[ -/]*[@-~]")


def write_terminal_images(frames: list[str], directory: Path) -> None:
    directory.mkdir(parents=True, exist_ok=True)
    try:
        font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", 29)
    except OSError:
        font = ImageFont.load_default()
    for index, frame in enumerate(frames, start=1):
        image = Image.new("RGB", (1920, 1080), "#0f172a")
        drawing = ImageDraw.Draw(image)
        drawing.multiline_text((175, 120), frame, font=font, fill="#86efac", spacing=5)
        image.save(directory / f"frame-{index:02d}.png")


def main() -> int:
    if len(sys.argv) != 4:
        print("usage: prepare_terminal_frames.py INPUT.ansi OUTPUT.json HEIGHT", file=sys.stderr)
        return 2
    source = Path(sys.argv[1]).read_text(encoding="utf-8")
    output = Path(sys.argv[2])
    height = int(sys.argv[3])
    frames = []
    for chunk in source.split("\x1b[H")[1:]:
        text = ANSI_ESCAPE.sub("", chunk)
        lines = text.splitlines()[:height]
        if len(lines) == height:
            frames.append("\n".join(lines))
    output.write_text(json.dumps(frames), encoding="utf-8")
    write_terminal_images(frames, output.parent / "terminal-frames")
    return 0 if frames else 1


if __name__ == "__main__":
    raise SystemExit(main())
