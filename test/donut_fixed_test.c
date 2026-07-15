#include "donut_fixed.h"

#include <assert.h>
#include <stdint.h>

static size_t count_non_background(const sketch_canvas *canvas, uint8_t background) {
    size_t count = 0;
    for (size_t y = 0; y < sketch_canvas_height(canvas); ++y)
        for (size_t x = 0; x < sketch_canvas_width(canvas); ++x)
            if (sketch_canvas_pixel(canvas, x, y) != background)
                ++count;
    return count;
}

int main(void) {
    sketch_canvas *canvas = sketch_canvas_create(80, 40);
    assert(canvas != NULL);
    donut_options options;
    donut_options_default(&options);
    options.theta_steps = 48;
    options.phi_steps = 96;
    assert(donut_render(canvas, &options) == SKETCH_OK);
    assert(count_non_background(canvas, options.background) > 200);

    options.trig_backend = DONUT_TRIG_FAST_APPROX;
    assert(donut_render(canvas, &options) == SKETCH_OK);
    assert(count_non_background(canvas, options.background) > 100);

    options.theta_steps = 2;
    assert(donut_render(canvas, &options) == SKETCH_INVALID_ARGUMENT);
    assert(donut_render(NULL, &options) == SKETCH_INVALID_ARGUMENT);
    sketch_canvas_destroy(canvas);
    return 0;
}
