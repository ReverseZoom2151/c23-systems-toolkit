#include "donut_fixed.h"

#include <limits.h>
#include <stddef.h>
#include <stdlib.h>

enum {
    Q_SHIFT = 30,
    TURN = 65536,
    HALF_TURN = TURN / 2,
    QUARTER_TURN = TURN / 4,
    CORDIC_GAIN_Q30 = 652032874
};

static const int cordic_atan_turns[] = {8192, 4836, 2555, 1297, 651, 326, 163, 82,
                                        41,   20,   10,   5,    3,   1,   1,   0};

static int64_t multiply_q30(int64_t left, int64_t right) {
    return (left * right) >> Q_SHIFT;
}

static int normalize_signed_turns(uint16_t turns) {
    int value = (int)turns;
    return value > HALF_TURN - 1 ? value - TURN : value;
}

static void cordic_sine_cosine(uint16_t turns, int64_t *sine, int64_t *cosine) {
    int angle = normalize_signed_turns(turns);
    int negate = 0;
    if (angle > QUARTER_TURN) {
        angle -= HALF_TURN;
        negate = 1;
    } else if (angle < -QUARTER_TURN) {
        angle += HALF_TURN;
        negate = 1;
    }
    int64_t x = CORDIC_GAIN_Q30;
    int64_t y = 0;
    for (size_t index = 0;
         index < sizeof(cordic_atan_turns) / sizeof(cordic_atan_turns[0]); ++index) {
        int64_t next_x;
        int64_t next_y;
        if (angle >= 0) {
            next_x = x - (y >> index);
            next_y = y + (x >> index);
            angle -= cordic_atan_turns[index];
        } else {
            next_x = x + (y >> index);
            next_y = y - (x >> index);
            angle += cordic_atan_turns[index];
        }
        x = next_x;
        y = next_y;
    }
    *cosine = negate ? -x : x;
    *sine = negate ? -y : y;
}

static int64_t fast_sine(uint16_t turns) {
    int64_t phase = normalize_signed_turns(turns);
    /* phase is [-1, 1) in Q30; this is the fixed-point parabola 4x(1-|x|). */
    int64_t x = (phase << Q_SHIFT) / HALF_TURN;
    int64_t magnitude = x < 0 ? -x : x;
    return 4 * multiply_q30(x, (INT64_C(1) << Q_SHIFT) - magnitude);
}

static void sine_cosine(uint16_t turns, donut_trig_backend backend, int64_t *sine,
                        int64_t *cosine) {
    if (backend == DONUT_TRIG_CORDIC) {
        cordic_sine_cosine(turns, sine, cosine);
    } else {
        *sine = fast_sine(turns);
        *cosine = fast_sine((uint16_t)(turns + QUARTER_TURN));
    }
}

void donut_options_default(donut_options *options) {
    if (options == NULL) {
        return;
    }
    *options = (donut_options){
        .theta_steps = 72,
        .phi_steps = 144,
        .rotation_x_turns = 6144,
        .rotation_z_turns = 3072,
        .foreground = 0,
        .background = UINT8_MAX,
        .trig_backend = DONUT_TRIG_CORDIC,
    };
}

static uint8_t shade(uint8_t foreground, uint8_t background, int64_t normal_z) {
    int64_t brightness = (normal_z + (INT64_C(1) << Q_SHIFT)) >> 1;
    if (brightness < 0)
        brightness = 0;
    if (brightness > (INT64_C(1) << Q_SHIFT))
        brightness = INT64_C(1) << Q_SHIFT;
    int64_t difference = (int64_t)background - foreground;
    return (uint8_t)(background - ((difference * brightness) >> Q_SHIFT));
}

sketch_status donut_render(sketch_canvas *canvas, const donut_options *options) {
    if (canvas == NULL || options == NULL || options->theta_steps < 3 ||
        options->phi_steps < 3 ||
        (options->trig_backend != DONUT_TRIG_CORDIC &&
         options->trig_backend != DONUT_TRIG_FAST_APPROX)) {
        return SKETCH_INVALID_ARGUMENT;
    }
    size_t width = sketch_canvas_width(canvas);
    size_t height = sketch_canvas_height(canvas);
    if (width == 0 || height == 0 || width > SIZE_MAX / height ||
        width * height > SIZE_MAX / sizeof(int64_t)) {
        return SKETCH_INVALID_ARGUMENT;
    }
    size_t pixels = width * height;
    int64_t *depth = malloc(pixels * sizeof(*depth));
    if (depth == NULL)
        return SKETCH_ALLOCATION_FAILURE;
    for (size_t index = 0; index < pixels; ++index)
        depth[index] = INT64_MIN;
    sketch_canvas_clear(canvas, options->background);

    const int64_t major_radius = 880468296;       /* 0.82 in Q30 */
    const int64_t minor_radius = 375809638;       /* 0.35 in Q30 */
    const int64_t camera_distance = 2576980378LL; /* 2.4 in Q30 */
    const int64_t projection_scale = 1825361101;  /* 1.7 in Q30 */
    int64_t sin_x, cos_x, sin_z, cos_z;
    sine_cosine(options->rotation_x_turns, options->trig_backend, &sin_x, &cos_x);
    sine_cosine(options->rotation_z_turns, options->trig_backend, &sin_z, &cos_z);

    for (uint32_t theta_index = 0; theta_index < options->theta_steps; ++theta_index) {
        uint16_t theta = (uint16_t)((theta_index * TURN) / options->theta_steps);
        int64_t sin_theta, cos_theta;
        sine_cosine(theta, options->trig_backend, &sin_theta, &cos_theta);
        int64_t ring_radius = major_radius + multiply_q30(minor_radius, cos_theta);
        for (uint32_t phi_index = 0; phi_index < options->phi_steps; ++phi_index) {
            uint16_t phi = (uint16_t)((phi_index * TURN) / options->phi_steps);
            int64_t sin_phi, cos_phi;
            sine_cosine(phi, options->trig_backend, &sin_phi, &cos_phi);
            int64_t x = multiply_q30(ring_radius, cos_phi);
            int64_t y = multiply_q30(ring_radius, sin_phi);
            int64_t z = multiply_q30(minor_radius, sin_theta);
            int64_t y_rotated = multiply_q30(y, cos_x) - multiply_q30(z, sin_x);
            int64_t z_rotated = multiply_q30(y, sin_x) + multiply_q30(z, cos_x);
            int64_t x_rotated = multiply_q30(x, cos_z) - multiply_q30(y_rotated, sin_z);
            int64_t y_screen = multiply_q30(x, sin_z) + multiply_q30(y_rotated, cos_z);
            int64_t denominator = camera_distance + z_rotated;
            if (denominator <= 0)
                continue;
            int64_t projected_x = multiply_q30(x_rotated, projection_scale) *
                                  (int64_t)width / 2 / denominator;
            int64_t projected_y = multiply_q30(y_screen, projection_scale) *
                                  (int64_t)height / 2 / denominator;
            int64_t screen_x = (int64_t)width / 2 + projected_x;
            int64_t screen_y = (int64_t)height / 2 - projected_y;
            if (screen_x < 0 || screen_y < 0 || (uint64_t)screen_x >= width ||
                (uint64_t)screen_y >= height)
                continue;
            size_t pixel = (size_t)screen_y * width + (size_t)screen_x;
            if (z_rotated <= depth[pixel])
                continue;
            depth[pixel] = z_rotated;
            int64_t normal_y = multiply_q30(cos_theta, sin_phi);
            int64_t normal_z = sin_theta;
            int64_t normal_z_rotated =
                multiply_q30(normal_y, sin_x) + multiply_q30(normal_z, cos_x);
            sketch_canvas_set_pixel(
                canvas, (size_t)screen_x, (size_t)screen_y,
                shade(options->foreground, options->background, normal_z_rotated));
        }
    }
    free(depth);
    return SKETCH_OK;
}
