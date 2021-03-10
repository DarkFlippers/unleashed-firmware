#pragma once

#include <stdint.h>
#include <gui/icon.h>
#include <assets_icons_i.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ColorWhite = 0x00,
    ColorBlack = 0x01,
} Color;

typedef enum {
    FontPrimary = 0x00,
    FontSecondary = 0x01,
    FontGlyph = 0x02,
    FontKeyboard = 0x03
} Font;

typedef enum {
    AlignLeft,
    AlignRight,
    AlignTop,
    AlignBottom,
    AlignCenter,
} Align;

typedef struct Canvas Canvas;

/*
 * Canvas width
 * @return width in pixels.
 */
uint8_t canvas_width(Canvas* canvas);

/*
 * Canvas height
 * @return height in pixels.
 */
uint8_t canvas_height(Canvas* canvas);

/*
 * Get current font height
 * @return height in pixels.
 */
uint8_t canvas_current_font_height(Canvas* canvas);

/*
 * Clear canvas, clear rendering buffer
 */
void canvas_clear(Canvas* canvas);

/*
 * Set drawing color
 */
void canvas_set_color(Canvas* canvas, Color color);

/*
 * Invert drawing color
 */
void canvas_invert_color(Canvas* canvas);

/*
 * Set drawing font
 */
void canvas_set_font(Canvas* canvas, Font font);

/*
 * Draw string at position of baseline defined by x, y.
 */
void canvas_draw_str(Canvas* canvas, uint8_t x, uint8_t y, const char* str);

/*
 * Draw aligned string defined by x, y.
 * Align calculated from position of baseline, string width and ascent (height of the glyphs above the baseline)
 */
void canvas_draw_str_aligned(
    Canvas* canvas,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical,
    const char* str);

/*
 * Get string width
 * @return width in pixels.
 */
uint16_t canvas_string_width(Canvas* canvas, const char* str);

/*
 * Draw stateful icon at position defined by x,y.
 */
void canvas_draw_icon(Canvas* canvas, uint8_t x, uint8_t y, Icon* icon);

/*
 * Draw stateless icon at position defined by x,y.
 */
void canvas_draw_icon_name(Canvas* canvas, uint8_t x, uint8_t y, IconName name);

/*
 * Draw xbm icon of width, height at position defined by x,y.
 */
void canvas_draw_xbm(
    Canvas* canvas,
    uint8_t x,
    uint8_t y,
    uint8_t w,
    uint8_t h,
    const uint8_t* bitmap);

/*
 * Draw dot at x,y
 */
void canvas_draw_dot(Canvas* canvas, uint8_t x, uint8_t y);

/*
 * Draw box of width, height at x,y
 */
void canvas_draw_box(Canvas* canvas, uint8_t x, uint8_t y, uint8_t width, uint8_t height);

/*
 * Draw frame of width, height at x,y
 */
void canvas_draw_frame(Canvas* canvas, uint8_t x, uint8_t y, uint8_t width, uint8_t height);

/*
 * Draw line from x1,y1 to x2,y2
 */
void canvas_draw_line(Canvas* canvas, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

/*
 * Draw circle at x,y with radius r
 */
void canvas_draw_circle(Canvas* canvas, uint8_t x, uint8_t y, uint8_t r);

/*
 * Draw disc at x,y with radius r
 */
void canvas_draw_disc(Canvas* canvas, uint8_t x, uint8_t y, uint8_t r);

/*
 * Draw glyph
 */
void canvas_draw_glyph(Canvas* canvas, uint8_t x, uint8_t y, uint16_t ch);

#ifdef __cplusplus
}
#endif
