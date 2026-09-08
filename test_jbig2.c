#define STB_JBIG2_IMPLEMENTATION
#define STB_JBIG2_DEBUG
#include "stb_jbig2.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    FILE *f;
    unsigned char *data;
    long size;
    stb_jbig2_context *ctx;
    stb_jbig2_image *pimg;
    int page_count = 0;

    if (argc < 2) { fprintf(stderr, "Usage: %s <file.jb2>\n", argv[0]); return 1; }
    f = fopen(argv[1], "rb");
    if (!f) { fprintf(stderr, "Cannot open %s\n", argv[1]); return 1; }
    fseek(f, 0, SEEK_END); size = ftell(f); fseek(f, 0, SEEK_SET);
    data = (unsigned char *)malloc(size);
    if (!data) { fclose(f); return 1; }
    fread(data, 1, size, f); fclose(f);

    fprintf(stderr, "File: %s, size: %ld\n", argv[1], size);

    ctx = stb_jbig2_create(NULL);
    if (!ctx) { fprintf(stderr, "Failed to create context\n"); free(data); return 1; }

    {
        int rc = stb_jbig2_submit(ctx, data, (int)size);
        fprintf(stderr, "Submit returned: %d\n", rc);
    }

    while ((pimg = stb_jbig2_page_out(ctx)) != NULL) {
        fprintf(stderr, "Page %d: %dx%d\n", page_count,
               stb_jbig2_image_width(pimg), stb_jbig2_image_height(pimg));
        stb_jbig2_release_page(ctx, pimg);
        page_count++;
    }
    fprintf(stderr, "Total pages: %d\n", page_count);

    stb_jbig2_destroy(ctx);
    free(data);
    return 0;
}
