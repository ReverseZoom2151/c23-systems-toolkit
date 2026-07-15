#include "render_diagnostics.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parse_size(const char *text, size_t *value) {
    char *end = NULL;
    errno = 0;
    unsigned long long parsed = strtoull(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || parsed == 0 || parsed > SIZE_MAX) {
        return 0;
    }
    *value = (size_t)parsed;
    return 1;
}

static int parse_mode(const char *text, render_diagnostic_mode *mode) {
    if (strcmp(text, "depth") == 0) {
        *mode = RENDER_DIAGNOSTIC_DEPTH;
    } else if (strcmp(text, "normals") == 0) {
        *mode = RENDER_DIAGNOSTIC_NORMALS;
    } else if (strcmp(text, "dither") == 0) {
        *mode = RENDER_DIAGNOSTIC_ORDERED_DITHER;
    } else if (strcmp(text, "raymarch") == 0) {
        *mode = RENDER_DIAGNOSTIC_RAY_MARCH_TORUS;
    } else {
        return 0;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 3 && argc != 5) {
        fputs("usage: render-diagnostics depth|normals|dither|raymarch OUTPUT.svg "
              "[WIDTH HEIGHT]\n",
              stderr);
        return EXIT_FAILURE;
    }
    render_diagnostic_mode mode;
    if (!parse_mode(argv[1], &mode)) {
        fputs("render-diagnostics: unknown mode\n", stderr);
        return EXIT_FAILURE;
    }
    size_t width = 160;
    size_t height = 60;
    if (argc == 5 && (!parse_size(argv[3], &width) || !parse_size(argv[4], &height))) {
        fputs("render-diagnostics: dimensions must be positive integers\n", stderr);
        return EXIT_FAILURE;
    }
    sketch_canvas *canvas = sketch_canvas_create(width, height);
    if (canvas == NULL) {
        fputs("render-diagnostics: allocation failure\n", stderr);
        return EXIT_FAILURE;
    }
    render_diagnostics_config config = {.frame_index = 0, .max_steps = 96};
    sketch_status status = render_diagnostics_render(canvas, mode, &config);
    if (status == SKETCH_OK) {
        status = sketch_write_svg(canvas, argv[2], true, 4);
    }
    sketch_canvas_destroy(canvas);
    if (status != SKETCH_OK) {
        fprintf(stderr, "render-diagnostics: %s\n", sketch_status_message(status));
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
