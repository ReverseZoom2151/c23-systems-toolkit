#include "donut_fixed.h"

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

static void print_usage(FILE *output) {
    fputs("usage: donut-fixed [--fast] [--frames N] [--gif OUTPUT.gif] [--svg "
          "OUTPUT.svg] "
          "[WIDTH HEIGHT]\n",
          output);
}

int main(int argc, char *argv[]) {
    size_t width = 80;
    size_t height = 24;
    size_t frame_count = 1;
    size_t dimensions[2] = {0};
    size_t dimension_count = 0;
    const char *gif_path = NULL;
    const char *svg_path = NULL;
    donut_trig_backend backend = DONUT_TRIG_CORDIC;
    for (int index = 1; index < argc; index++) {
        if (strcmp(argv[index], "--fast") == 0) {
            backend = DONUT_TRIG_FAST_APPROX;
        } else if (strcmp(argv[index], "--frames") == 0) {
            if (++index == argc || !parse_size(argv[index], &frame_count)) {
                print_usage(stderr);
                return EXIT_FAILURE;
            }
        } else if (strcmp(argv[index], "--gif") == 0) {
            if (++index == argc) {
                print_usage(stderr);
                return EXIT_FAILURE;
            }
            gif_path = argv[index];
        } else if (strcmp(argv[index], "--svg") == 0) {
            if (++index == argc) {
                print_usage(stderr);
                return EXIT_FAILURE;
            }
            svg_path = argv[index];
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
    if (width > SIZE_MAX / height || frame_count > SIZE_MAX / sizeof(sketch_canvas *)) {
        print_usage(stderr);
        return EXIT_FAILURE;
    }
    sketch_canvas **frames = calloc(frame_count, sizeof(*frames));
    if (frames == NULL) {
        fputs("donut-fixed: allocation failure\n", stderr);
        return EXIT_FAILURE;
    }
    sketch_status status = SKETCH_OK;
    for (size_t index = 0; index < frame_count && status == SKETCH_OK; index++) {
        frames[index] = sketch_canvas_create(width, height);
        if (frames[index] == NULL) {
            status = SKETCH_ALLOCATION_FAILURE;
            break;
        }
        donut_options options;
        donut_options_default(&options);
        options.rotation_x_turns = (uint16_t)(options.rotation_x_turns + index * 1024U);
        options.rotation_z_turns = (uint16_t)(options.rotation_z_turns + index * 512U);
        options.trig_backend = backend;
        status = donut_render(frames[index], &options);
    }
    if (status == SKETCH_OK && gif_path != NULL) {
        status = sketch_write_gif((const sketch_canvas *const *)frames, frame_count,
                                  gif_path, 5, true);
    }
    if (status == SKETCH_OK && svg_path != NULL) {
        status = sketch_write_svg(frames[frame_count - 1], svg_path, true, 8);
    }
    if (status == SKETCH_OK && gif_path == NULL && svg_path == NULL) {
        status = sketch_write_ascii(frames[frame_count - 1], stdout);
    }
    for (size_t index = 0; index < frame_count; index++) {
        sketch_canvas_destroy(frames[index]);
    }
    free(frames);
    if (status != SKETCH_OK) {
        fprintf(stderr, "donut-fixed: %s\n", sketch_status_message(status));
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
