#include "binary.h"
#include "donut.h"
#include "list.h"
#include "sketch.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { FRAME_WIDTH = 96, FRAME_HEIGHT = 32, PANEL_MARGIN = 24 };

static void set_pixel(sketch_canvas *canvas, int x, int y, uint8_t value) {
    if (x >= 0 && y >= 0) {
        sketch_canvas_set_pixel(canvas, (size_t)x, (size_t)y, value);
    }
}

static void fill_rect(sketch_canvas *canvas, int x, int y, int width, int height,
                      uint8_t value) {
    for (int row = y; row < y + height; row++) {
        for (int column = x; column < x + width; column++) {
            set_pixel(canvas, column, row, value);
        }
    }
}

static void stroke_rect(sketch_canvas *canvas, int x, int y, int width, int height,
                        uint8_t value) {
    fill_rect(canvas, x, y, width, 1, value);
    fill_rect(canvas, x, y + height - 1, width, 1, value);
    fill_rect(canvas, x, y, 1, height, value);
    fill_rect(canvas, x + width - 1, y, 1, height, value);
}

static void draw_arrow(sketch_canvas *canvas, int x0, int y, int x1, uint8_t value) {
    for (int x = x0; x < x1; x++) {
        set_pixel(canvas, x, y, value);
    }
    set_pixel(canvas, x1, y, value);
    set_pixel(canvas, x1 - 1, y - 1, value);
    set_pixel(canvas, x1 - 1, y + 1, value);
}

static void draw_bit_frame(sketch_canvas *canvas, const char bits[9], size_t visible) {
    sketch_canvas_clear(canvas, UINT8_MAX);
    for (size_t index = 0; index < 8; index++) {
        int x = 8 + (int)index * 10;
        uint8_t border = index == 0 ? 32 : 80;
        if (index < visible) {
            fill_rect(canvas, x, 10, 8, 12, bits[index] == '1' ? 0 : 180);
            stroke_rect(canvas, x, 10, 8, 12, border);
        } else {
            stroke_rect(canvas, x, 10, 8, 12, 180);
        }
    }
}

static void draw_list_frame(sketch_canvas *canvas, size_t nodes, size_t cursor) {
    sketch_canvas_clear(canvas, UINT8_MAX);
    for (size_t index = 0; index < nodes; index++) {
        int x = 16 + (int)index * 24;
        fill_rect(canvas, x, 12, 16, 10, index == cursor ? 0 : 170);
        stroke_rect(canvas, x, 12, 16, 10, 40);
        if (index + 1 < nodes) {
            draw_arrow(canvas, x + 16, 17, x + 23, 60);
        }
    }
    if (nodes != 0 && cursor < nodes) {
        int x = 16 + (int)cursor * 24 + 8;
        for (int y = 4; y < 11; y++) {
            set_pixel(canvas, x, y, 0);
        }
        set_pixel(canvas, x - 1, 9, 0);
        set_pixel(canvas, x + 1, 9, 0);
    }
}

static int write_svg_begin(FILE *output, unsigned width, unsigned height,
                           const char *title, const char *subtitle) {
    return fprintf(
               output,
               "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%u\" height=\"%u\" "
               "viewBox=\"0 0 %u %u\" role=\"img\">\n"
               "<rect width=\"100%%\" height=\"100%%\" rx=\"20\" fill=\"#0F172A\"/>\n"
               "<text x=\"%u\" y=\"42\" text-anchor=\"middle\" fill=\"#F8FAFC\" "
               "font-family=\"system-ui,sans-serif\" font-size=\"24\" "
               "font-weight=\"700\">%s</text>\n"
               "<text x=\"%u\" y=\"68\" text-anchor=\"middle\" fill=\"#94A3B8\" "
               "font-family=\"system-ui,sans-serif\" font-size=\"14\">%s</text>\n",
               width, height, width, height, width / 2, title, width / 2, subtitle) < 0
               ? 0
               : 1;
}

static int write_svg_end(FILE *output) {
    return fputs("</svg>\n", output) != EOF;
}

static int write_binary_svg(const char *path, const char bits[9]) {
    FILE *output = fopen(path, "w");
    if (output == NULL ||
        !write_svg_begin(
            output, 960, 300, "Binary representation, made visible",
            "i8 −12: input → stored two’s-complement bits → hexadecimal")) {
        if (output != NULL) {
            fclose(output);
        }
        return 0;
    }
    if (fprintf(output,
                "<g font-family=\"system-ui,sans-serif\">"
                "<text x=\"70\" y=\"125\" fill=\"#CBD5E1\" font-size=\"15\">decimal "
                "input</text>"
                "<text x=\"70\" y=\"166\" fill=\"#F8FAFC\" font-size=\"30\" "
                "font-weight=\"700\">−12</text>"
                "<path d=\"M205 150H270\" stroke=\"#64748B\" stroke-width=\"3\"/><path "
                "d=\"M270 150l-10-7v14z\" fill=\"#64748B\"/>"
                "<text x=\"318\" y=\"125\" fill=\"#CBD5E1\" font-size=\"15\">stored i8 "
                "bits</text>") < 0) {
        fclose(output);
        return 0;
    }
    for (size_t index = 0; index < 8; index++) {
        unsigned x = 318 + (unsigned)index * 52;
        const char *fill =
            bits[index] == '1' ? (index == 0 ? "#F97316" : "#2563EB") : "#1E293B";
        if (fprintf(output,
                    "<rect x=\"%u\" y=\"140\" width=\"42\" height=\"52\" rx=\"8\" "
                    "fill=\"%s\"/>"
                    "<text x=\"%u\" y=\"174\" text-anchor=\"middle\" fill=\"#F8FAFC\" "
                    "font-size=\"24\" font-weight=\"700\">%c</text>"
                    "<text x=\"%u\" y=\"215\" text-anchor=\"middle\" fill=\"#94A3B8\" "
                    "font-size=\"12\">b%zu</text>",
                    x, fill, x + 21, bits[index], x + 21, 7 - index) < 0) {
            fclose(output);
            return 0;
        }
    }
    int result =
        fprintf(output,
                "<text x=\"318\" y=\"245\" fill=\"#FB923C\" font-size=\"13\">orange = "
                "sign bit</text>"
                "<path d=\"M745 150H805\" stroke=\"#64748B\" stroke-width=\"3\"/><path "
                "d=\"M805 150l-10-7v14z\" fill=\"#64748B\"/>"
                "<text x=\"835\" y=\"125\" fill=\"#CBD5E1\" font-size=\"15\">hex</text>"
                "<text x=\"835\" y=\"174\" fill=\"#22C55E\" font-size=\"30\" "
                "font-weight=\"700\">F4</text>"
                "</g>\n") >= 0 &&
        write_svg_end(output) && fclose(output) == 0;
    return result;
}

static int write_list_svg(const char *path) {
    FILE *output = fopen(path, "w");
    if (output == NULL ||
        !write_svg_begin(output, 960, 340, "Cursor operations become list states",
                         "The same int_list operations shown by list-demo")) {
        if (output != NULL) {
            fclose(output);
        }
        return 0;
    }
    const char *labels[] = {"1. append 20", "2. insert 10 before cursor",
                            "3. insert 15 before cursor",
                            "4. erase 20; cursor moves back"};
    const char *states[] = {"[20]", "[10]  ↔  [20]", "[10]  ↔  [15]  ↔  [20]",
                            "[10]  ↔  [15]"};
    const unsigned cursor_x[] = {438, 390, 508, 508};
    for (size_t index = 0; index < 4; index++) {
        unsigned y = 98 + (unsigned)index * 56;
        if (fprintf(
                output,
                "<rect x=\"36\" y=\"%u\" width=\"888\" height=\"44\" rx=\"10\" "
                "fill=\"#172554\"/>"
                "<text x=\"58\" y=\"%u\" fill=\"#BFDBFE\" "
                "font-family=\"system-ui,sans-serif\" font-size=\"14\">%s</text>"
                "<text x=\"340\" y=\"%u\" fill=\"#F8FAFC\" font-family=\"monospace\" "
                "font-size=\"17\">%s</text>"
                "<path d=\"M%u %u l-7 -9 h14 z\" fill=\"#4ADE80\"/>"
                "<text x=\"%u\" y=\"%u\" text-anchor=\"middle\" fill=\"#86EFAC\" "
                "font-family=\"system-ui,sans-serif\" font-size=\"11\">cursor</text>",
                y, y + 19, labels[index], y + 31, states[index], cursor_x[index],
                y + 24, cursor_x[index], y + 42) < 0) {
            fclose(output);
            return 0;
        }
    }
    int result = write_svg_end(output) && fclose(output) == 0;
    return result;
}

static int svg_canvas(FILE *output, const sketch_canvas *canvas, unsigned origin_x,
                      unsigned origin_y, unsigned scale) {
    unsigned width = (unsigned)sketch_canvas_width(canvas);
    unsigned height = (unsigned)sketch_canvas_height(canvas);
    if (fprintf(output,
                "<rect x=\"%u\" y=\"%u\" width=\"%u\" height=\"%u\" rx=\"8\" "
                "fill=\"#020617\"/>\n",
                origin_x, origin_y, width * scale, height * scale) < 0) {
        return 0;
    }
    for (size_t y = 0; y < sketch_canvas_height(canvas); y++) {
        for (size_t x = 0; x < sketch_canvas_width(canvas); x++) {
            uint8_t value = (uint8_t)(UINT8_MAX - sketch_canvas_pixel(canvas, x, y));
            if (value == 0) {
                continue;
            }
            if (fprintf(output,
                        "<rect x=\"%u\" y=\"%u\" width=\"%u\" height=\"%u\" "
                        "fill=\"#%02X%02X%02X\"/>\n",
                        origin_x + (unsigned)x * scale, origin_y + (unsigned)y * scale,
                        scale, scale, value, value, value) < 0) {
                return 0;
            }
        }
    }
    return 1;
}

static int load_bytes(const char *path, uint8_t **bytes, size_t *length) {
    *bytes = NULL;
    *length = 0;
    FILE *input = fopen(path, "rb");
    if (input == NULL || fseek(input, 0, SEEK_END) != 0) {
        if (input != NULL) {
            fclose(input);
        }
        return 0;
    }
    long size = ftell(input);
    if (size < 0 || fseek(input, 0, SEEK_SET) != 0) {
        fclose(input);
        return 0;
    }
    *length = (size_t)size;
    *bytes = *length == 0 ? NULL : malloc(*length);
    int result = (*length == 0 || *bytes != NULL) &&
                 (*length == 0 || fread(*bytes, 1, *length, input) == *length) &&
                 fclose(input) == 0;
    if (!result) {
        free(*bytes);
        *bytes = NULL;
    }
    return result;
}

static int write_canvas_story_svg(const char *path, const char *title,
                                  const char *subtitle, const char *const *labels,
                                  sketch_canvas *const *frames, size_t count,
                                  unsigned columns, unsigned scale) {
    unsigned rows = (unsigned)((count + columns - 1) / columns);
    unsigned panel_width =
        (unsigned)sketch_canvas_width(frames[0]) * scale + PANEL_MARGIN * 2;
    unsigned panel_height = (unsigned)sketch_canvas_height(frames[0]) * scale + 70;
    unsigned width = columns * panel_width + PANEL_MARGIN;
    unsigned height = 82 + rows * panel_height + PANEL_MARGIN;
    FILE *output = fopen(path, "w");
    if (output == NULL || !write_svg_begin(output, width, height, title, subtitle)) {
        if (output != NULL) {
            fclose(output);
        }
        return 0;
    }
    for (size_t index = 0; index < count; index++) {
        unsigned column = (unsigned)(index % columns);
        unsigned row = (unsigned)(index / columns);
        unsigned x = PANEL_MARGIN + column * panel_width;
        unsigned y = 94 + row * panel_height;
        if (fprintf(output,
                    "<text x=\"%u\" y=\"%u\" fill=\"#CBD5E1\" "
                    "font-family=\"system-ui,sans-serif\" font-size=\"13\">%s</text>\n",
                    x, y, labels[index]) < 0 ||
            !svg_canvas(output, frames[index], x, y + 12, scale)) {
            fclose(output);
            return 0;
        }
    }
    int result = write_svg_end(output) && fclose(output) == 0;
    return result;
}

static int write_binary_story(const char *svg_path, const char *gif_path) {
    char bits[9];
    if (binary_encode_signed(-12, 8, bits, sizeof(bits)) != BINARY_OK ||
        !write_binary_svg(svg_path, bits)) {
        return 0;
    }
    sketch_canvas *frames[5] = {0};
    for (size_t index = 0; index < 5; index++) {
        frames[index] = sketch_canvas_create(FRAME_WIDTH, FRAME_HEIGHT);
        if (frames[index] == NULL) {
            for (size_t cleanup = 0; cleanup < index; cleanup++) {
                sketch_canvas_destroy(frames[cleanup]);
            }
            return 0;
        }
        draw_bit_frame(frames[index], bits, index * 2);
    }
    sketch_status status =
        sketch_write_gif((const sketch_canvas *const *)frames, 5, gif_path, 45, true);
    for (size_t index = 0; index < 5; index++) {
        sketch_canvas_destroy(frames[index]);
    }
    return status == SKETCH_OK;
}

static int write_list_story(const char *svg_path, const char *gif_path) {
    int_list *list = int_list_create();
    if (list == NULL || !int_list_insert_after(list, 20)) {
        int_list_destroy(list);
        return 0;
    }
    (void)int_list_first(list);
    int success = int_list_insert_before(list, 10);
    (void)int_list_last(list);
    success = success && int_list_insert_before(list, 15);
    (void)int_list_last(list);
    success = success && int_list_erase_to_previous(list);
    int_list_destroy(list);
    if (!success || !write_list_svg(svg_path)) {
        return 0;
    }
    const size_t nodes[] = {1, 2, 3, 2};
    const size_t cursors[] = {0, 0, 1, 1};
    sketch_canvas *frames[4] = {0};
    for (size_t index = 0; index < 4; index++) {
        frames[index] = sketch_canvas_create(FRAME_WIDTH, FRAME_HEIGHT);
        if (frames[index] == NULL) {
            for (size_t cleanup = 0; cleanup < index; cleanup++) {
                sketch_canvas_destroy(frames[cleanup]);
            }
            return 0;
        }
        draw_list_frame(frames[index], nodes[index], cursors[index]);
    }
    sketch_status status =
        sketch_write_gif((const sketch_canvas *const *)frames, 4, gif_path, 65, true);
    for (size_t index = 0; index < 4; index++) {
        sketch_canvas_destroy(frames[index]);
    }
    return status == SKETCH_OK;
}

static int write_sketch_story(const char *input_path, const char *svg_path,
                              const char *gif_path) {
    uint8_t *bytes = NULL;
    size_t length = 0;
    if (!load_bytes(input_path, &bytes, &length) || length == 0) {
        return 0;
    }
    size_t *draw_prefixes = malloc(length * sizeof(*draw_prefixes));
    if (draw_prefixes == NULL) {
        free(bytes);
        return 0;
    }
    size_t draw_count = 0;
    for (size_t index = 0; index < length; index++) {
        if (bytes[index] >> 6 == 2) {
            draw_prefixes[draw_count++] = index + 1;
        }
    }
    if (draw_count == 0) {
        free(draw_prefixes);
        free(bytes);
        return 0;
    }
    size_t count = draw_count < 6 ? draw_count : 6;
    sketch_canvas *frames[6] = {0};
    char label_storage[6][32] = {{0}};
    const char *labels[6] = {0};
    int success = 1;
    for (size_t index = 0; index < count; index++) {
        frames[index] = sketch_canvas_create(32, 20);
        size_t prefix =
            draw_prefixes[count == 1 ? 0 : index * (draw_count - 1) / (count - 1)];
        if (frames[index] == NULL ||
            sketch_decode_bytes(frames[index], bytes, prefix) != SKETCH_OK) {
            success = 0;
            break;
        }
        (void)snprintf(label_storage[index], sizeof(label_storage[index]),
                       "draw at byte %zu", prefix);
        labels[index] = label_storage[index];
    }
    if (success) {
        success =
            write_canvas_story_svg(
                svg_path, "A sketch stream executes one byte at a time",
                "Each panel is the decoded canvas after the indicated input prefix",
                labels, frames, count, 3, 7) &&
            sketch_write_gif((const sketch_canvas *const *)frames, count, gif_path, 55,
                             true) == SKETCH_OK;
    }
    for (size_t index = 0; index < count; index++) {
        sketch_canvas_destroy(frames[index]);
    }
    free(draw_prefixes);
    free(bytes);
    return success;
}

static int write_renderer_story(const char *svg_path, const char *gif_path) {
    sketch_canvas *story[4] = {0};
    const char *labels[] = {"frame 0 · A=0.00 B=0.00", "frame 8 · A=0.80 B=0.40",
                            "frame 16 · A=1.60 B=0.80", "frame 24 · A=2.40 B=1.20"};
    int success = 1;
    for (size_t index = 0; index < 4; index++) {
        story[index] = sketch_canvas_create(160, 60);
        if (story[index] == NULL ||
            donut_render_frame_mode(story[index], (float)index * 0.80F,
                                    (float)index * 0.40F,
                                    DONUT_INCREMENTAL) != DONUT_OK) {
            success = 0;
            break;
        }
    }
    sketch_canvas *animation[24] = {0};
    for (size_t index = 0; success && index < 24; index++) {
        animation[index] = sketch_canvas_create(160, 60);
        if (animation[index] == NULL ||
            donut_render_frame_mode(animation[index], (float)index * 0.10F,
                                    (float)index * 0.05F,
                                    DONUT_INCREMENTAL) != DONUT_OK) {
            success = 0;
        }
    }
    if (success) {
        success = write_canvas_story_svg(
                      svg_path, "A renderer produces a sequence of depth-tested frames",
                      "Incremental rotation updates the sampled surface without "
                      "per-sample trig calls",
                      labels, story, 4, 2, 3) &&
                  sketch_write_gif((const sketch_canvas *const *)animation, 24,
                                   gif_path, 5, true) == SKETCH_OK;
    }
    for (size_t index = 0; index < 4; index++) {
        sketch_canvas_destroy(story[index]);
    }
    for (size_t index = 0; index < 24; index++) {
        sketch_canvas_destroy(animation[index]);
    }
    return success;
}

static void print_usage(FILE *output) {
    fputs("usage: toolkit-visualize binary|list SVG_OUTPUT GIF_OUTPUT\n"
          "       toolkit-visualize sketch INPUT.sk SVG_OUTPUT GIF_OUTPUT\n"
          "       toolkit-visualize renderer SVG_OUTPUT GIF_OUTPUT\n",
          output);
}

int main(int argc, char *argv[]) {
    int success = 0;
    if (argc == 4 && strcmp(argv[1], "binary") == 0) {
        success = write_binary_story(argv[2], argv[3]);
    } else if (argc == 4 && strcmp(argv[1], "list") == 0) {
        success = write_list_story(argv[2], argv[3]);
    } else if (argc == 5 && strcmp(argv[1], "sketch") == 0) {
        success = write_sketch_story(argv[2], argv[3], argv[4]);
    } else if (argc == 4 && strcmp(argv[1], "renderer") == 0) {
        success = write_renderer_story(argv[2], argv[3]);
    } else {
        print_usage(stderr);
        return EXIT_FAILURE;
    }
    if (!success) {
        fputs("toolkit-visualize: could not generate visual story\n", stderr);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
