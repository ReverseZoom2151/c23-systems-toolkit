#ifndef IMPERATIVE_TOOLKIT_DONUT_FIXED_H
#define IMPERATIVE_TOOLKIT_DONUT_FIXED_H

#include "sketch.h"

#include <stdint.h>

/* Trigonometry is evaluated without libm or floating point. */
typedef enum { DONUT_TRIG_CORDIC, DONUT_TRIG_FAST_APPROX } donut_trig_backend;

typedef struct {
    uint16_t theta_steps;
    uint16_t phi_steps;
    /* Angles use 65536 units per revolution. */
    uint16_t rotation_x_turns;
    uint16_t rotation_z_turns;
    uint8_t foreground;
    uint8_t background;
    donut_trig_backend trig_backend;
} donut_options;

void donut_options_default(donut_options *options);

/* Clear and render a shaded torus into canvas.  Rendering is deterministic,
 * uses a per-pixel depth buffer, and has no dependencies beyond the toolkit. */
sketch_status donut_render(sketch_canvas *canvas, const donut_options *options);

#endif
