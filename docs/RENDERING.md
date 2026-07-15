# Terminal rendering

`donut-demo` is a small, deterministic renderer built around the same grayscale
canvas used by the sketch decoder. It is inspired by Andy Sloane's explanations
of the classic terminal donut, but is intentionally written as readable C23
with validation, tests, finite output, and export support.

## Frame pipeline

For every frame, the renderer:

1. Samples two angles on a torus with major radius `2` and minor radius `1`.
2. Rotates those samples around two axes using the supplied frame angles.
3. Projects them onto the canvas using perspective division.
4. Compares inverse depth at each projected pixel, keeping only the nearest
   surface sample.
5. Computes a surface-lighting value and maps it to the twelve-character
   terminal ramp `.,-~:;=!*#$@`.

The canvas retains those shades as grayscale pixels. That makes one rendering
available as terminal text, pixel-faithful SVG, or a looping GIF without
duplicating scene logic or introducing a graphics dependency.

## What belongs here

The reusable ideas are sampled geometry, perspective projection, depth
buffering, lighting, and deterministic frame generation. They complement the
existing bytecode sketch renderer: sketches construct a canvas from compact
instructions; `donut-demo` constructs one from a 3D surface.

## What is deliberately excluded

This toolkit does not copy the original obfuscation, turn the renderer into an
unbounded terminal loop, or pretend a hardware implementation is a small C
feature. The floating-point renderer remains the readable baseline;
`--incremental` adds normalized sine/cosine recurrence for the sampled torus
angles. Fixed-point arithmetic, CORDIC, and FPGA/ASIC-style constraints remain
valuable follow-on experiments only once they can be exposed as independently
tested modes with comparable output.

## Sources of the design

- [How donut.c works](https://www.a1k0n.net/2011/07/20/donut-math.html): torus
  parameterisation, projection, depth buffering, and lighting.
- [Optimising donut.c](https://www.a1k0n.net/2021/01/13/optimizing-donut.html):
  incremental rotation and the fixed-point direction.
- [Tiny Tapeout Donut](https://www.a1k0n.net/2025/01/10/tiny-tapeout-donut.html):
  a useful demonstration of how the same geometry changes under hardware
  constraints.
