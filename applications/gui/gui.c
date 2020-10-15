#include "gui.h"
#include "gui_i.h"

#include <flipper.h>
#include <flipper_v2.h>
#include <stdio.h>
#include <m-array.h>

#include "gui_event.h"
#include "canvas.h"
#include "canvas_i.h"
#include "widget.h"
#include "widget_i.h"

ARRAY_DEF(WidgetArray, Widget*, M_PTR_OPLIST);

struct Gui {
    GuiEvent* event;
    CanvasApi* canvas_api;
    WidgetArray_t widgets_status_bar;
    WidgetArray_t widgets;
    WidgetArray_t widgets_fs;
    WidgetArray_t widgets_dialog;
};

void gui_add_widget(GuiApi* gui_api, Widget* widget, WidgetLayer layer) {
    assert(gui_api);
    assert(gui_api->gui);

    // TODO add mutex on widget array

    WidgetArray_t* widget_array = NULL;

    switch(layer) {
        case WidgetLayerStatusBar:
            widget_array = &gui_api->gui->widgets_status_bar;
        break;
        case WidgetLayerMain:
            widget_array = &gui_api->gui->widgets;
        break;
        case WidgetLayerFullscreen:
            widget_array = &gui_api->gui->widgets_fs;
        break;
        case WidgetLayerDialog:
            widget_array = &gui_api->gui->widgets_dialog;
        break;

        default: break;
    }

    assert(widget);
    assert(widget_array);

    gui_event_lock(gui_api->gui->event);
    WidgetArray_push_back((struct WidgetArray_s*)widget_array, widget);
    widget_gui_set(widget, gui_api->gui);
    gui_event_unlock(gui_api->gui->event);

    gui_update(gui_api->gui);
}

void gui_update(Gui* gui) {
    assert(gui);
    GuiMessage message;
    message.type = GuiMessageTypeRedraw;
    gui_event_messsage_send(gui->event, &message);
}

Widget* gui_widget_find_enabled(WidgetArray_t array) {
    size_t widgets_count = WidgetArray_size(array);
    for(size_t i = 0; i < widgets_count; i++) {
        Widget* widget = *WidgetArray_get(array, widgets_count - i - 1);
        if(widget_is_enabled(widget)) {
            return widget;
        }
    }
    return NULL;
}

bool gui_redraw_fs(Gui* gui) {
    canvas_frame_set(gui->canvas_api, 0, 0, 128, 64);
    Widget* widget = gui_widget_find_enabled(gui->widgets_fs);
    if(widget) {
        widget_draw(widget, gui->canvas_api);
        return true;
    } else {
        return false;
    }
}

void gui_redraw_status_bar(Gui* gui) {
    canvas_frame_set(gui->canvas_api, 0, 0, 128, 64);
    Widget* widget = gui_widget_find_enabled(gui->widgets_status_bar);
    if(widget) widget_draw(widget, gui->canvas_api);
}

void gui_redraw_normal(Gui* gui) {
    canvas_frame_set(gui->canvas_api, 0, 9, 128, 55);
    Widget* widget = gui_widget_find_enabled(gui->widgets);
    if(widget) widget_draw(widget, gui->canvas_api);
}

void gui_redraw_dialogs(Gui* gui) {
    canvas_frame_set(gui->canvas_api, 10, 20, 118, 44);
    Widget* widget = gui_widget_find_enabled(gui->widgets_dialog);
    if(widget) widget_draw(widget, gui->canvas_api);
}

void gui_redraw(Gui* gui) {
    assert(gui);

    if(!gui_redraw_fs(gui)) {
        gui_redraw_status_bar(gui);
        gui_redraw_normal(gui);
    }
    gui_redraw_dialogs(gui);

    canvas_commit(gui->canvas_api);
}

void gui_input(Gui* gui, InputEvent* input_event) {
    assert(gui);

    Widget* widget = gui_widget_find_enabled(gui->widgets_dialog);
    if(!widget) widget = gui_widget_find_enabled(gui->widgets_fs);
    if(!widget) widget = gui_widget_find_enabled(gui->widgets);

    if(widget) {
        widget_input(widget, input_event);
    }
}

Gui* gui_alloc() {
    Gui* gui = furi_alloc(sizeof(Gui));
    // Initialize widget arrays
    WidgetArray_init(gui->widgets_status_bar);
    WidgetArray_init(gui->widgets);
    WidgetArray_init(gui->widgets_fs);
    WidgetArray_init(gui->widgets_dialog);

    // Event dispatcher
    gui->event = gui_event_alloc();

    // Drawing canvas api

    gui->canvas_api = canvas_api_init();

    return gui;
}

void gui_task(void* p) {
    Gui* gui = gui_alloc();

    GuiApi gui_api = {
        .add_widget = gui_add_widget,
        .gui = gui,
    };

    // Create FURI record
    if(!furi_create("gui", &gui_api)) {
        printf("[gui_task] cannot create the gui record\n");
        furiac_exit(NULL);
    }

    furiac_ready();

    // Forever dispatch
    while(1) {
        GuiMessage message = gui_event_message_next(gui->event);
        if(message.type == GuiMessageTypeRedraw) {
            gui_redraw(gui);
        } else if(message.type == GuiMessageTypeInput) {
            gui_input(gui, &message.input);
        }
    }
}
