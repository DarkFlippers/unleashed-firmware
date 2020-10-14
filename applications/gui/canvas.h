#pragma once

#include <stdint.h>
#include <u8g2.h>

#define COLOR_WHITE 0x00
#define COLOR_BLACK 0x01

#define CANVAS_FONT_PRIMARY u8g2_font_Born2bSportyV2_tr
#define CANVAS_FONT_SECONDARY u8g2_font_HelvetiPixel_tr

typedef struct Canvas Canvas;
typedef const uint8_t* font_t;

uint8_t canvas_width(Canvas* canvas);
uint8_t canvas_height(Canvas* canvas);

void canvas_clear(Canvas* canvas);

void canvas_color_set(Canvas* canvas, uint8_t color);

void canvas_font_set(Canvas* canvas, font_t font);

void canvas_str_draw(Canvas* canvas, uint8_t x, uint8_t y, const char* str);
