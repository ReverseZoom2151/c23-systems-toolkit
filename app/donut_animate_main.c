#include "donut.h"

#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <time.h>

enum { DEFAULT_FRAMES = 120, DEFAULT_FPS = 24, MAX_FPS = 1000 };

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

static int parse_fps(const char *text, double *value) {
    char *end = NULL;
    errno = 0;
    double parsed = strtod(text, &end);
    if (errno != 0 || end == text || *end != '\0' || !isfinite(parsed) ||
        parsed <= 0.0 || parsed > MAX_FPS) {
        return 0;
    }
    *value = parsed;
    return 1;
}

static void sleep_for_frame(double fps) {
    double seconds = 1.0 / fps;
    struct timespec remaining = {
        .tv_sec = (time_t)seconds,
        .tv_nsec = (long)((seconds - (double)(time_t)seconds) * 1000000000.0),
    };
    if (remaining.tv_nsec >= 1000000000L) {
        remaining.tv_sec++;
        remaining.tv_nsec -= 1000000000L;
    }
    struct timespec request = remaining;
    while (thrd_sleep(&request, &remaining) == -1) {
        request = remaining;
    }
}

static void print_usage(FILE *output) {
    fputs("usage: donut-animate [--incremental] [--frames N] [--fps N] "
          "[WIDTH HEIGHT]\n"
          "  defaults: 120 frames at 24 fps on an 80x24 canvas\n",
          output);
}

int main(int argc, char *argv[]) {
    size_t width = 80;
    size_t height = 24;
    size_t frame_count = DEFAULT_FRAMES;
    size_t dimensions[2] = {0};
    size_t dimension_count = 0;
    double fps = DEFAULT_FPS;
    donut_rotation_mode mode = DONUT_TRIGONOMETRIC;

    for (int index = 1; index < argc; index++) {
        if (strcmp(argv[index], "--incremental") == 0) {
            mode = DONUT_INCREMENTAL;
        } else if (strcmp(argv[index], "--frames") == 0) {
            if (++index == argc || !parse_size(argv[index], &frame_count)) {
                print_usage(stderr);
                return EXIT_FAILURE;
            }
        } else if (strcmp(argv[index], "--fps") == 0) {
            if (++index == argc || !parse_fps(argv[index], &fps)) {
                print_usage(stderr);
                return EXIT_FAILURE;
            }
        } else if (dimension_count < 2 &&
                   parse_size(argv[index], &dimensions[dimension_count])) {
            dimension_count++;
        } else {
            print_usage(stderr);
            return EXIT_FAILURE;
        }
    }
    if (dimension_count != 0 && dimension_count != 2) {
        print_usage(stderr);
        return EXIT_FAILURE;
    }
    if (dimension_count == 2) {
        width = dimensions[0];
        height = dimensions[1];
    }
    if (width > SIZE_MAX / height) {
        fputs("donut-animate: canvas is too large\n", stderr);
        return EXIT_FAILURE;
    }

    sketch_canvas *canvas = sketch_canvas_create(width, height);
    if (canvas == NULL) {
        fputs("donut-animate: allocation failure\n", stderr);
        return EXIT_FAILURE;
    }

    bool cursor_hidden = false;
    donut_status status = DONUT_OK;
    if (fputs("\x1b[2J\x1b[H\x1b[?25l", stdout) == EOF) {
        status = DONUT_IO_ERROR;
    } else {
        cursor_hidden = true;
    }
    for (size_t frame = 0; frame < frame_count && status == DONUT_OK; frame++) {
        status = donut_render_frame_mode(canvas, (float)frame * 0.10F,
                                         (float)frame * 0.05F, mode);
        if (status == DONUT_OK && fputs("\x1b[H", stdout) == EOF) {
            status = DONUT_IO_ERROR;
        }
        if (status == DONUT_OK) {
            status = donut_write_ascii(canvas, stdout);
        }
        if (status == DONUT_OK && fflush(stdout) == EOF) {
            status = DONUT_IO_ERROR;
        }
        if (status == DONUT_OK && frame + 1 < frame_count) {
            sleep_for_frame(fps);
        }
    }
    if (cursor_hidden && fputs("\x1b[?25h", stdout) == EOF) {
        status = DONUT_IO_ERROR;
    }
    if (fflush(stdout) == EOF) {
        status = DONUT_IO_ERROR;
    }
    sketch_canvas_destroy(canvas);
    if (status != DONUT_OK) {
        fprintf(stderr, "donut-animate: %s\n", donut_status_message(status));
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
