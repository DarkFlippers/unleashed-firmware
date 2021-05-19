#pragma once

#include "gui_i.h"
#include "view_port.h"

struct ViewPort {
    Gui* gui;
    bool is_enabled;
    ViewPortOrientation orientation;

    uint8_t width;
    uint8_t height;

    ViewPortDrawCallback draw_callback;
    void* draw_callback_context;

    ViewPortInputCallback input_callback;
    void* input_callback_context;
};

/*
 * Set GUI reference.
 * To be used by GUI, called upon view_port tree insert
 * @param gui - gui instance pointer.
 */
void view_port_gui_set(ViewPort* view_port, Gui* gui);

/*
 * Process draw call. Calls draw callback.
 * To be used by GUI, called on tree redraw.
 * @param canvas - canvas to draw at.
 */
void view_port_draw(ViewPort* view_port, Canvas* canvas);

/*
 * Process input. Calls input callbac
 * To be used by GUI, called on input dispatch.
 * @param event - pointer to input event.
 */
void view_port_input(ViewPort* view_port, InputEvent* event);
