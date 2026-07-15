"""High-resolution explainers composed from toolkit fixtures and output.

Run animations/render.sh first. BinaryStory and ListStory express their named
fixtures; SketchStory and RendererStory display frames emitted by C binaries.
"""

import json
from pathlib import Path

from manim import (  # type: ignore[import-not-found]
    BLUE,
    DOWN,
    GREEN,
    LEFT,
    ORANGE,
    RIGHT,
    UP,
    Arrow,
    Create,
    FadeIn,
    FadeOut,
    FadeTransform,
    ImageMobject,
    Rectangle,
    Scene,
    Text,
    Transform,
    VGroup,
    Write,
)


ASSET_DIRECTORY = Path(__file__).resolve().parent.parent / "media" / "manim-input"


class BinaryStory(Scene):
    """The exact binary fixture used by the C binary visualizer."""

    def construct(self):
        title = Text("Exact binary representation", font_size=42).to_edge(UP)
        value = Text("i8  −12", font_size=34).shift(3.8 * LEFT)
        bits = VGroup(
            *[
                VGroup(
                    Rectangle(width=0.68, height=0.9, color=ORANGE if index == 0 else BLUE),
                    Text(bit, font_size=32),
                )
                for index, bit in enumerate("11110100")
            ]
        ).arrange(RIGHT, buff=0.08)
        for cell in bits:
            cell[1].move_to(cell[0])
        bits.move_to([0.4, 0, 0])
        hexadecimal = Text("0xF4", font_size=42, color=GREEN).shift(4.0 * RIGHT)
        arrow_one = Arrow(value.get_right(), bits.get_left(), buff=0.25)
        arrow_two = Arrow(bits.get_right(), hexadecimal.get_left(), buff=0.25)
        note = Text("orange = sign bit", font_size=23, color=ORANGE).next_to(bits, DOWN)
        self.play(Write(title), FadeIn(value))
        self.play(Create(arrow_one))
        self.play(*[FadeIn(cell) for cell in bits], run_time=1.5)
        self.play(Write(note), Create(arrow_two), FadeIn(hexadecimal))
        self.wait(1)


def node(value: str, cursor: bool = False) -> VGroup:
    box = Rectangle(width=1.15, height=0.7, color=GREEN if cursor else BLUE)
    label = Text(value, font_size=28).move_to(box)
    marker = Text("cursor", font_size=18, color=GREEN).next_to(box, UP) if cursor else None
    return VGroup(box, label, *([] if marker is None else [marker]))


class ListStory(Scene):
    """The same four mutations executed by list-demo."""

    def construct(self):
        title = Text("Cursor-based linked-list operations", font_size=40).to_edge(UP)
        steps = [
            ("append 20", ["20"], 0),
            ("insert 10 before cursor", ["10", "20"], 0),
            ("insert 15 before cursor", ["10", "15", "20"], 1),
            ("erase 20; move cursor back", ["10", "15"], 1),
        ]
        caption = Text(steps[0][0], font_size=28).shift(2.4 * DOWN)
        group = VGroup(node("20", True)).move_to([0, 0, 0])
        self.play(Write(title), FadeIn(caption), FadeIn(group))
        for description, values, cursor in steps[1:]:
            next_nodes = VGroup(
                *[node(value, index == cursor) for index, value in enumerate(values)]
            )
            next_nodes.arrange(RIGHT, buff=0.9).move_to([0, 0, 0])
            arrows = VGroup(
                *[
                    Arrow(next_nodes[index].get_right(), next_nodes[index + 1].get_left(), buff=0.12)
                    for index in range(len(next_nodes) - 1)
                ]
            )
            next_caption = Text(description, font_size=28).move_to(caption)
            self.play(Transform(group, next_nodes), Transform(caption, next_caption), FadeIn(arrows))
            self.play(FadeOut(arrows), run_time=0.25)
        self.wait(1)


def sketch_frame(index: int) -> ImageMobject:
    path = ASSET_DIRECTORY / "sketch-frames" / f"frame-{index:02d}.png"
    return ImageMobject(str(path)).scale_to_fit_width(10.7)


class SketchStory(Scene):
    """Real canvas snapshots decoded from prefixes of examples/gallery.sk."""

    def construct(self):
        paths = sorted((ASSET_DIRECTORY / "sketch-frames").glob("frame-*.png"))
        if not paths:
            raise RuntimeError("run animations/render.sh before rendering SketchStory")
        title = Text("gallery.sk: real decoder checkpoints", font_size=38).to_edge(UP)
        subtitle = Text("each frame is emitted by toolkit-visualize", font_size=22, color=ORANGE)
        subtitle.next_to(title, DOWN, buff=0.18)
        label = Text(f"decoded checkpoint 1 / {len(paths)}", font_size=25).shift(2.95 * DOWN)
        current = sketch_frame(1)
        self.play(Write(title), FadeIn(subtitle), FadeIn(label), FadeIn(current))
        for index in range(2, len(paths) + 1):
            next_frame = sketch_frame(index)
            next_label = Text(f"decoded checkpoint {index} / {len(paths)}", font_size=25).move_to(label)
            self.play(FadeTransform(current, next_frame), Transform(label, next_label), run_time=0.65)
            current = next_frame
        self.wait(1)


def terminal_frame(index: int) -> ImageMobject:
    path = ASSET_DIRECTORY / "terminal-frames" / f"frame-{index:02d}.png"
    return ImageMobject(str(path)).scale_to_fit_width(12.25)


class RendererStory(Scene):
    """Actual ANSI frames emitted by donut-animate, not a substitute 3D model."""

    def construct(self):
        frames_path = ASSET_DIRECTORY / "donut-frames.json"
        frames = json.loads(frames_path.read_text(encoding="utf-8"))
        if not frames:
            raise RuntimeError("run animations/render.sh before rendering RendererStory")
        title = Text("donut-animate: real ANSI terminal frames", font_size=36).to_edge(UP)
        subtitle = Text("incremental sampling · inverse-depth buffer · luminance ramp", font_size=21, color=ORANGE)
        subtitle.next_to(title, DOWN, buff=0.18)
        command = Text("$ donut-animate --incremental 80 24", font="DejaVu Sans Mono", font_size=18, color=BLUE)
        command.shift(3.05 * DOWN)
        current = terminal_frame(1)
        self.play(Write(title), FadeIn(subtitle), FadeIn(command), FadeIn(current))
        for index in range(2, len(frames) + 1):
            next_frame = terminal_frame(index)
            self.play(FadeTransform(current, next_frame), run_time=0.10)
            current = next_frame
        self.wait(0.6)
