#include "view_port_i.h"

#include <furi.h>

#include "gui.h"
#include "gui_i.h"

// TODO add mutex to view_port ops

static void view_port_rotate_buttons(InputEvent* event) {
    switch(event->key) {
    case InputKeyUp:
        event->key = InputKeyRight;
        break;
    case InputKeyDown:
        event->key = InputKeyLeft;
        break;
    case InputKeyRight:
        event->key = InputKeyDown;
        break;
    case InputKeyLeft:
        event->key = InputKeyUp;
        break;
    default:
        break;
    }
}

static void view_port_setup_canvas_orientation(const ViewPort* view_port, Canvas* canvas) {
    if(view_port->orientation == ViewPortOrientationHorizontal) {
        canvas_set_orientation(canvas, CanvasOrientationHorizontal);
    } else if(view_port->orientation == ViewPortOrientationVertical) {
        canvas_set_orientation(canvas, CanvasOrientationVertical);
    }
}

ViewPort* view_port_alloc() {
    ViewPort* view_port = malloc(sizeof(ViewPort));
    view_port->orientation = ViewPortOrientationHorizontal;
    view_port->is_enabled = true;
    return view_port;
}

void view_port_free(ViewPort* view_port) {
    furi_assert(view_port);
    furi_check(view_port->gui == NULL);
    free(view_port);
}

void view_port_set_width(ViewPort* view_port, uint8_t width) {
    furi_assert(view_port);
    view_port->width = width;
}

uint8_t view_port_get_width(ViewPort* view_port) {
    furi_assert(view_port);
    return view_port->width;
}

void view_port_set_height(ViewPort* view_port, uint8_t height) {
    furi_assert(view_port);
    view_port->height = height;
}

uint8_t view_port_get_height(ViewPort* view_port) {
    furi_assert(view_port);
    return view_port->height;
}

void view_port_enabled_set(ViewPort* view_port, bool enabled) {
    furi_assert(view_port);
    if(view_port->is_enabled != enabled) {
        view_port->is_enabled = enabled;
        if(view_port->gui) gui_update(view_port->gui);
    }
}

bool view_port_is_enabled(ViewPort* view_port) {
    furi_assert(view_port);
    return view_port->is_enabled;
}

void view_port_draw_callback_set(ViewPort* view_port, ViewPortDrawCallback callback, void* context) {
    furi_assert(view_port);
    view_port->draw_callback = callback;
    view_port->draw_callback_context = context;
}

void view_port_input_callback_set(
    ViewPort* view_port,
    ViewPortInputCallback callback,
    void* context) {
    furi_assert(view_port);
    view_port->input_callback = callback;
    view_port->input_callback_context = context;
}

void view_port_update(ViewPort* view_port) {
    furi_assert(view_port);
    if(view_port->gui && view_port->is_enabled) gui_update(view_port->gui);
}

void view_port_gui_set(ViewPort* view_port, Gui* gui) {
    furi_assert(view_port);
    view_port->gui = gui;
}

void view_port_draw(ViewPort* view_port, Canvas* canvas) {
    furi_assert(view_port);
    furi_assert(canvas);
    furi_check(view_port->gui);

    if(view_port->draw_callback) {
        view_port_setup_canvas_orientation(view_port, canvas);
        view_port->draw_callback(canvas, view_port->draw_callback_context);
    }
}

void view_port_input(ViewPort* view_port, InputEvent* event) {
    furi_assert(view_port);
    furi_assert(event);
    furi_check(view_port->gui);

    if(view_port->input_callback) {
        if(view_port_get_orientation(view_port) == ViewPortOrientationVertical) {
            view_port_rotate_buttons(event);
        }
        view_port->input_callback(event, view_port->input_callback_context);
    }
}

void view_port_set_orientation(ViewPort* view_port, ViewPortOrientation orientation) {
    furi_assert(view_port);
    view_port->orientation = orientation;
}

ViewPortOrientation view_port_get_orientation(const ViewPort* view_port) {
    return view_port->orientation;
}
