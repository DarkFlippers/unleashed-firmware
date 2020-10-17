#pragma once

#include <stdint.h>
#include <u8g2.h>

typedef enum {
    ColorWhite = 0x00,
    ColorBlack = 0x01,
} Color;

typedef enum {
    FontPrimary = 0x00,
    FontSecondary = 0x01,
} Font;

typedef struct CanvasApi CanvasApi;
struct CanvasApi {
    uint8_t (*width)(CanvasApi* canvas);
    uint8_t (*height)(CanvasApi* canvas);

    void (*clear)(CanvasApi* canvas);

    void (*set_color)(CanvasApi* canvas, Color color);
    void (*set_font)(CanvasApi* canvas, Font font);

    void (*draw_str)(CanvasApi* canvas, uint8_t x, uint8_t y, const char* str);
};
