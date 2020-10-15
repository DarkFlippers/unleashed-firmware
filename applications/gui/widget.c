#include "widget.h"
#include "widget_i.h"

#include <cmsis_os.h>
#include <flipper.h>

#include "gui.h"
#include "gui_i.h"

Widget* widget_alloc(WidgetDrawCallback callback, void* callback_context) {
    Widget* widget = furi_alloc(sizeof(Widget));
    widget->is_enabled = true;
    return widget;
}

void widget_free(Widget* widget) {
    assert(widget);
    assert(widget->gui == NULL);
    free(widget);
}

void widget_enabled_set(Widget* widget, bool enabled) {
    assert(widget);
    widget->is_enabled = enabled;
    widget_update(widget);
}

bool widget_is_enabled(Widget* widget) {
    assert(widget);
    return widget->is_enabled;
}

void widget_draw_callback_set(Widget* widget, WidgetDrawCallback callback, void* context) {
    assert(widget);
    widget->draw_callback = callback;
    widget->draw_callback_context = context;
}

void widget_input_callback_set(Widget* widget, WidgetInputCallback callback, void* context) {
    assert(widget);
    widget->input_callback = callback;
    widget->input_callback_context = context;
}

void widget_update(Widget* widget) {
    assert(widget);
    if(widget->gui) gui_update(widget->gui);
}

void widget_gui_set(Widget* widget, GUI* gui) {
    assert(widget);
    assert(gui);
    widget->gui = gui;
}

void widget_draw(Widget* widget, Canvas* canvas) {
    assert(widget);
    assert(canvas);
    assert(widget->gui);

    if(widget->draw_callback) widget->draw_callback(canvas, widget->draw_callback_context);
}

void widget_input(Widget* widget, InputEvent* event) {
    assert(widget);
    assert(event);
    assert(widget->gui);

    if(widget->input_callback) widget->input_callback(event, widget->input_callback_context);
}
