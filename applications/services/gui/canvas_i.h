/**
 * @file canvas_i.h
 * GUI: internal Canvas API
 */

#pragma once

#include "canvas.h"
#include <u8g2.h>
#include <toolbox/compress.h>
#include <m-array.h>
#include <m-algo.h>
#include <furi.h>

#define ICON_DECOMPRESSOR_BUFFER_SIZE (128u * 64 / 8)

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*CanvasCommitCallback)(
    uint8_t* data,
    size_t size,
    CanvasOrientation orientation,
    void* context);

typedef struct {
    CanvasCommitCallback callback;
    void* context;
} CanvasCallbackPair;

ARRAY_DEF(CanvasCallbackPairArray, CanvasCallbackPair, M_POD_OPLIST);

#define M_OPL_CanvasCallbackPairArray_t() ARRAY_OPLIST(CanvasCallbackPairArray, M_POD_OPLIST)

ALGO_DEF(CanvasCallbackPairArray, CanvasCallbackPairArray_t);

/** Canvas structure
 */
struct Canvas {
    u8g2_t fb;
    CanvasOrientation orientation;
    size_t offset_x;
    size_t offset_y;
    size_t width;
    size_t height;
    CompressIcon* compress_icon;
    CanvasCallbackPairArray_t canvas_callback_pair;
    FuriMutex* mutex;
};

/** Allocate memory and initialize canvas
 *
 * @return     Canvas instance
 */
Canvas* canvas_init(void);

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
    int32_t offset_x,
    int32_t offset_y,
    size_t width,
    size_t height);

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

/** Draw a u8g2 bitmap
 *
 * @param      u8g2     u8g2 instance
 * @param      x        x coordinate
 * @param      y        y coordinate
 * @param      width    width
 * @param      height   height
 * @param      bitmap   bitmap
 * @param      rotation rotation
 */
void canvas_draw_u8g2_bitmap(
    u8g2_t* u8g2,
    int32_t x,
    int32_t y,
    size_t width,
    size_t height,
    const uint8_t* bitmap,
    IconRotation rotation);

/** Add canvas commit callback.
 *
 * This callback will be called upon Canvas commit.
 * 
 * @param      canvas    Canvas instance
 * @param      callback  CanvasCommitCallback
 * @param      context   CanvasCommitCallback context
 */
void canvas_add_framebuffer_callback(Canvas* canvas, CanvasCommitCallback callback, void* context);

/** Remove canvas commit callback.
 *
 * @param      canvas    Canvas instance
 * @param      callback  CanvasCommitCallback
 * @param      context   CanvasCommitCallback context
 */
void canvas_remove_framebuffer_callback(
    Canvas* canvas,
    CanvasCommitCallback callback,
    void* context);

#ifdef __cplusplus
}
#endif
