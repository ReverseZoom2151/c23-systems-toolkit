#include "sketch.h"

#include <assert.h>
#include <stdint.h>

int main(void) {
    /* Set black, move down, then draw a horizontal line. */
    const uint8_t line[] = {0xc0, 0x03, 0x83, 0x42, 0x80};
    sketch_canvas *canvas = sketch_canvas_create(5, 5);
    assert(canvas != NULL);
    assert(sketch_decode_bytes(canvas, line, sizeof(line)) == SKETCH_OK);
    assert(sketch_canvas_pixel(canvas, 0, 3) == 0);
    assert(sketch_canvas_pixel(canvas, 1, 3) == 0);
    assert(sketch_canvas_pixel(canvas, 2, 3) == 0);
    assert(sketch_canvas_pixel(canvas, 3, 3) == UINT8_MAX);

    /* Move without drawing, then fill the rectangle from (2, 3) to (4, 4). */
    const uint8_t block[] = {0x00, 0xc2, 0x04, 0xc3, 0x05, 0x80,
                             0x02, 0xc4, 0x04, 0xc4, 0x05, 0x80};
    assert(sketch_decode_bytes(canvas, block, sizeof(block)) == SKETCH_OK);
    assert(sketch_canvas_pixel(canvas, 2, 3) == 0);
    assert(sketch_canvas_pixel(canvas, 4, 4) == 0);
    assert(sketch_canvas_pixel(canvas, 1, 4) == UINT8_MAX);

    sketch_canvas_clear(canvas, UINT8_MAX);
    /* An off-canvas endpoint is clipped instead of making rasterisation unbounded. */
    const uint8_t clipped[] = {0xff, 0x04, 0x80};
    assert(sketch_decode_bytes(canvas, clipped, sizeof(clipped)) == SKETCH_OK);
    for (size_t x = 0; x < 5; x++) {
        assert(sketch_canvas_pixel(canvas, x, 0) == 0);
    }

    assert(sketch_decode_bytes(canvas, (const uint8_t[]){0x3f}, 1) ==
           SKETCH_MALFORMED_STREAM);
    sketch_canvas_destroy(canvas);
    return 0;
}
