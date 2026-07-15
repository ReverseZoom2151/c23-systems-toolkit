# Optional high-resolution explainers

The committed `examples/*-story.svg` and GIF files are generated directly by
the C toolkit. They preserve terminal-scale output and are the canonical,
dependency-free documentation assets.

`toolkit_stories.py` is an optional Manim Community companion for rendering
three explainers as high-resolution MP4 video. It is deliberately outside the
CMake build and CI, so the portable C23 toolkit does not require Python,
FFmpeg, Pango, OpenGL, or a video renderer.

Install the prerequisites appropriate for your platform, then create an
isolated environment and render:

```bash
python3 -m venv .venv
. .venv/bin/activate
pip install -r animations/requirements.txt
manim -qh animations/toolkit_stories.py BinaryStory ListStory SketchStory
```

Use `-ql`, `-qm`, or `-qh` for low, medium, or high quality. Manim writes MP4
files below `media/videos/`; that directory is ignored because rendered video
is derived output. The scenes use the same operation sequences and values as
the repository's deterministic visualizer fixtures; they explain those real
states at presentation resolution rather than enlarging a terminal raster.
