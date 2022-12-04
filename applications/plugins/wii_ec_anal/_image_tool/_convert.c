#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[]) {
    const unsigned char* pp = NULL;
    uint32_t pix = 0;
    int bit = 0;

    uint8_t b = 0;
    uint8_t bcnt = 0;

    unsigned int lcnt = 0;
    static const int lmax = 16; // max hex values per line

    uint8_t* buf = NULL;
    uint8_t* bp = NULL;
    unsigned int blen = 0;

    uint8_t* cmp = NULL;
    uint8_t* cp = NULL;
    unsigned int clen = 0;
    uint8_t ctag = 0xFF;
    uint32_t tag[256] = {0};
    uint32_t tmax = UINT32_MAX;

    unsigned int x, y, z;

    const char* name = argv[1];
    FILE* fh = fopen(argv[2], "wb");

    uint32_t white = 0xFF;

    int rv = 0; // assume success

    // allocate buffers
    blen = ((img.w * img.h) + 0x7) >> 3;
    bp = (buf = calloc(blen + 1, 1));
    cp = (cmp = calloc(blen + 4, 1));

    // sanity check
    if(!fh || !buf || !cmp) {
        printf("! fopen() or malloc() fail.\n");
        rv = 255;
        goto bail;
    }

    // Find white value
    for(x = 1; x < img.bpp; x++) white = (white << 8) | 0xFF;

    // build bit pattern
    // create the comment as we go
    for(pp = img.b, y = 0; y < img.h; y++) {
        fprintf(fh, "// ");
        for(x = 0; x < img.w; x++) {
            // read pixel
            for(pix = 0, z = 0; z < img.bpp; pix = (pix << 8) | *pp++, z++)
                ;
            // get bit and draw
            if(pix < white) {
                b = (b << 1) | 1;
                fprintf(fh, "##");
            } else {
                b <<= 1;
                fprintf(fh, "..");
            }
            // got byte
            if((++bcnt) == 8) {
                *bp++ = b;
                tag[b]++;
                bcnt = (b = 0);
            }
        }
        fprintf(fh, "\n");
    }
    fprintf(fh, "\n");
    // padding
    if(bcnt) {
        b <<= (bcnt = 8 - bcnt);
        *bp++ = b;
        tag[b]++;
    }
    // Kill the compression
    *bp = ~bp[-1]; // https://youtube.com/clip/Ugkx-JZIr16hETy7hz_H6yIdKPtxVe8C5w_V

    // Byte run length compression
    // Find a good tag
    for(x = 0; tmax && (x < 256); x++) {
        if(tag[x] < tmax) {
            tmax = tag[x];
            ctag = x;
        }
    }

    // compress the data
    for(bp = buf, x = 0; (clen < blen) && (x < blen); x++) {
        // need at least 4 the same to be worth it
        // must compress tag (if it occurs)
        if((bp[x] == bp[x + 1]) && (bp[x] == bp[x + 2]) && (bp[x] == bp[x + 3]) ||
           (bp[x] == ctag)) {
            for(y = 1; (y < 255) && (bp[x] == bp[x + y]); y++)
                ;
            *cp++ = ctag; // tag
            *cp++ = y; // length
            *cp++ = bp[x]; // byte
            x += y - 1;
            clen += 3;
        } else {
            *cp++ = bp[x];
            clen++;
        }
    }

    // create struct
    fprintf(fh, "#include \"images.h\"\n\n");
    fprintf(fh, "const image_t  img_%s = { %d, %d, ", name, img.w, img.h);

    if(clen < blen) { // dump compressed?
        fprintf(
            fh,
            "true, %d, 0x%02X, {  // orig:%d, comp:%.2f%%\n\t",
            clen,
            ctag,
            blen,
            100.0 - ((clen * 100.0) / blen));
        for(x = 0; x < clen; x++)
            if(x == clen - 1)
                fprintf(fh, "0x%02X\n}};\n", cmp[x]);
            else
                fprintf(fh, "0x%02X%s", cmp[x], (!((x + 1) % 16)) ? ",\n\t" : ", ");

    } else { // dump UNcompressed
        fprintf(fh, "false, %d, 0, {\n\t", blen);
        for(x = 0; x < blen; x++)
            if(x == blen - 1)
                fprintf(fh, "0x%02X\n}};\n", buf[x]);
            else
                fprintf(fh, "0x%02X%s", buf[x], (!((x + 1) % 16)) ? ",\n\t" : ", ");
    }

bail:
    if(fh) fclose(fh);
    if(buf) free(buf);
    if(cmp) free(cmp);

    return rv;
}
