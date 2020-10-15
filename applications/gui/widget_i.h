#pragma once

void widget_gui_set(Widget* widget, Gui* gui);

void widget_draw(Widget* widget, Canvas* canvas);

void widget_input(Widget* widget, InputEvent* event);
