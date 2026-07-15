#ifndef IMPERATIVE_TOOLKIT_RENDER_DIAGNOSTICS_H
#define IMPERATIVE_TOOLKIT_RENDER_DIAGNOSTICS_H

#include "sketch.h"

#include <stdint.h>

/*
 * Deterministic software-rendering diagnostics for sketch_canvas.
 *
 * These are compact visual debugging aids, not hardware emulation.  The
 * depth and normal modes use an integer sphere proxy; ordered dither quantises
 * that depth proxy with a 4x4 Bayer matrix.  The ray-march mode is a bounded
 * signed-distance-field simulation of a torus.  Identical inputs always yield
 * identical pixels on every supported platform.
 */
typedef enum {
    RENDER_DIAGNOSTIC_DEPTH,
    RENDER_DIAGNOSTIC_NORMALS,
    RENDER_DIAGNOSTIC_ORDERED_DITHER,
    RENDER_DIAGNOSTIC_RAY_MARCH_TORUS
} render_diagnostic_mode;

typedef struct {
    /* Changes the deterministic light direction; zero is a useful default. */
    uint32_t frame_index;
    /* Ray-march only: 1..256 steps. Zero selects the default of 96. */
    uint16_t max_steps;
} render_diagnostics_config;

/* Render one diagnostic frame. config may be NULL to use deterministic defaults. */
sketch_status render_diagnostics_render(sketch_canvas *canvas,
                                        render_diagnostic_mode mode,
                                        const render_diagnostics_config *config);

/* Convenience entry point for the explicitly bounded torus ray-march mode. */
sketch_status
render_diagnostics_ray_march_torus(sketch_canvas *canvas,
                                   const render_diagnostics_config *config);

#endif
