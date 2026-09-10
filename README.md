# stb_jbig2

A single-file JBIG2 decoder library written in C89, following the [stb](https://github.com/nothings/stb) single-file library convention.

## What is this?

`stb_jbig2.h` decodes JBIG2 (ITU-T T.88) bitstreams into 1-bit monochrome images. JBIG2 is commonly used in PDF files for compressing scanned documents and faxes.

## Features

- **Single header** -- just `#define STB_JBIG2_IMPLEMENTATION` before including
- **C89** -- compatible with any C89 compiler
- **No dependencies** -- only needs `<stdlib.h>` and `<string.h>`
- **All JBIG2 segment types** -- arithmetic coding, text regions, symbol dictionaries, halftone, pattern dictionaries, refinement regions, generic regions
- **Random-access format** -- supports both sequential and random-access JBIG2 files
- **Multi-page** -- handles multiple pages in a single file
- **Pixel-perfect** -- matches jbig2dec-0.20 output on all tested files

## Usage

```c
#define STB_JBIG2_IMPLEMENTATION
#include "stb_jbig2.h"

/* Decode a complete JBIG2 file */
int width, height;
unsigned char *pixels = stb_jbig2_decode(data, data_len, &width, &height);
if (pixels) {
    /* pixels is 1bpp, MSB-first, row stride = (width+7)/8 */
    process_image(pixels, width, height);
    stb_jbig2_free(pixels);
}
```

### Incremental decoding (streaming)

```c
stb_jbig2_context *ctx = stb_jbig2_create(NULL);
stb_jbig2_submit(ctx, chunk1, len1);
stb_jbig2_submit(ctx, chunk2, len2);

stb_jbig2_image *page;
while ((page = stb_jbig2_page_out(ctx)) != NULL) {
    int w = stb_jbig2_image_width(page);
    int h = stb_jbig2_image_height(page);
    unsigned char *data = stb_jbig2_image_data(page);
    /* ... */
    stb_jbig2_release_page(ctx, page);
}
stb_jbig2_destroy(ctx);
```

### Image access

```c
int w = stb_jbig2_image_width(img);    /* pixels */
int h = stb_jbig2_image_height(img);   /* pixels */
int s = stb_jbig2_image_stride(img);   /* bytes per row */
unsigned char *d = stb_jbig2_image_data(img); /* raw 1bpp data */
int pixel = stb_jbig2_image_getpixel(img, x, y); /* 0 or 1 */
```

## Building

```bash
gcc -std=c89 -Wall -O3 -o test_jbig2 test_jbig2.c -lm
./test_jbig2 images/003.jb2
```

Or with MSVC:

```bash
cl /W4 /O2 test_jbig2.c
```

`-O3` is recommended for performance -- it enables inlining of the arithmetic decoder hot path.

## API Reference

### One-shot decode

```c
unsigned char *stb_jbig2_decode(const unsigned char *data, int size, int *width, int *height);
```

Decode an entire JBIG2 file. Returns a newly allocated 1bpp bitmap (MSB-first), or `NULL` on failure. Free with `stb_jbig2_free()`.

### Streaming decode

```c
stb_jbig2_context *stb_jbig2_create(stb_jbig2_context *shared);
stb_jbig2_context *stb_jbig2_create_ex(int options, stb_jbig2_context *shared);
int stb_jbig2_submit(stb_jbig2_context *ctx, const unsigned char *data, int size);
stb_jbig2_image *stb_jbig2_page_out(stb_jbig2_context *ctx);
int stb_jbig2_complete_page(stb_jbig2_context *ctx);
void stb_jbig2_release_page(stb_jbig2_context *ctx, stb_jbig2_image *img);
void stb_jbig2_destroy(stb_jbig2_context *ctx);
```

- `shared` -- share arithmetic state between contexts (for parallel decoding), or `NULL`
- `options` -- `STB_JBIG2_OPTION_EMBEDDED` for embedded JBIG2 data (no file header)
- `stb_jbig2_submit()` returns 0 on success
- `stb_jbig2_page_out()` returns the next complete page, or `NULL` when no more pages are ready
- `stb_jbig2_complete_page()` forces the current page to complete (useful for end-of-data)

### Image operations

```c
int stb_jbig2_image_width(stb_jbig2_image *img);
int stb_jbig2_image_height(stb_jbig2_image *img);
int stb_jbig2_image_stride(stb_jbig2_image *img);
unsigned char *stb_jbig2_image_data(stb_jbig2_image *img);
int stb_jbig2_image_getpixel(stb_jbig2_image *img, int x, int y);
void stb_jbig2_free(void *p);
```

## Quality

Tested against jbig2dec-0.20 (Artifex reference implementation):

| File | Pixels Differ | % |
|------|--------------|---|
| 002_page16 | 0 / 7,906,560 | 0.0000% |
| 003 | 0 / 8,427,750 | 0.0000% |
| 005 | 0 / 8,471,520 | 0.0000% |
| 006_page0 | 0 / 8,329,152 | 0.0000% |
| amb_1 | 0 / 960,000 | 0.0000% |
| text | 0 / 3,475,425 | 0.0000% |

## Limitations

- Only decodes JBIG2 (not JBIG2 in PDF containers -- extract the JBIG2 stream first)
- 1-bit output only (no grayscale or color)
- Requires `-O3` for good performance (inlining is critical)

## License

Dual-licensed under The Unlicense (public domain) or MIT License -- see end of `stb_jbig2.h` for full text.

## Credits

Built by studying [jbig2dec-0.20](https://artifex.com/projects/jbig2dec/) (Artifex Software, AGPLv3).
