#pragma once

#include <stdint.h>
#include "canvas.h"

#ifdef __cplusplus
extern "C" {
#endif

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
 * Draw multiline text
 * @param x, y - top left corner coordinates
 * @param text - string (possible multiline)
 */
void elements_multiline_text(Canvas* canvas, uint8_t x, uint8_t y, char* text);

#ifdef __cplusplus
}
#endif
