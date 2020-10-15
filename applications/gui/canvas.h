#pragma once

#include <stdint.h>

typedef enum {
	ColorWhite = 0x00,
	ColorBlack = 0x01,
} Color;

typedef struct Canvas Canvas;
typedef const uint8_t* Font;

struct _CanvasApi;

typedef struct _CanvasApi CanvasApi;

typedef struct {
	Font primary;
	Font secondary;
} Fonts;

struct {
    Canvas canvas;
    Fonts* fonts;
    
    uint8_t (*width)(CanvasApi* canvas);
    uint8_t (*height)(CanvasApi* canvas);

    void (*clear)(CanvasApi* canvas);

    void (*canvas_color_set)(CanvasApi* canvas, Color color);
    void (*canvas_font_set)(CanvasApi* canvas, Font font);

    void (*draw_str)(CanvasApi* canvas, uint8_t x, uint8_t y, const char* str);
} _CanvasApi;
