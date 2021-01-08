#pragma once

#include <input/input.h>
#include "canvas.h"

typedef struct Widget Widget;

/*
 * Widget Draw callback
 * @warning called from GUI thread
 */
typedef void (*WidgetDrawCallback)(Canvas* canvas, void* context);

/*
 * Widget Input callback
 * @warning called from GUI thread
 */
typedef void (*WidgetInputCallback)(InputEvent* event, void* context);

/*
 * Widget allocator
 * always returns widget or stops system if not enough memory.
 */
Widget* widget_alloc();

/*
 * Widget deallocator
 * Ensure that widget was unregistered in GUI system before use.
 */
void widget_free(Widget* widget);

/*
 * Set widget width.
 * Will be used to limit canvas drawing area and autolayout feature.
 * @param width - wanted width, 0 - auto.
 */
void widget_set_width(Widget* widget, uint8_t width);
uint8_t widget_get_width(Widget* widget);

/*
 * Set widget height.
 * Will be used to limit canvas drawing area and autolayout feature.
 * @param height - wanted height, 0 - auto.
 */
void widget_set_height(Widget* widget, uint8_t height);
uint8_t widget_get_height(Widget* widget);

/*
 * Enable or disable widget rendering.
 * @param enabled.
 */
void widget_enabled_set(Widget* widget, bool enabled);
bool widget_is_enabled(Widget* widget);

/*
 * Widget event callbacks
 * @param callback - appropriate callback function
 * @param context - context to pass to callback
 */
void widget_draw_callback_set(Widget* widget, WidgetDrawCallback callback, void* context);
void widget_input_callback_set(Widget* widget, WidgetInputCallback callback, void* context);

/*
 * Emit update signal to GUI system.
 * Rendering will happen later after GUI system process signal.
 */
void widget_update(Widget* widget);
