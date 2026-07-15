#!/usr/bin/env bash
set -euo pipefail

# Render the README explainers at 1080p/60fps, then make 960px GIF previews
# that GitHub can display inline. Set MANIM=/path/to/manim for a virtual env.

readonly manim_command="${MANIM:-manim}"
readonly media_directory="media/manim"

"${manim_command}" -qh --media_dir "${media_directory}" \
  animations/toolkit_stories.py BinaryStory ListStory SketchStory RendererStory

for story in BinaryStory ListStory SketchStory RendererStory; do
  case "${story}" in
    BinaryStory) output="examples/binary-explainer.gif" ;;
    ListStory) output="examples/list-explainer.gif" ;;
    SketchStory) output="examples/sketch-explainer.gif" ;;
    RendererStory) output="examples/renderer-explainer.gif" ;;
  esac

  movie="${media_directory}/videos/toolkit_stories/1080p60/${story}.mp4"
  palette="${media_directory}/${story}-palette.png"
  ffmpeg -y -i "${movie}" \
    -vf "fps=20,scale=960:-2:flags=lanczos,palettegen=max_colors=128:stats_mode=diff" \
    "${palette}"
  ffmpeg -y -i "${movie}" -i "${palette}" \
    -lavfi "fps=20,scale=960:-2:flags=lanczos[x];[x][1:v]paletteuse=dither=sierra2_4a" \
    "${output}"
done
