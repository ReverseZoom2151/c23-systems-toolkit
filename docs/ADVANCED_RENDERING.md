# Advanced rendering modes

The renderer has one readable reference path and several explicitly named
experiments. They are comparable through the same grayscale canvas, SVG/GIF
exporters, and deterministic test suite.

| Command | Technique | Purpose |
| --- | --- | --- |
| `donut-demo` | Floating point | Reference projection, depth, and lighting model |
| `donut-demo --incremental` | Normalized sine/cosine recurrence | Removes per-sample trigonometric calls |
| `donut-fixed` | Q30 fixed point + CORDIC | No-`libm` arithmetic comparison |
| `donut-fixed --fast` | Q30 parabola approximation | Faster, visibly approximate trigonometry |
| `donut-animate` | ANSI terminal control | Finite real-time terminal playback |
| `render-diagnostics depth` | Integer depth proxy | Depth-buffer intuition |
| `render-diagnostics normals` | Integer normal lighting | Light-direction intuition |
| `render-diagnostics dither` | 4 × 4 Bayer matrix | Quantisation and ordered dithering |
| `render-diagnostics raymarch` | Bounded integer SDF ray march | Hardware-inspired surface tracing |

The diagnostic sphere views are deliberately simpler than the torus renderer:
they isolate depth, lighting, and quantisation. The ray-march torus is a
bounded software simulation with a maximum of 256 steps per pixel; it is not a
claim of cycle-accurate hardware behavior.
