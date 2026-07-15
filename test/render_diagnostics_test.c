#include "render_diagnostics.h"

#include <assert.h>
#include <stdint.h>

static size_t count_value(const sketch_canvas *canvas, uint8_t value) {
    size_t count = 0;
    for (size_t y = 0; y < sketch_canvas_height(canvas); y++) {
        for (size_t x = 0; x < sketch_canvas_width(canvas); x++) {
            count += sketch_canvas_pixel(canvas, x, y) == value;
        }
    }
    return count;
}

int main(void) {
    sketch_canvas *canvas = sketch_canvas_create(32, 32);
    assert(canvas != NULL);

    assert(render_diagnostics_render(canvas, RENDER_DIAGNOSTIC_DEPTH, NULL) ==
           SKETCH_OK);
    assert(sketch_canvas_pixel(canvas, 0, 0) == UINT8_MAX);
    assert(sketch_canvas_pixel(canvas, 16, 16) < 8);
    assert(sketch_canvas_pixel(canvas, 16, 4) > sketch_canvas_pixel(canvas, 16, 16));

    render_diagnostics_config lit = {.frame_index = 3, .max_steps = 0};
    assert(render_diagnostics_render(canvas, RENDER_DIAGNOSTIC_NORMALS, &lit) ==
           SKETCH_OK);
    uint8_t normal_centre = sketch_canvas_pixel(canvas, 16, 16);
    assert(normal_centre > 0 && normal_centre < UINT8_MAX);
    assert(sketch_canvas_pixel(canvas, 0, 0) == UINT8_MAX);

    assert(render_diagnostics_render(canvas, RENDER_DIAGNOSTIC_ORDERED_DITHER, NULL) ==
           SKETCH_OK);
    size_t black_pixels = count_value(canvas, 0);
    size_t white_pixels = count_value(canvas, UINT8_MAX);
    assert(black_pixels > 0);
    assert(white_pixels > black_pixels);

    render_diagnostics_config ray = {.frame_index = 0, .max_steps = 64};
    assert(render_diagnostics_ray_march_torus(canvas, &ray) == SKETCH_OK);
    assert(count_value(canvas, UINT8_MAX) > 0);
    assert(count_value(canvas, UINT8_MAX) < 32 * 32);
    uint8_t first_render = sketch_canvas_pixel(canvas, 16, 16);
    assert(render_diagnostics_ray_march_torus(canvas, &ray) == SKETCH_OK);
    assert(sketch_canvas_pixel(canvas, 16, 16) == first_render);

    ray.max_steps = 257;
    assert(render_diagnostics_ray_march_torus(canvas, &ray) == SKETCH_OUT_OF_RANGE);
    assert(render_diagnostics_render(NULL, RENDER_DIAGNOSTIC_DEPTH, NULL) ==
           SKETCH_INVALID_ARGUMENT);
    sketch_canvas_destroy(canvas);
    return 0;
}
