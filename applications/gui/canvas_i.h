#pragma once

#include "canvas.h"
#include <u8g2.h>

struct Canvas {
    u8g2_t fb;
    CanvasOrientation orientation;
    uint8_t offset_x;
    uint8_t offset_y;
    uint8_t width;
    uint8_t height;
};

/*
 * Allocate memory and initialize canvas
 */
Canvas* canvas_init();

/*
 * Free canvas memory
 */
void canvas_free(Canvas* canvas);

/*
 * Reset canvas drawing tools configuration
 */
void canvas_reset(Canvas* canvas);

/*
 * Commit canvas. Send buffer to display
 */
void canvas_commit(Canvas* canvas);

/*
 * Get canvas buffer.
 * @return pointer to buffer
 */
uint8_t* canvas_get_buffer(Canvas* canvas);

/*
 * Get canvas buffer size.
 * @return size of canvas in bytes
 */
size_t canvas_get_buffer_size(Canvas* canvas);

/*
 * Set drawing region relative to real screen buffer
 */
void canvas_frame_set(
    Canvas* canvas,
    uint8_t offset_x,
    uint8_t offset_y,
    uint8_t width,
    uint8_t height);

/*
 * Set canvas orientation
 */
void canvas_set_orientation(Canvas* canvas, CanvasOrientation orientation);

/*
 * Get canvas orientation
 */
CanvasOrientation canvas_get_orientation(const Canvas* canvas);
