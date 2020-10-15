#pragma once

#include <stdint.h>
#include <u8g2.h>

typedef enum {
    ColorWhite = 0x00,
    ColorBlack = 0x01,
} Color;

typedef const uint8_t* Font;

typedef struct {
    Font primary;
    Font secondary;
} Fonts;

struct _CanvasApi;

typedef struct _CanvasApi CanvasApi;

// Canvas is private but we need its declaration here
typedef struct {
    u8g2_t fb;
    uint8_t offset_x;
    uint8_t offset_y;
    uint8_t width;
    uint8_t height;
} Canvas;

struct _CanvasApi {
    Canvas canvas;
    Fonts* fonts;

    uint8_t (*width)(CanvasApi* canvas);
    uint8_t (*height)(CanvasApi* canvas);

    void (*clear)(CanvasApi* canvas);

    void (*set_color)(CanvasApi* canvas, Color color);
    void (*set_font)(CanvasApi* canvas, Font font);

    void (*draw_str)(CanvasApi* canvas, uint8_t x, uint8_t y, const char* str);
};
