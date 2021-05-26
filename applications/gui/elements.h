#pragma once

#include <stdint.h>
#include <m-string.h>
#include "canvas.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Draw scrollbar on canvas at specific position.
 * @param x - scrollbar position on X axis
 * @param y - scrollbar position on Y axis
 * @param height - scrollbar height
 * @param pos - current element 
 * @param total - total elements
 */

void elements_scrollbar_pos(
    Canvas* canvas,
    uint8_t x,
    uint8_t y,
    uint8_t height,
    uint16_t pos,
    uint16_t total);

/*
 * Draw scrollbar on canvas.
 * width 3px, height equal to canvas height
 * @param pos - current element of total elements
 * @param total - total elements
 */
void elements_scrollbar(Canvas* canvas, uint8_t pos, uint8_t total);

/*
 * Draw rounded frame
 * @param x, y - top left corner coordinates
 * @param width, height - frame width and height
 */
void elements_frame(Canvas* canvas, uint8_t x, uint8_t y, uint8_t width, uint8_t height);

/*
 * Draw button in left corner
 * @param str - button text
 */
void elements_button_left(Canvas* canvas, const char* str);

/*
 * Draw button in right corner
 * @param str - button text
 */
void elements_button_right(Canvas* canvas, const char* str);

/*
 * Draw button in center
 * @param str - button text
 */
void elements_button_center(Canvas* canvas, const char* str);

/*
 * Draw aligned multiline text
 * @param x, y - coordinates based on align param
 * @param horizontal, vertical - aligment of multiline text
 * @param text - string (possible multiline)
 */
void elements_multiline_text_aligned(
    Canvas* canvas,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical,
    const char* text);

/*
 * Draw multiline text
 * @param x, y - top left corner coordinates
 * @param text - string (possible multiline)
 */
void elements_multiline_text(Canvas* canvas, uint8_t x, uint8_t y, const char* text);

/*
 * Draw framed multiline text
 * @param x, y - top left corner coordinates
 * @param text - string (possible multiline)
 */
void elements_multiline_text_framed(Canvas* canvas, uint8_t x, uint8_t y, const char* text);

/*
 * Draw framed multiline text
 * @param x, y - top left corner coordinates
 * @param text - string (possible multiline)
 */

void elements_multiline_text_framed(Canvas* canvas, uint8_t x, uint8_t y, const char* text);

/*
 * Draw slightly rounded frame
 * @param x, y - top left corner coordinates
 * @param width, height - size of frame
 */
void elements_slightly_rounded_frame(
    Canvas* canvas,
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height);

/*
 * Trim string buffer to fit width in pixels
 * @param string - string to trim
 * @param width - max width
 */
void elements_string_fit_width(Canvas* canvas, string_t string, uint8_t width);

#ifdef __cplusplus
}
#endif
