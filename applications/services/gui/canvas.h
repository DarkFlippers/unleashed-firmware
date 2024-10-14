/**
 * @file canvas.h
 * GUI: Canvas API
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <gui/icon_animation.h>
#include <gui/icon.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Color enumeration */
typedef enum {
    ColorWhite = 0x00,
    ColorBlack = 0x01,
    ColorXOR = 0x02,
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
    CanvasOrientationHorizontalFlip,
    CanvasOrientationVertical,
    CanvasOrientationVerticalFlip,
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

/** Icon flip */
typedef enum {
    IconFlipNone,
    IconFlipHorizontal,
    IconFlipVertical,
    IconFlipBoth,
} IconFlip;

/** Icon rotation */
typedef enum {
    IconRotation0,
    IconRotation90,
    IconRotation180,
    IconRotation270,
} IconRotation;

/** Canvas anonymous structure */
typedef struct Canvas Canvas;

/** Reset canvas drawing tools configuration
 *
 * @param      canvas  Canvas instance
 */
void canvas_reset(Canvas* canvas);

/** Commit canvas. Send buffer to display
 *
 * @param      canvas  Canvas instance
 */
void canvas_commit(Canvas* canvas);

/** Get Canvas width
 *
 * @param      canvas  Canvas instance
 *
 * @return     width in pixels.
 */
size_t canvas_width(const Canvas* canvas);

/** Get Canvas height
 *
 * @param      canvas  Canvas instance
 *
 * @return     height in pixels.
 */
size_t canvas_height(const Canvas* canvas);

/** Get current font height
 *
 * @param      canvas  Canvas instance
 *
 * @return     height in pixels.
 */
size_t canvas_current_font_height(const Canvas* canvas);

/** Get font parameters
 *
 * @param      canvas  Canvas instance
 * @param      font    Font
 *
 * @return     pointer to CanvasFontParameters structure
 */
const CanvasFontParameters* canvas_get_font_params(const Canvas* canvas, Font font);

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

/** Set font swap Argument String Rotation Description
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

/** Set custom drawing font
 *
 * @param      canvas  Canvas instance
 * @param      font    Pointer to u8g2 const uint8_t* font array
 */
void canvas_set_custom_u8g2_font(Canvas* canvas, const uint8_t* font);

/** Draw string at position of baseline defined by x, y.
 *
 * @param      canvas  Canvas instance
 * @param      x       anchor point x coordinate
 * @param      y       anchor point y coordinate
 * @param      str     C-string
 */
void canvas_draw_str(Canvas* canvas, int32_t x, int32_t y, const char* str);

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
    int32_t x,
    int32_t y,
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
size_t canvas_glyph_width(Canvas* canvas, uint16_t symbol);

/** Draw bitmap picture at position defined by x,y.
 *
 * @param      canvas                  Canvas instance
 * @param      x                       x coordinate
 * @param      y                       y coordinate
 * @param      width                   width of bitmap
 * @param      height                  height of bitmap
 * @param      compressed_bitmap_data  compressed bitmap data
 */
void canvas_draw_bitmap(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    size_t width,
    size_t height,
    const uint8_t* compressed_bitmap_data);

/** Draw icon at position defined by x,y with rotation and flip.
 *
 * @param      canvas    Canvas instance
 * @param      x         x coordinate
 * @param      y         y coordinate
 * @param      icon      Icon instance
 * @param      rotation  IconRotation
 */
void canvas_draw_icon_ex(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    const Icon* icon,
    IconRotation rotation);

/** Draw animation at position defined by x,y.
 *
 * @param      canvas          Canvas instance
 * @param      x               x coordinate
 * @param      y               y coordinate
 * @param      icon_animation  IconAnimation instance
 */
void canvas_draw_icon_animation(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    IconAnimation* icon_animation);

/** Draw icon at position defined by x,y.
 *
 * @param      canvas  Canvas instance
 * @param      x       x coordinate
 * @param      y       y coordinate
 * @param      icon    Icon instance
 */
void canvas_draw_icon(Canvas* canvas, int32_t x, int32_t y, const Icon* icon);

/** Draw XBM bitmap
 *
 * @param      canvas  Canvas instance
 * @param      x       x coordinate
 * @param      y       y coordinate
 * @param[in]  width   bitmap width
 * @param[in]  height  bitmap height
 * @param      bitmap  pointer to XBM bitmap data
 */
void canvas_draw_xbm(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    size_t width,
    size_t height,
    const uint8_t* bitmap);

/** Draw rotated XBM bitmap
 *
 * @param      canvas       Canvas instance
 * @param      x            x coordinate
 * @param      y            y coordinate
 * @param[in]  width        bitmap width
 * @param[in]  height       bitmap height
 * @param[in]  rotation     bitmap rotation
 * @param      bitmap_data  pointer to XBM bitmap data
 */
void canvas_draw_xbm_ex(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    size_t width,
    size_t height,
    IconRotation rotation,
    const uint8_t* bitmap_data);

/** Draw dot at x,y
 *
 * @param      canvas  Canvas instance
 * @param      x       x coordinate
 * @param      y       y coordinate
 */
void canvas_draw_dot(Canvas* canvas, int32_t x, int32_t y);

/** Draw box of width, height at x,y
 *
 * @param      canvas  Canvas instance
 * @param      x       x coordinate
 * @param      y       y coordinate
 * @param      width   box width
 * @param      height  box height
 */
void canvas_draw_box(Canvas* canvas, int32_t x, int32_t y, size_t width, size_t height);

/** Draw frame of width, height at x,y
 *
 * @param      canvas  Canvas instance
 * @param      x       x coordinate
 * @param      y       y coordinate
 * @param      width   frame width
 * @param      height  frame height
 */
void canvas_draw_frame(Canvas* canvas, int32_t x, int32_t y, size_t width, size_t height);

/** Draw line from x1,y1 to x2,y2
 *
 * @param      canvas  Canvas instance
 * @param      x1      x1 coordinate
 * @param      y1      y1 coordinate
 * @param      x2      x2 coordinate
 * @param      y2      y2 coordinate
 */
void canvas_draw_line(Canvas* canvas, int32_t x1, int32_t y1, int32_t x2, int32_t y2);

/** Draw circle at x,y with radius r
 *
 * @param      canvas  Canvas instance
 * @param      x       x coordinate
 * @param      y       y coordinate
 * @param      radius  radius
 */
void canvas_draw_circle(Canvas* canvas, int32_t x, int32_t y, size_t radius);

/** Draw disc at x,y with radius r
 *
 * @param      canvas  Canvas instance
 * @param      x       x coordinate
 * @param      y       y coordinate
 * @param      radius  radius
 */
void canvas_draw_disc(Canvas* canvas, int32_t x, int32_t y, size_t radius);

/** Draw triangle with given base and height lengths and their intersection
 * coordinate
 *
 * @param      canvas  Canvas instance
 * @param      x       x coordinate of base and height intersection
 * @param      y       y coordinate of base and height intersection
 * @param      base    length of triangle side
 * @param      height  length of triangle height
 * @param      dir     CanvasDirection triangle orientation
 */
void canvas_draw_triangle(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    size_t base,
    size_t height,
    CanvasDirection dir);

/** Draw glyph
 *
 * @param      canvas  Canvas instance
 * @param      x       x coordinate
 * @param      y       y coordinate
 * @param      ch      character
 */
void canvas_draw_glyph(Canvas* canvas, int32_t x, int32_t y, uint16_t ch);

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
    int32_t x,
    int32_t y,
    size_t width,
    size_t height,
    size_t radius);

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
    int32_t x,
    int32_t y,
    size_t width,
    size_t height,
    size_t radius);

#ifdef __cplusplus
}
#endif
