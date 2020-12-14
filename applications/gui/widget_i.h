#pragma once

#include "gui_i.h"
#include "widget.h"

struct Widget {
    Gui* gui;
    bool is_enabled;

    uint8_t width;
    uint8_t height;

    WidgetDrawCallback draw_callback;
    void* draw_callback_context;

    WidgetInputCallback input_callback;
    void* input_callback_context;
};

/*
 * Set GUI reference.
 * To be used by GUI, called upon widget tree insert
 * @param gui - gui instance pointer.
 */
void widget_gui_set(Widget* widget, Gui* gui);

/*
 * Process draw call. Calls draw callback.
 * To be used by GUI, called on tree redraw.
 * @param canvas - canvas to draw at.
 */
void widget_draw(Widget* widget, Canvas* canvas);

/*
 * Process input. Calls input callbac
 * To be used by GUI, called on input dispatch.
 * @param event - pointer to input event.
 */
void widget_input(Widget* widget, InputEvent* event);
