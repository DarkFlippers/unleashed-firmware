/**
 * @file canvas.h
 * GUI: Canvas API
 */

#pragma once

#include <stdint.h>
#include <gui/icon_animation.h>
#include <assets_icons.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Color enumeration */
typedef enum {
    ColorWhite = 0x00,
    ColorBlack = 0x01,
} Color;

/** Fonts enumeration */
typedef enum {
    FontPrimary,
    FontSecondary,
    FontKeyboard,
    FontBigNumbers,

    // Keep last for fonts number calculation
    FontTotalNumber,
} Font;

/** Alignment enumeration */
typedef enum {
    AlignLeft,
    AlignRight,
    AlignTop,
    AlignBottom,
    AlignCenter,
} Align;

/** Canvas Orientation */
typedef enum {
    CanvasOrientationHorizontal,
    CanvasOrientationVertical,
} CanvasOrientation;

/** Font Direction */
typedef enum {
    CanvasDirectionLeftToRight,
    CanvasDirectionTopToBottom,
    CanvasDirectionRightToLeft,
    CanvasDirectionBottomToTop,
} CanvasDirection;

/** Font parameters */
typedef struct {
    uint8_t leading_default;
    uint8_t leading_min;
    uint8_t height;
    uint8_t descender;
} CanvasFontParameters;

/** Canvas anonymous structure */
typedef struct Canvas Canvas;

/** Get Canvas width
 *
 * @param      canvas  Canvas instance
 *
 * @return     width in pixels.
 */
uint8_t canvas_width(Canvas* canvas);

/** Get Canvas height
 *
 * @param      canvas  Canvas instance
 *
 * @return     height in pixels.
 */
uint8_t canvas_height(Canvas* canvas);

/** Get current font height
 *
 * @param      canvas  Canvas instance
 *
 * @return     height in pixels.
 */
uint8_t canvas_current_font_height(Canvas* canvas);

/** Get font parameters
 *
 * @param      canvas  Canvas instance
 * @param      font    Font
 *
 * @return     pointer to CanvasFontParameters structure
 */
CanvasFontParameters* canvas_get_font_params(Canvas* canvas, Font font);

/** Clear canvas
 *
 * @param      canvas  Canvas instance
 */
void canvas_clear(Canvas* canvas);

/** Set drawing color
 *
 * @param      canvas  Canvas instance
 * @param      color   Color
 */
void canvas_set_color(Canvas* canvas, Color color);

/** Set font swap
 * Argument String Rotation Description
 *
 * @param      canvas  Canvas instance
 * @param      dir     Direction font
 */
void canvas_set_font_direction(Canvas* canvas, CanvasDirection dir);

/** Invert drawing color
 *
 * @param      canvas  Canvas instance
 */
void canvas_invert_color(Canvas* canvas);

/** Set drawing font
 *
 * @param      canvas  Canvas instance
 * @param      font    Font
 */
void canvas_set_font(Canvas* canvas, Font font);

/** Draw string at position of baseline defined by x, y.
 *
 * @param      canvas  Canvas instance
 * @param      x       anchor point x coordinate
 * @param      y       anchor point y coordinate
 * @param      str     C-string
 */
void canvas_draw_str(Canvas* canvas, uint8_t x, uint8_t y, const char* str);

/** Draw aligned string defined by x, y.
 *
 * Align calculated from position of baseline, string width and ascent (height
 * of the glyphs above the baseline)
 *
 * @param      canvas      Canvas instance
 * @param      x           anchor point x coordinate
 * @param      y           anchor point y coordinate
 * @param      horizontal  horizontal alignment
 * @param      vertical    vertical alignment
 * @param      str         C-string
 */
void canvas_draw_str_aligned(
    Canvas* canvas,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical,
    const char* str);

/** Get string width
 *
 * @param      canvas  Canvas instance
 * @param      str     C-string
 *
 * @return     width in pixels.
 */
uint16_t canvas_string_width(Canvas* canvas, const char* str);

/** Get glyph width
 *
 * @param      canvas  Canvas instance
 * @param[in]  symbol  character
 *
 * @return     width in pixels
 */
uint8_t canvas_glyph_width(Canvas* canvas, char symbol);

/** Draw bitmap picture at position defined by x,y.
 *
 * @param      canvas                   Canvas instance
 * @param      x                        x coordinate
 * @param      y                        y coordinate
 * @param      width                    width of bitmap
 * @param      height                   height of bitmap
 * @param      compressed_bitmap_data   compressed bitmap data
 */
void canvas_draw_bitmap(
    Canvas* canvas,
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height,
    const uint8_t* compressed_bitmap_data);

/** Draw animation at position defined by x,y.
 *
 * @param      canvas          Canvas instance
 * @param      x               x coordinate
 * @param      y               y coordinate
 * @param      icon_animation  IconAnimation instance
 */
void canvas_draw_icon_animation(
    Canvas* canvas,
    uint8_t x,
    uint8_t y,
    IconAnimation* icon_animation);

/** Draw icon at position defined by x,y.
 *
 * @param      canvas  Canvas instance
 * @param      x       x coordinate
 * @param      y       y coordinate
 * @param      icon    Icon instance
 */
void canvas_draw_icon(Canvas* canvas, uint8_t x, uint8_t y, const Icon* icon);

/** Draw XBM bitmap
 *
 * @param      canvas  Canvas instance
 * @param      x       x coordinate
 * @param      y       y coordinate
 * @param      w       bitmap width
 * @param      h       bitmap height
 * @param      bitmap  pointer to XBM bitmap data
 */
void canvas_draw_xbm(
    Canvas* canvas,
    uint8_t x,
    uint8_t y,
    uint8_t w,
    uint8_t h,
    const uint8_t* bitmap);

/** Draw dot at x,y
 *
 * @param      canvas  Canvas instance
 * @param      x       x coordinate
 * @param      y       y coordinate
 */
void canvas_draw_dot(Canvas* canvas, uint8_t x, uint8_t y);

/** Draw box of width, height at x,y
 *
 * @param      canvas  Canvas instance
 * @param      x       x coordinate
 * @param      y       y coordinate
 * @param      width   box width
 * @param      height  box height
 */
void canvas_draw_box(Canvas* canvas, uint8_t x, uint8_t y, uint8_t width, uint8_t height);

/** Draw frame of width, height at x,y
 *
 * @param      canvas  Canvas instance
 * @param      x       x coordinate
 * @param      y       y coordinate
 * @param      width   frame width
 * @param      height  frame height
 */
void canvas_draw_frame(Canvas* canvas, uint8_t x, uint8_t y, uint8_t width, uint8_t height);

/** Draw line from x1,y1 to x2,y2
 *
 * @param      canvas  Canvas instance
 * @param      x1      x1 coordinate
 * @param      y1      y1 coordinate
 * @param      x2      x2 coordinate
 * @param      y2      y2 coordinate
 */
void canvas_draw_line(Canvas* canvas, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

/** Draw circle at x,y with radius r
 *
 * @param      canvas  Canvas instance
 * @param      x       x coordinate
 * @param      y       y coordinate
 * @param      r       radius
 */
void canvas_draw_circle(Canvas* canvas, uint8_t x, uint8_t y, uint8_t r);

/** Draw disc at x,y with radius r
 *
 * @param      canvas  Canvas instance
 * @param      x       x coordinate
 * @param      y       y coordinate
 * @param      r       radius
 */
void canvas_draw_disc(Canvas* canvas, uint8_t x, uint8_t y, uint8_t r);

/** Draw triangle with given base and height lengths and their intersection coordinate
 *
 * @param       canvas  Canvas instance
 * @param       x       x coordinate of base and height intersection
 * @param       y       y coordinate of base and height intersection
 * @param       base    length of triangle side
 * @param       height  length of triangle height
 * @param       dir     CanvasDirection triangle orientation
 */
void canvas_draw_triangle(
    Canvas* canvas,
    uint8_t x,
    uint8_t y,
    uint8_t base,
    uint8_t height,
    CanvasDirection dir);

/** Draw glyph
 *
 * @param      canvas  Canvas instance
 * @param      x       x coordinate
 * @param      y       y coordinate
 * @param      ch      character
 */
void canvas_draw_glyph(Canvas* canvas, uint8_t x, uint8_t y, uint16_t ch);

/** Set transparency mode
 *
 * @param      canvas  Canvas instance
 * @param      alpha   transparency mode
 */
void canvas_set_bitmap_mode(Canvas* canvas, bool alpha);

/** Draw rounded-corner frame of width, height at x,y, with round value radius
 *
 * @param      canvas  Canvas instance
 * @param      x       x coordinate
 * @param      y       y coordinate
 * @param      width   frame width
 * @param      height  frame height
 * @param      radius  frame corner radius
 */
void canvas_draw_rframe(
    Canvas* canvas,
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height,
    uint8_t radius);

/** Draw rounded-corner box of width, height at x,y, with round value raduis
 *
 * @param      canvas  Canvas instance
 * @param      x       x coordinate
 * @param      y       y coordinate
 * @param      width   box width
 * @param      height  box height
 * @param      radius  box corner radius
 */
void canvas_draw_rbox(
    Canvas* canvas,
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height,
    uint8_t radius);

#ifdef __cplusplus
}
#endif
