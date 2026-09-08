#define STB_JBIG2_IMPLEMENTATION
#include "stb_jbig2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void write_ppm(const char *path, stb_jbig2_image *img) {
    FILE *f; int x, y, w, h, stride;
    unsigned char *row;
    f = fopen(path, "wb");
    if (!f) { fprintf(stderr, "Cannot write %s\n", path); return; }
    w = stb_jbig2_image_width(img);
    h = stb_jbig2_image_height(img);
    stride = stb_jbig2_image_stride(img);
    row = stb_jbig2_image_data(img);
    fprintf(f, "P6\n%d %d\n255\n", w, h);
    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            int bit = (row[(y * stride) + (x >> 3)] >> (7 - (x & 7))) & 1;
            unsigned char c = bit ? 0 : 255;
            fwrite(&c, 1, 1, f);
            fwrite(&c, 1, 1, f);
            fwrite(&c, 1, 1, f);
        }
    }
    fclose(f);
    fprintf(stderr, "Wrote %s (%dx%d)\n", path, w, h);
}

int main(int argc, char **argv) {
    FILE *f;
    unsigned char *data;
    long size;
    stb_jbig2_context *ctx;
    stb_jbig2_image *pimg;
    int page_count = 0;
    clock_t start, submit_done;
    const char *inpath;
    char outpath[512];

    if (argc < 2) { fprintf(stderr, "Usage: %s <file.jb2>\n", argv[0]); return 1; }
    inpath = argv[1];
    f = fopen(inpath, "rb");
    if (!f) { fprintf(stderr, "Cannot open %s\n", inpath); return 1; }
    fseek(f, 0, SEEK_END); size = ftell(f); fseek(f, 0, SEEK_SET);
    data = (unsigned char *)malloc(size);
    if (!data) { fclose(f); return 1; }
    fread(data, 1, size, f); fclose(f);

    fprintf(stderr, "File: %s, size: %ld\n", inpath, size);

    ctx = stb_jbig2_create(NULL);
    if (!ctx) { fprintf(stderr, "Failed\n"); free(data); return 1; }

    start = clock();
    {
        int rc = stb_jbig2_submit(ctx, data, (int)size);
        submit_done = clock();
        fprintf(stderr, "Submit returned: %d (took %.3f sec)\n", rc,
               (double)(submit_done - start) / CLOCKS_PER_SEC);
    }

    while ((pimg = stb_jbig2_page_out(ctx)) != NULL) {
        fprintf(stderr, "Page %d: %dx%d\n", page_count,
               stb_jbig2_image_width(pimg), stb_jbig2_image_height(pimg));
        /* strip .jb2 extension, write _pageN.ppm */
        {
            size_t len = strlen(inpath);
            if (len > 4 && (strcmp(inpath + len - 4, ".jb2") == 0 || strcmp(inpath + len - 4, ".JB2") == 0))
                len -= 4;
            if (len + 20 < sizeof(outpath)) {
                memcpy(outpath, inpath, len);
                sprintf(outpath + len, "_page%d.ppm", page_count);
            } else {
                sprintf(outpath, "page%d.ppm", page_count);
            }
        }
        write_ppm(outpath, pimg);
        stb_jbig2_release_page(ctx, pimg);
        page_count++;
    }
    fprintf(stderr, "Total pages: %d\n", page_count);

    stb_jbig2_destroy(ctx);
    free(data);
    return 0;
}
