#pragma once

#include <input/input.h>
#include "canvas.h"

typedef struct Widget Widget;

typedef void (*WidgetDrawCallback)(CanvasApi* api, void* context);
typedef void (*WidgetInputCallback)(InputEvent* event, void* context);

Widget* widget_alloc();
void widget_free(Widget* widget);

void widget_enabled_set(Widget* widget, bool enabled);
bool widget_is_enabled(Widget* widget);

void widget_draw_callback_set(Widget* widget, WidgetDrawCallback callback, void* context);
void widget_input_callback_set(Widget* widget, WidgetInputCallback callback, void* context);

// emit update signal
void widget_update(Widget* widget);
