# Optional high-resolution explainers

The committed `examples/*-explainer.gif` files are 960 × 540 previews rendered
from these scenes. They are the README presentation assets; the C toolkit's
SVG exporters remain available for diagnostics, not for README presentation.

`toolkit_stories.py` is a Manim Community companion for rendering four
explainers as high-resolution MP4 video. It is deliberately outside the
CMake build and CI, so the portable C23 toolkit does not require Python,
FFmpeg, Pango, OpenGL, or a video renderer.

Install the prerequisites appropriate for your platform, then create an
isolated environment and render the committed GIF previews:

```bash
python3 -m venv .venv
. .venv/bin/activate
pip install -r animations/requirements.txt
MANIM="$PWD/.venv/bin/manim" ./animations/render.sh
```

The script invokes Manim with `-qh` (1080p/60fps), retains MP4 files below
`media/manim/`, and derives 960 × 540 GIF previews. `media/` is ignored because
video files are derived output. The binary, list, and sketch scenes use the
same operation sequences and values as the deterministic visualizer fixtures.
The renderer scene uses the same torus geometry and makes its sampling, depth,
and lighting pipeline visible at presentation resolution rather than enlarging
a terminal raster.
