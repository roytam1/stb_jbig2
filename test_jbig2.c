#define STB_JBIG2_IMPLEMENTATION
#include "stb_jbig2.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    FILE *f;
    unsigned char *data;
    long size;
    int w, h;
    unsigned char *img;
    stb_jbig2_context *ctx;
    stb_jbig2_image *pimg;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file.jb2>\n", argv[0]);
        return 1;
    }

    f = fopen(argv[1], "rb");
    if (!f) { fprintf(stderr, "Cannot open %s\n", argv[1]); return 1; }
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    data = (unsigned char *)malloc(size);
    if (!data) { fclose(f); return 1; }
    fread(data, 1, size, f);
    fclose(f);

    printf("File size: %ld bytes\n", size);
    printf("First bytes: %02x %02x %02x %02x %02x %02x %02x %02x\n",
           data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);

    /* Test one-shot decode */
    img = stb_jbig2_decode(data, (int)size, &w, &h);
    if (img) {
        printf("One-shot decoded: %dx%d\n", w, h);
        stb_jbig2_free(img);
    } else {
        printf("One-shot decode failed\n");
    }

    /* Test streaming decode */
    ctx = stb_jbig2_create(NULL);
    if (ctx) {
        int rc = stb_jbig2_submit(ctx, data, (int)size);
        printf("Submit returned: %d\n", rc);
        pimg = stb_jbig2_page_out(ctx);
        if (pimg) {
            printf("Streaming decoded: %dx%d stride=%d\n",
                   stb_jbig2_image_width(pimg), stb_jbig2_image_height(pimg),
                   stb_jbig2_image_stride(pimg));
            stb_jbig2_release_page(ctx, pimg);
        } else {
            printf("No page available from streaming\n");
        }
        stb_jbig2_destroy(ctx);
    }

    free(data);
    return 0;
}
