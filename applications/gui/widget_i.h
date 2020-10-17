#pragma once

#include "gui_i.h"

void widget_gui_set(Widget* widget, Gui* gui);

void widget_draw(Widget* widget, CanvasApi* canvas_api);

void widget_input(Widget* widget, InputEvent* event);
