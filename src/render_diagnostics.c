#include "render_diagnostics.h"

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum {
    DIAGNOSTIC_SPACE = 256,
    DEFAULT_MAX_STEPS = 96,
    MAX_STEPS = 256,
    TORUS_MAJOR_RADIUS = 88,
    TORUS_MINOR_RADIUS = 30,
    CAMERA_Z = -280,
    MAX_RAY_DISTANCE = 600
};

static uint32_t integer_sqrt(uint64_t value) {
    uint64_t result = 0;
    uint64_t bit = UINT64_C(1) << 62;
    while (bit > value) {
        bit >>= 2;
    }
    while (bit != 0) {
        if (value >= result + bit) {
            value -= result + bit;
            result = (result >> 1) + bit;
        } else {
            result >>= 1;
        }
        bit >>= 2;
    }
    return (uint32_t)result;
}

static int32_t clamp_i32(int64_t value, int32_t low, int32_t high) {
    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return (int32_t)value;
}

static uint8_t clamp_u8(int64_t value) {
    return (uint8_t)clamp_i32(value, 0, UINT8_MAX);
}

static bool valid_canvas_dimensions(const sketch_canvas *canvas) {
    return canvas != NULL && sketch_canvas_width(canvas) != 0 &&
           sketch_canvas_height(canvas) != 0 &&
           sketch_canvas_width(canvas) <= INT32_MAX &&
           sketch_canvas_height(canvas) <= INT32_MAX;
}

static int32_t screen_coordinate(size_t coordinate, size_t extent) {
    /* Pixel-centred coordinates in [-256, 256], independent of resolution. */
    return (int32_t)(((int64_t)(2 * coordinate + 1) * DIAGNOSTIC_SPACE) /
                         (int64_t)extent -
                     DIAGNOSTIC_SPACE);
}

static int32_t sphere_depth(int32_t x, int32_t y) {
    int64_t radius_squared = (int64_t)DIAGNOSTIC_SPACE * DIAGNOSTIC_SPACE;
    int64_t distance_squared = (int64_t)x * x + (int64_t)y * y;
    if (distance_squared > radius_squared) {
        return -1;
    }
    /* Parabolic approximation: stable, inexpensive and useful as a depth proxy. */
    return (int32_t)((radius_squared - distance_squared) / DIAGNOSTIC_SPACE);
}

static uint8_t sphere_normal_shade(int32_t x, int32_t y, int32_t depth,
                                   uint32_t frame_index) {
    int32_t light_x = (frame_index & 1U) == 0 ? -96 : 96;
    int32_t light_y = (frame_index & 2U) == 0 ? -72 : 72;
    int32_t light_z = 224;
    int64_t dot =
        (int64_t)x * light_x + (int64_t)y * light_y + (int64_t)depth * light_z;
    return clamp_u8(128 + dot / (2 * DIAGNOSTIC_SPACE));
}

static int32_t torus_sdf(int32_t x, int32_t y, int32_t z) {
    uint64_t horizontal_squared = (uint64_t)((int64_t)x * x + (int64_t)z * z);
    int32_t horizontal = (int32_t)integer_sqrt(horizontal_squared);
    int32_t ring_distance = horizontal - TORUS_MAJOR_RADIUS;
    uint64_t tube_squared =
        (uint64_t)((int64_t)ring_distance * ring_distance + (int64_t)y * y);
    return (int32_t)integer_sqrt(tube_squared) - TORUS_MINOR_RADIUS;
}

static uint8_t torus_shade(int32_t x, int32_t y, int32_t z, uint32_t frame_index,
                           uint16_t steps) {
    enum { EPSILON = 2 };
    int32_t nx = torus_sdf(x + EPSILON, y, z) - torus_sdf(x - EPSILON, y, z);
    int32_t ny = torus_sdf(x, y + EPSILON, z) - torus_sdf(x, y - EPSILON, z);
    int32_t nz = torus_sdf(x, y, z + EPSILON) - torus_sdf(x, y, z - EPSILON);
    int32_t light_x = (frame_index & 1U) == 0 ? -48 : 48;
    int32_t light_y = -72;
    int32_t light_z = -96;
    int64_t dot = (int64_t)nx * light_x + (int64_t)ny * light_y + (int64_t)nz * light_z;
    /* Fewer steps are darker, exposing the ray-march budget in the output. */
    return clamp_u8(168 - dot / 8 - (int32_t)steps / 3);
}

static uint8_t render_torus_pixel(int32_t screen_x, int32_t screen_y,
                                  uint32_t frame_index, uint16_t max_steps) {
    int32_t x = 0;
    int32_t y = 0;
    int32_t z = CAMERA_Z;
    /* Direction is intentionally unnormalised: integer distance advances stay simple.
     */
    int32_t direction_x = screen_x / 2;
    int32_t direction_y = screen_y / 2;
    int32_t direction_z = DIAGNOSTIC_SPACE;
    int32_t travelled = 0;

    for (uint16_t step = 0; step < max_steps && travelled < MAX_RAY_DISTANCE; step++) {
        int32_t distance = torus_sdf(x, y, z);
        if (distance <= 2) {
            return torus_shade(x, y, z, frame_index, step);
        }
        int32_t advance = distance < 1 ? 1 : distance;
        travelled += advance;
        x += (int32_t)((int64_t)direction_x * advance / DIAGNOSTIC_SPACE);
        y += (int32_t)((int64_t)direction_y * advance / DIAGNOSTIC_SPACE);
        z += (int32_t)((int64_t)direction_z * advance / DIAGNOSTIC_SPACE);
    }
    return UINT8_MAX;
}

static sketch_status render_sphere_proxy(sketch_canvas *canvas,
                                         render_diagnostic_mode mode,
                                         uint32_t frame_index) {
    static const uint8_t bayer_4x4[4][4] = {
        {0, 8, 2, 10}, {12, 4, 14, 6}, {3, 11, 1, 9}, {15, 7, 13, 5}};
    size_t width = sketch_canvas_width(canvas);
    size_t height = sketch_canvas_height(canvas);
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            int32_t sx = screen_coordinate(x, width);
            int32_t sy = screen_coordinate(y, height);
            int32_t depth = sphere_depth(sx, sy);
            uint8_t value = UINT8_MAX;
            if (depth >= 0) {
                if (mode == RENDER_DIAGNOSTIC_DEPTH) {
                    value = clamp_u8(UINT8_MAX -
                                     (int64_t)depth * UINT8_MAX / DIAGNOSTIC_SPACE);
                } else if (mode == RENDER_DIAGNOSTIC_NORMALS) {
                    value = sphere_normal_shade(sx, sy, depth, frame_index);
                } else {
                    uint8_t tone =
                        clamp_u8((int64_t)depth * UINT8_MAX / DIAGNOSTIC_SPACE);
                    uint8_t threshold = bayer_4x4[y % 4][x % 4] * 16U + 8U;
                    value = tone > threshold ? 0 : UINT8_MAX;
                }
            }
            sketch_canvas_set_pixel(canvas, x, y, value);
        }
    }
    return SKETCH_OK;
}

sketch_status render_diagnostics_render(sketch_canvas *canvas,
                                        render_diagnostic_mode mode,
                                        const render_diagnostics_config *config) {
    if (!valid_canvas_dimensions(canvas)) {
        return SKETCH_INVALID_ARGUMENT;
    }
    uint32_t frame_index = config == NULL ? 0 : config->frame_index;
    uint16_t max_steps = config == NULL || config->max_steps == 0 ? DEFAULT_MAX_STEPS
                                                                  : config->max_steps;
    if (max_steps > MAX_STEPS) {
        return SKETCH_OUT_OF_RANGE;
    }

    switch (mode) {
    case RENDER_DIAGNOSTIC_DEPTH:
    case RENDER_DIAGNOSTIC_NORMALS:
    case RENDER_DIAGNOSTIC_ORDERED_DITHER:
        return render_sphere_proxy(canvas, mode, frame_index);
    case RENDER_DIAGNOSTIC_RAY_MARCH_TORUS: {
        size_t width = sketch_canvas_width(canvas);
        size_t height = sketch_canvas_height(canvas);
        for (size_t y = 0; y < height; y++) {
            for (size_t x = 0; x < width; x++) {
                sketch_canvas_set_pixel(canvas, x, y,
                                        render_torus_pixel(screen_coordinate(x, width),
                                                           screen_coordinate(y, height),
                                                           frame_index, max_steps));
            }
        }
        return SKETCH_OK;
    }
    }
    return SKETCH_INVALID_ARGUMENT;
}

sketch_status
render_diagnostics_ray_march_torus(sketch_canvas *canvas,
                                   const render_diagnostics_config *config) {
    return render_diagnostics_render(canvas, RENDER_DIAGNOSTIC_RAY_MARCH_TORUS, config);
}
