#include "donut.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

enum { DONUT_SHADE_COUNT = 12, DONUT_THETA_SAMPLES = 90, DONUT_PHI_SAMPLES = 315 };

static void rotate_incrementally(float *cosine, float *sine, float delta_cosine,
                                 float delta_sine) {
    float next_cosine = *cosine * delta_cosine - *sine * delta_sine;
    float next_sine = *sine * delta_cosine + *cosine * delta_sine;
    /* One Newton step keeps the recurrence close to the unit circle. */
    float normalization =
        (3.0F - next_cosine * next_cosine - next_sine * next_sine) / 2.0F;
    *cosine = next_cosine * normalization;
    *sine = next_sine * normalization;
}

static size_t shade_index(float luminance) {
    if (luminance <= 0.0F) {
        return 0;
    }
    size_t shade = (size_t)(luminance * (float)(DONUT_SHADE_COUNT - 1));
    return shade < DONUT_SHADE_COUNT ? shade : DONUT_SHADE_COUNT - 1;
}

donut_status donut_render_frame_mode(sketch_canvas *canvas, float angle_a,
                                     float angle_b, donut_rotation_mode mode) {
    if (canvas == NULL || sketch_canvas_width(canvas) == 0 ||
        sketch_canvas_height(canvas) == 0) {
        return DONUT_INVALID_ARGUMENT;
    }
    size_t width = sketch_canvas_width(canvas);
    size_t height = sketch_canvas_height(canvas);
    if (width > SIZE_MAX / height ||
        (mode != DONUT_TRIGONOMETRIC && mode != DONUT_INCREMENTAL)) {
        return DONUT_INVALID_ARGUMENT;
    }
    float *depth = calloc(width * height, sizeof(*depth));
    if (depth == NULL) {
        return DONUT_ALLOCATION_FAILURE;
    }

    sketch_canvas_clear(canvas, UINT8_MAX);
    const float sin_a = sinf(angle_a);
    const float cos_a = cosf(angle_a);
    const float sin_b = sinf(angle_b);
    const float cos_b = cosf(angle_b);
    const float major_radius = 2.0F;
    const float minor_radius = 1.0F;
    const float viewer_distance = 5.0F;
    const float projection =
        (float)width * viewer_distance * 3.0F / (8.0F * (major_radius + minor_radius));

    float theta_sines[DONUT_THETA_SAMPLES];
    float theta_cosines[DONUT_THETA_SAMPLES];
    float phi_sines[DONUT_PHI_SAMPLES];
    float phi_cosines[DONUT_PHI_SAMPLES];
    float theta_sine = 0.0F;
    float theta_cosine = 1.0F;
    float phi_sine = 0.0F;
    float phi_cosine = 1.0F;
    const float theta_delta_sine = sinf(0.07F);
    const float theta_delta_cosine = cosf(0.07F);
    const float phi_delta_sine = sinf(0.02F);
    const float phi_delta_cosine = cosf(0.02F);
    for (size_t index = 0; index < DONUT_THETA_SAMPLES; index++) {
        theta_sines[index] =
            mode == DONUT_INCREMENTAL ? theta_sine : sinf((float)index * 0.07F);
        theta_cosines[index] =
            mode == DONUT_INCREMENTAL ? theta_cosine : cosf((float)index * 0.07F);
        rotate_incrementally(&theta_cosine, &theta_sine, theta_delta_cosine,
                             theta_delta_sine);
    }
    for (size_t index = 0; index < DONUT_PHI_SAMPLES; index++) {
        phi_sines[index] =
            mode == DONUT_INCREMENTAL ? phi_sine : sinf((float)index * 0.02F);
        phi_cosines[index] =
            mode == DONUT_INCREMENTAL ? phi_cosine : cosf((float)index * 0.02F);
        rotate_incrementally(&phi_cosine, &phi_sine, phi_delta_cosine, phi_delta_sine);
    }

    for (size_t theta_index = 0; theta_index < DONUT_THETA_SAMPLES; theta_index++) {
        const float sin_theta = theta_sines[theta_index];
        const float cos_theta = theta_cosines[theta_index];
        const float ring_radius = major_radius + minor_radius * cos_theta;
        for (size_t phi_index = 0; phi_index < DONUT_PHI_SAMPLES; phi_index++) {
            const float sin_phi = phi_sines[phi_index];
            const float cos_phi = phi_cosines[phi_index];
            const float inverse_depth = 1.0F / (sin_phi * ring_radius * sin_a +
                                                sin_theta * cos_a + viewer_distance);
            const float rotated_y = sin_phi * ring_radius * cos_a - sin_theta * sin_a;
            const int x =
                (int)((float)width / 2.0F +
                      projection * inverse_depth *
                          (cos_phi * ring_radius * cos_b - rotated_y * sin_b));
            const int y =
                (int)((float)height / 2.0F +
                      projection * inverse_depth *
                          (cos_phi * ring_radius * sin_b + rotated_y * cos_b) / 2.0F);
            if (x < 0 || y < 0 || (size_t)x >= width || (size_t)y >= height) {
                continue;
            }
            size_t offset = (size_t)y * width + (size_t)x;
            if (inverse_depth <= depth[offset]) {
                continue;
            }
            const float luminance =
                (sin_theta * sin_a - sin_phi * cos_theta * cos_a) * cos_b -
                sin_phi * cos_theta * sin_a - sin_theta * cos_a -
                cos_phi * cos_theta * sin_b;
            size_t shade = shade_index(luminance * 0.8F);
            depth[offset] = inverse_depth;
            sketch_canvas_set_pixel(
                canvas, (size_t)x, (size_t)y,
                (uint8_t)(UINT8_MAX - shade * UINT8_MAX / (DONUT_SHADE_COUNT - 1)));
        }
    }
    free(depth);
    return DONUT_OK;
}

donut_status donut_render_frame(sketch_canvas *canvas, float angle_a, float angle_b) {
    return donut_render_frame_mode(canvas, angle_a, angle_b, DONUT_TRIGONOMETRIC);
}

donut_status donut_write_ascii(const sketch_canvas *canvas, FILE *output) {
    static const char shades[] = ".,-~:;=!*#$@";
    if (canvas == NULL || output == NULL) {
        return DONUT_INVALID_ARGUMENT;
    }
    for (size_t y = 0; y < sketch_canvas_height(canvas); y++) {
        for (size_t x = 0; x < sketch_canvas_width(canvas); x++) {
            uint8_t pixel = sketch_canvas_pixel(canvas, x, y);
            size_t shade =
                (size_t)(UINT8_MAX - pixel) * (DONUT_SHADE_COUNT - 1) / UINT8_MAX;
            if (fputc(pixel == UINT8_MAX ? ' ' : shades[shade], output) == EOF) {
                return DONUT_IO_ERROR;
            }
        }
        if (fputc('\n', output) == EOF) {
            return DONUT_IO_ERROR;
        }
    }
    return DONUT_OK;
}

const char *donut_status_message(donut_status status) {
    switch (status) {
    case DONUT_OK:
        return "ok";
    case DONUT_INVALID_ARGUMENT:
        return "invalid argument";
    case DONUT_ALLOCATION_FAILURE:
        return "allocation failure";
    case DONUT_IO_ERROR:
        return "I/O error";
    }
    return "unknown donut error";
}
