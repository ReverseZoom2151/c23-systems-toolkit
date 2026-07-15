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

## Advanced modes

The floating-point renderer remains the readable baseline; `--incremental`
adds normalized sine/cosine recurrence for the sampled torus angles.

- `donut-fixed` is an independent Q30 renderer. It has a CORDIC backend and a
  faster parabola-based fixed-point approximation, both with a deterministic
  depth buffer and no `libm` dependency.
- `donut-animate` writes a bounded ANSI animation, hides and restores the
  cursor, and accepts finite frame and FPS controls. It never defaults to an
  unbounded loop.
- `render-diagnostics` provides depth, normal-lighting, ordered-dither, and
  bounded ray-march views. They are deterministic software diagnostics inspired
  by hardware constraints—not a claim of hardware emulation.

The toolkit still deliberately excludes original-code obfuscation and physical
VGA/ASIC timing. Those do not strengthen its reusable command-line APIs.

## Sources of the design

- [How donut.c works](https://www.a1k0n.net/2011/07/20/donut-math.html): torus
  parameterisation, projection, depth buffering, and lighting.
- [Optimising donut.c](https://www.a1k0n.net/2021/01/13/optimizing-donut.html):
  incremental rotation and the fixed-point direction.
- [Tiny Tapeout Donut](https://www.a1k0n.net/2025/01/10/tiny-tapeout-donut.html):
  a useful demonstration of how the same geometry changes under hardware
  constraints.
