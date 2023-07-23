#include <gui/gui.h>
#include <furi_hal.h>
#include "constants.h"
#include <doom_icons.h>
#include "assets.h"

#define CHECK_BIT(var, pos) ((var) & (1 << (pos)))

static const uint8_t bit_mask[8] = {128, 64, 32, 16, 8, 4, 2, 1};

#define pgm_read_byte(addr) (*(const unsigned char*)(addr))
#define read_bit(b, n) b& pgm_read_byte(bit_mask + n) ? 1 : 0
//#define read_bit(byte, index) (((unsigned)(byte) >> (index)) & 1)

void drawVLine(uint8_t x, int8_t start_y, int8_t end_y, uint8_t intensity, Canvas* const canvas);
void drawPixel(int8_t x, int8_t y, bool color, bool raycasterViewport, Canvas* const canvas);
void drawSprite(
    int8_t x,
    int8_t y,
    const uint8_t* bitmap,
    const uint8_t* bitmap_mask,
    int16_t w,
    int16_t h,
    uint8_t sprite,
    double distance,
    Canvas* const canvas);
void drawBitmap(
    int16_t x,
    int16_t y,
    const Icon* i,
    int16_t w,
    int16_t h,
    uint16_t color,
    Canvas* const canvas);
void drawTextSpace(int8_t x, int8_t y, char* txt, uint8_t space, Canvas* const canvas);
void drawChar(int8_t x, int8_t y, char ch, Canvas* const canvas);
void clearRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, Canvas* const canvas);
void drawGun(
    int16_t x,
    int16_t y,
    const uint8_t* bitmap,
    int16_t w,
    int16_t h,
    uint16_t color,
    Canvas* const canvas);
void drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, Canvas* const canvas);
void drawText(uint8_t x, uint8_t y, uint8_t num, Canvas* const canvas);
void fadeScreen(uint8_t intensity, bool color, Canvas* const canvas);
bool getGradientPixel(uint8_t x, uint8_t y, uint8_t i);
double getActualFps();
void fps();
uint8_t reverse_bits(uint8_t num);

// FPS control
double delta = 1;
uint32_t lastFrameTime = 0;
uint8_t zbuffer[128]; /// 128 = screen width & REMOVE WHEN DISPLAY.H IMPLEMENTED

void drawGun(
    int16_t x,
    int16_t y,
    const uint8_t* bitmap,
    int16_t w,
    int16_t h,
    uint16_t color,
    Canvas* const canvas) {
    int16_t byteWidth = (w + 7) / 8;
    uint8_t byte = 0;
    for(int16_t j = 0; j < h; j++, y++) {
        for(int16_t i = 0; i < w; i++) {
            if(i & 7)
                byte <<= 1;
            else
                byte = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
            if(byte & 0x80) drawPixel(x + i, y, color, false, canvas);
        }
    }
}

void drawVLine(uint8_t x, int8_t start_y, int8_t end_y, uint8_t intensity, Canvas* const canvas) {
    UNUSED(intensity);
    uint8_t dots = end_y - start_y;
    for(int i = 0; i < dots; i++) {
        canvas_draw_dot(canvas, x, start_y + i);
    }
}

void drawBitmap(
    int16_t x,
    int16_t y,
    const Icon* i,
    int16_t w,
    int16_t h,
    uint16_t color,
    Canvas* const canvas) {
    UNUSED(w);
    UNUSED(h);
    if(!color) {
        canvas_invert_color(canvas);
    }
    canvas_draw_icon(canvas, x, y, i);
    if(!color) {
        canvas_invert_color(canvas);
    }
}

void drawText(uint8_t x, uint8_t y, uint8_t num, Canvas* const canvas) {
    char buf[4];
    snprintf(buf, 4, "%d", num);
    drawTextSpace(x, y, buf, 1, canvas);
}

void drawTextSpace(int8_t x, int8_t y, char* txt, uint8_t space, Canvas* const canvas) {
    uint8_t pos = x;
    uint8_t i = 0;
    char ch;
    while((ch = txt[i]) != '\0') {
        drawChar(pos, y, ch, canvas);
        i++;
        pos += CHAR_WIDTH + space;

        // shortcut on end of screen
        if(pos > SCREEN_WIDTH) return;
    }
}

// Custom drawBitmap method with scale support, mask, zindex and pattern filling
void drawSprite(
    int8_t x,
    int8_t y,
    const uint8_t* bitmap,
    const uint8_t* bitmap_mask,
    int16_t w,
    int16_t h,
    uint8_t sprite,
    double distance,
    Canvas* const canvas) {
    uint8_t tw = (double)w / distance;
    uint8_t th = (double)h / distance;
    uint8_t byte_width = w / 8;
    uint8_t pixel_size = fmax(1, (double)1.0 / (double)distance);
    uint16_t sprite_offset = byte_width * h * sprite;

    bool pixel;
    bool maskPixel;

    // Don't draw the whole sprite if the anchor is hidden by z buffer
    // Not checked per pixel for performance reasons
    if(zbuffer[(int)(fmin(fmax(x, 0), ZBUFFER_SIZE - 1) / Z_RES_DIVIDER)] <
       distance * DISTANCE_MULTIPLIER) {
        return;
    }

    for(uint8_t ty = 0; ty < th; ty += pixel_size) {
        // Don't draw out of screen
        if(y + ty < 0 || y + ty >= RENDER_HEIGHT) {
            continue;
        }

        uint8_t sy = ty * distance; // The y from the sprite

        for(uint8_t tx = 0; tx < tw; tx += pixel_size) {
            uint8_t sx = tx * distance; // The x from the sprite
            uint16_t byte_offset = sprite_offset + sy * byte_width + sx / 8;

            // Don't draw out of screen
            if(x + tx < 0 || x + tx >= SCREEN_WIDTH) {
                continue;
            }

            maskPixel = read_bit(pgm_read_byte(bitmap_mask + byte_offset), sx % 8);

            if(maskPixel) {
                pixel = read_bit(pgm_read_byte(bitmap + byte_offset), sx % 8);
                for(uint8_t ox = 0; ox < pixel_size; ox++) {
                    for(uint8_t oy = 0; oy < pixel_size; oy++) {
                        if(bitmap == imp_inv)
                            drawPixel(x + tx + ox, y + ty + oy, 1, true, canvas);
                        else
                            drawPixel(x + tx + ox, y + ty + oy, pixel, true, canvas);
                    }
                }
            }
        }
    }
}

void drawPixel(int8_t x, int8_t y, bool color, bool raycasterViewport, Canvas* const canvas) {
    if(x < 0 || x >= SCREEN_WIDTH || y < 0 ||
       y >= (raycasterViewport ? RENDER_HEIGHT : SCREEN_HEIGHT)) {
        return;
    }
    if(color)
        canvas_draw_dot(canvas, x, y);
    else {
        canvas_invert_color(canvas);
        canvas_draw_dot(canvas, x, y);
        canvas_invert_color(canvas);
    }
}

void drawChar(int8_t x, int8_t y, char ch, Canvas* const canvas) {
    uint8_t lsb;
    uint8_t c = 0;
    while(CHAR_MAP[c] != ch && CHAR_MAP[c] != '\0') c++;
    for(uint8_t i = 0; i < 6; i++) {
        //lsb = (char_arr[c][i] >> 4);
        lsb = reverse_bits(char_arr[c][i]);
        for(uint8_t n = 0; n < 4; n++) {
            if(CHECK_BIT(lsb, n)) {
                drawPixel(x + n, y + i, true, false, canvas);
            }
        }
    }
}

void clearRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, Canvas* const canvas) {
    canvas_invert_color(canvas);

    for(int i = 0; i < w; i++) {
        for(int j = 0; j < h; j++) {
            canvas_draw_dot(canvas, x + i, y + j);
        }
    }

    canvas_invert_color(canvas);
}

void drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, Canvas* const canvas) {
    for(int i = 0; i < w; i++) {
        for(int j = 0; j < h; j++) {
            canvas_draw_dot(canvas, x + i, y + j);
        }
    }
}

bool getGradientPixel(uint8_t x, uint8_t y, uint8_t i) {
    if(i == 0) return 0;
    if(i >= GRADIENT_COUNT - 1) return 1;

    uint8_t index =
        fmax(0, fmin(GRADIENT_COUNT - 1, i)) * GRADIENT_WIDTH * GRADIENT_HEIGHT // gradient index
        + y * GRADIENT_WIDTH % (GRADIENT_WIDTH * GRADIENT_HEIGHT) // y byte offset
        + x / GRADIENT_HEIGHT % GRADIENT_WIDTH; // x byte offset
    //uint8_t *gradient_data = NULL;
    //furi_hal_compress_icon_decode(icon_get_data(&I_gradient_inv), &gradient_data);
    // return the bit based on x
    return read_bit(pgm_read_byte(gradient + index), x % 8);
}

void fadeScreen(uint8_t intensity, bool color, Canvas* const canvas) {
    for(uint8_t x = 0; x < SCREEN_WIDTH; x++) {
        for(uint8_t y = 0; y < SCREEN_HEIGHT; y++) {
            if(getGradientPixel(x, y, intensity)) drawPixel(x, y, color, false, canvas);
        }
    }
}

// Adds a delay to limit play to specified fps
// Calculates also delta to keep movement consistent in lower framerates
void fps() {
    while(furi_get_tick() - lastFrameTime < FRAME_TIME)
        ;
    delta = (double)(furi_get_tick() - lastFrameTime) / (double)FRAME_TIME;
    lastFrameTime = furi_get_tick();
}

double getActualFps() {
    return 1000 / ((double)FRAME_TIME * (double)delta);
}

uint8_t reverse_bits(uint8_t num) {
    unsigned int NO_OF_BITS = sizeof(num) * 8;
    uint8_t reverse_num = 0;
    uint8_t i;
    for(i = 0; i < NO_OF_BITS; i++) {
        if((num & (1 << i))) reverse_num |= 1 << ((NO_OF_BITS - 1) - i);
    }
    return reverse_num;
}