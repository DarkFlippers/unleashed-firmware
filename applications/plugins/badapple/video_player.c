#include <stdbool.h>

#include <furi.h>

#include "bad_apple.h"
#include "video_player.h"

#define TAG "video_player"

static void vp_decode_rle(BadAppleCtx* ctx);
static void vp_decode_delta(BadAppleCtx* ctx);

int vp_play_frame(BadAppleCtx* ctx) {
    // Check frame type
    // FURI_LOG_D(TAG, "Buffer offset: %04lx", ctx->file_buffer_offset);
    uint8_t b = bad_apple_read_byte(ctx);
    // FURI_LOG_D(TAG, "Frame byte: %02x", b);
    switch(b) {
    case 1: // PFrame (delta)
        vp_decode_delta(ctx);
        break;
    case 2: // IFrame (rle)
        vp_decode_rle(ctx);
        break;
    case 3: // DFrame (duplicate)
        break;
    case 4: // next page (not supported)
    case 5: // end
    default:
        return 0;
    }

    return 1;
}

static inline void vp_write_pixel(BadAppleCtx* ctx, bool color) {
    if(color) {
        // White
        ctx->framebuffer[ctx->frame_write_offset / 8] &= ~(1 << (ctx->frame_write_offset % 8));
    } else {
        // Black
        ctx->framebuffer[ctx->frame_write_offset / 8] |= (1 << (ctx->frame_write_offset % 8));
    }
    ++ctx->frame_write_offset;
}

static inline void vp_set_rect_fast(BadAppleCtx* ctx, int x, int y) {
    ctx->frame_write_offset = y * VIDEO_WIDTH + x;
}

static void vp_decode_rle(BadAppleCtx* ctx) {
    int i = 0;
    int repeat_byte = bad_apple_read_byte(ctx);

    // Set rect to video area
    vp_set_rect_fast(ctx, 0, 0);

    while(i < VIDEO_WIDTH * VIDEO_HEIGHT) {
        int count;
        unsigned pixels;
        int j;
        int b = bad_apple_read_byte(ctx);

        if(b == repeat_byte) {
            count = bad_apple_read_byte(ctx);
            if(count == 0) count = 256;
            pixels = bad_apple_read_byte(ctx);
        } else {
            count = 1;
            pixels = (unsigned)b;
        }

        for(j = 0; j < count; ++j) {
            bool out_color;
            int k;
            unsigned loop_pixels = pixels;

            for(k = 0; k < 8; ++k) {
                if(loop_pixels & 0x80)
                    out_color = COLOR_WHITE;
                else
                    out_color = COLOR_BLACK;

                vp_write_pixel(ctx, out_color);
                loop_pixels <<= 1;
                ++i;
            }
        }
    }
}

static void vp_decode_delta(BadAppleCtx* ctx) {
    int frame_header[(VIDEO_HEIGHT + 7) / 8];
    uint i;
    int fh_byte = 0;
    int fh_index = 0;

    for(i = 0; i < sizeof(frame_header) / sizeof(frame_header[0]); ++i) {
        frame_header[i] = bad_apple_read_byte(ctx);
    }

    for(i = 0; i < VIDEO_HEIGHT; ++i) {
        if(i % 8 == 0) fh_byte = frame_header[fh_index++];

        if(fh_byte & 0x80) {
            int j;
            int sl_byte = 0;

            for(j = 0; j < VIDEO_WIDTH;) {
                if(j % (8 * 8) == 0) sl_byte = bad_apple_read_byte(ctx);

                if(sl_byte & 0x80) {
                    unsigned out_color;
                    int k;
                    unsigned pixel_group = bad_apple_read_byte(ctx);

                    // Note: this needs to be revised for screen width not multiple of 8
                    vp_set_rect_fast(ctx, j, i);

                    for(k = 0; k < 8 && j < VIDEO_WIDTH; ++k, ++j) {
                        if(pixel_group & 0x80)
                            out_color = COLOR_WHITE;
                        else
                            out_color = COLOR_BLACK;

                        vp_write_pixel(ctx, out_color);
                        pixel_group <<= 1;
                    }
                } else {
                    j += 8;
                }
                sl_byte <<= 1;
            }
        }
        fh_byte <<= 1;
    }
}
