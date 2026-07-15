<div align="center">

# C23 Systems Toolkit

### A portable C23 CLI toolkit for exact binary data, cursor-based containers, and compact raster sketches.

[![CI](https://github.com/ReverseZoom2151/c23-systems-toolkit/actions/workflows/ci.yml/badge.svg)](https://github.com/ReverseZoom2151/c23-systems-toolkit/actions/workflows/ci.yml)

</div>

`c23-systems-toolkit` is a buildable C23 project with three focused systems
programming domains. Each has a small public API, a terminal-friendly boundary,
and executable regression tests. The project treats invalid input, integer
widths, memory ownership, and file output as explicit parts of its contract.

| Domain | What you can do | Systems idea |
| --- | --- | --- |
| **Binary** | Encode and decode exact signed or unsigned 8-, 16-, 32-, and 64-bit values | Two's-complement representation and range checking |
| **Lists** | Reuse a cursor-based circular doubly linked list of integers | Sentinel nodes, pointer invariants, and ownership |
| **Sketches** | Decode compact byte streams into terminal ASCII previews or PGM files | Bit-packed commands, rasterisation, clipping, and file I/O |

## Run it

You need a C23-capable compiler, [CMake](https://cmake.org/) 3.20+, and
optionally [Ninja](https://ninja-build.org/). [Valgrind](https://valgrind.org/)
is used by the complete local verification target.

```bash
git clone https://github.com/ReverseZoom2151/c23-systems-toolkit.git
cd c23-systems-toolkit
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

Convert a signed value to its exact two's-complement form, then decode it:

```bash
./build/binary-tool encode i8 -12
# 11110100

./build/binary-tool decode i8 11110100
# -12
```

Use the sketch tools on a compact `.sk` stream:

```bash
./build/sketch-view drawing.sk 80 40
./build/sketch-pgm drawing.sk drawing.pgm 320 240
```

The generated PGM is deliberately simple and portable: it can be inspected by
image tools, converted elsewhere, or treated as raw grayscale output.

## Why these modules belong together

The executables are intentionally thin. Parsing and terminal/file I/O live at
the edge; reusable code in `src/` exposes explicit values and status results.
That makes fixed-width behaviour testable, pointer ownership local, and binary
formats usable without a GUI framework.

For example, the binary API takes an explicit bit width and reports why a
request could not be represented instead of silently truncating a value:

```c
binary_status binary_encode_signed(
    int64_t value, unsigned bits, char *output, size_t output_size);
```

The list remains opaque to callers. Its cursor is either on an item or at an
end sentinel, so navigation and mutation have observable, safe boundary
states:

```c
int_list *items = int_list_create();
int_list_insert_after(items, 20);
int_list_insert_before(items, 10);

int_list_first(items);       // cursor → 10
int_list_next(items);        // cursor → 20
int_list_destroy(items);
```

The sketch decoder has the same compact boundary: each input byte is a two-bit
opcode plus a six-bit operand. `DATA` accumulates parameters; movement and
tool commands consume them to produce clipped line or block raster operations.

```c
sketch_status sketch_decode_bytes(
    sketch_canvas *canvas, const uint8_t *bytes, size_t length);
```

This keeps the format decoder independent from the output choice: the same
canvas can be written as ASCII or binary PGM.

## Command reference

```text
binary-tool encode TYPE VALUE
binary-tool decode TYPE BITS
sketch-view INPUT.sk [WIDTH HEIGHT]
sketch-pgm INPUT.sk OUTPUT.pgm [WIDTH HEIGHT]
```

`TYPE` is one of `i8`, `u8`, `i16`, `u16`, `i32`, `u32`, `i64`, or `u64`.
Signed decoding uses two's complement; unsigned decoding rejects negative or
out-of-range values. Sketch output defaults to an 80 × 40 canvas when no size
is supplied. The complete, implementation-backed stream specification is in
[`docs/SKETCH_FORMAT.md`](docs/SKETCH_FORMAT.md).

## Project layout

```text
app/                    Command-line boundaries
include/                Public headers: binary, list, sketch, toolkit
src/                    Reusable C23 implementations
test/                   Executable regression tests
docs/                   Architecture and sketch-stream format notes
resources/original/     Preserved source material
.github/workflows/      Continuous-integration quality gate
```

## Quality bar

```bash
make build
make test
make check
```

The build applies `-Wall -Wextra -Wpedantic -Werror` to library consumers and
targets. The regression suite covers signed and unsigned binary boundaries,
list traversal/insertion/erasure/sentinel states, and sketch line, block,
malformed-input, and clipping behaviour.

`make check` runs every executable test under Valgrind. GitHub Actions repeats
the strict build and test suite on every push and pull request, runs Valgrind
against all three test executables, and performs a separate
AddressSanitizer/UndefinedBehaviorSanitizer build.

## Design notes

The toolkit favours small ownership-aware C modules and thin command-line
boundaries. [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) explains that split;
[`docs/SKETCH_FORMAT.md`](docs/SKETCH_FORMAT.md) documents the compact rendering
format.

---

The name **C23 Systems Toolkit** is intentional: it describes a growing group
of complete, reusable C programs without tying the repository to one data
structure, format, or historical course.
