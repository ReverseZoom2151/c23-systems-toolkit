#include "sketch.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

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

int main(int argc, char *argv[]) {
    size_t width = 80;
    size_t height = 40;
    if (argc != 2 && argc != 4) {
        fprintf(stderr, "usage: sketch-view FILE [WIDTH HEIGHT]\n");
        return EXIT_FAILURE;
    }
    if (argc == 4 && (!parse_size(argv[2], &width) || !parse_size(argv[3], &height))) {
        fprintf(stderr, "WIDTH and HEIGHT must be positive integers\n");
        return EXIT_FAILURE;
    }
    sketch_canvas *canvas = sketch_canvas_create(width, height);
    if (canvas == NULL) {
        fprintf(stderr, "could not allocate canvas\n");
        return EXIT_FAILURE;
    }
    sketch_status status = sketch_decode_file(canvas, argv[1]);
    if (status == SKETCH_OK) {
        status = sketch_write_ascii(canvas, stdout);
    }
    sketch_canvas_destroy(canvas);
    if (status != SKETCH_OK) {
        fprintf(stderr, "sketch-view: %s\n", sketch_status_message(status));
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
