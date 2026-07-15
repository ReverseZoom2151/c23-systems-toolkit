"""Manim Community scenes for high-resolution toolkit explainers.

The values and operation order mirror the deterministic C visualizer fixtures.
Run with: manim -qh animations/toolkit_stories.py BinaryStory ListStory SketchStory RendererStory
"""

import numpy as np

from manim import (  # type: ignore[import-not-found]
    BLUE,
    BLUE_D,
    BLUE_E,
    DEGREES,
    GREEN,
    ORANGE,
    DOWN,
    LEFT,
    RIGHT,
    UP,
    WHITE,
    Arrow,
    Create,
    FadeIn,
    FadeOut,
    Integer,
    Rectangle,
    Scene,
    Surface,
    Text,
    ThreeDScene,
    TAU,
    Transform,
    VGroup,
    Write,
)


class BinaryStory(Scene):
    def construct(self):
        title = Text("Exact binary representation", font_size=42).to_edge(UP)
        value = Text("i8  −12", font_size=34).shift(3.8 * LEFT)
        bits = VGroup(
            *[
                VGroup(
                    Rectangle(width=0.68, height=0.9, color=ORANGE if i == 0 else BLUE),
                    Text(bit, font_size=32),
                )
                for i, bit in enumerate("11110100")
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
            next_nodes = VGroup(*[node(value, index == cursor) for index, value in enumerate(values)])
            next_nodes.arrange(RIGHT, buff=0.9).move_to([0, 0, 0])
            arrows = VGroup(*[Arrow(next_nodes[i].get_right(), next_nodes[i + 1].get_left(), buff=0.12)
                              for i in range(len(next_nodes) - 1)])
            next_caption = Text(description, font_size=28).move_to(caption)
            self.play(Transform(group, next_nodes), Transform(caption, next_caption), FadeIn(arrows))
            self.play(FadeOut(arrows), run_time=0.25)
        self.wait(1)


class SketchStory(Scene):
    def construct(self):
        title = Text("Compact sketch stream: drawing checkpoints", font_size=38).to_edge(UP)
        byte_label = Text("draw at byte 6", font_size=28).shift(2.6 * DOWN)
        grid = VGroup(*[Rectangle(width=0.23, height=0.23, color=BLUE, fill_opacity=0)
                        for _ in range(32 * 12)])
        grid.arrange_in_grid(rows=12, cols=32, buff=0).scale(0.9)
        self.play(Write(title), FadeIn(byte_label), Create(grid))
        selected = [0, 1, 2, 33, 34, 65, 66, 97, 98, 129]
        for checkpoint, cell_index in enumerate(selected, start=1):
            grid[cell_index].set_fill(BLUE, opacity=0.95)
            next_label = Text(f"draw checkpoint {checkpoint}", font_size=28).move_to(byte_label)
            self.play(Transform(byte_label, next_label), FadeIn(grid[cell_index]), run_time=0.25)
        self.wait(1)


class RendererStory(ThreeDScene):
    def construct(self):
        title = Text("Torus renderer: sample → depth-test → light", font_size=34).to_edge(UP)
        subtitle = Text("same geometry, presentation-scale animation", font_size=22, color=ORANGE)
        subtitle.next_to(title, DOWN, buff=0.2)
        self.add_fixed_in_frame_mobjects(title, subtitle)

        major_radius = 2.25
        minor_radius = 0.82
        torus = Surface(
            lambda u, v: np.array(
                [
                    (major_radius + minor_radius * np.cos(v)) * np.cos(u),
                    (major_radius + minor_radius * np.cos(v)) * np.sin(u),
                    minor_radius * np.sin(v),
                ]
            ),
            u_range=[0, TAU],
            v_range=[0, TAU],
            resolution=(48, 24),
            checkerboard_colors=[BLUE_D, BLUE_E],
            fill_opacity=0.95,
        )
        self.set_camera_orientation(phi=68 * DEGREES, theta=-42 * DEGREES, zoom=0.9)
        self.play(Create(torus), run_time=2)
        self.begin_ambient_camera_rotation(rate=0.32)
        self.wait(4)
        self.stop_ambient_camera_rotation()
