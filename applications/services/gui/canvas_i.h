/**
 * @file canvas_i.h
 * GUI: internal Canvas API
 */

#pragma once

#include "canvas.h"
#include <u8g2.h>

/** Canvas structure
 */
struct Canvas {
    u8g2_t fb;
    CanvasOrientation orientation;
    uint8_t offset_x;
    uint8_t offset_y;
    uint8_t width;
    uint8_t height;
};

/** Allocate memory and initialize canvas
 *
 * @return     Canvas instance
 */
Canvas* canvas_init();

/** Free canvas memory
 *
 * @param      canvas  Canvas instance
 */
void canvas_free(Canvas* canvas);

/** Get canvas buffer.
 *
 * @param      canvas  Canvas instance
 *
 * @return     pointer to buffer
 */
uint8_t* canvas_get_buffer(Canvas* canvas);

/** Get canvas buffer size.
 *
 * @param      canvas  Canvas instance
 *
 * @return     size of canvas in bytes
 */
size_t canvas_get_buffer_size(const Canvas* canvas);

/** Set drawing region relative to real screen buffer
 *
 * @param      canvas    Canvas instance
 * @param      offset_x  x coordinate offset
 * @param      offset_y  y coordinate offset
 * @param      width     width
 * @param      height    height
 */
void canvas_frame_set(
    Canvas* canvas,
    uint8_t offset_x,
    uint8_t offset_y,
    uint8_t width,
    uint8_t height);

/** Set canvas orientation
 *
 * @param      canvas       Canvas instance
 * @param      orientation  CanvasOrientation
 */
void canvas_set_orientation(Canvas* canvas, CanvasOrientation orientation);

/** Get canvas orientation
 *
 * @param      canvas  Canvas instance
 *
 * @return     CanvasOrientation
 */
CanvasOrientation canvas_get_orientation(const Canvas* canvas);
