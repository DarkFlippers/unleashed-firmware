#include "gui.h"
#include "gui_i.h"

#include <flipper.h>
#include <flipper_v2.h>
#include <m-array.h>
#include <stdio.h>

#include "gui_event.h"
#include "canvas.h"
#include "canvas_i.h"
#include "widget.h"
#include "widget_i.h"

ARRAY_DEF(WidgetArray, Widget*, M_PTR_OPLIST);

struct Gui {
    GuiApi api;
    GuiEvent* event;
    CanvasApi* canvas_api;
    WidgetArray_t layers[GuiLayerMAX];
    osMutexId_t mutex;
};

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

void gui_update(Gui* gui) {
    furi_assert(gui);
    GuiMessage message;
    message.type = GuiMessageTypeRedraw;
    gui_event_messsage_send(gui->event, &message);
}

bool gui_redraw_fs(Gui* gui) {
    canvas_frame_set(gui->canvas_api, 0, 0, 128, 64);
    Widget* widget = gui_widget_find_enabled(gui->layers[GuiLayerFullscreen]);
    if(widget) {
        widget_draw(widget, gui->canvas_api);
        return true;
    } else {
        return false;
    }
}

bool gui_redraw_status_bar(Gui* gui) {
    canvas_frame_set(gui->canvas_api, 0, 0, 128, 64);
    Widget* widget = gui_widget_find_enabled(gui->layers[GuiLayerStatusBar]);
    if(widget) {
        widget_draw(widget, gui->canvas_api);
        return true;
    }
    return false;
}

bool gui_redraw_normal(Gui* gui) {
    canvas_frame_set(gui->canvas_api, 0, 9, 128, 55);
    Widget* widget = gui_widget_find_enabled(gui->layers[GuiLayerMain]);
    if(widget) {
        widget_draw(widget, gui->canvas_api);
        return true;
    }
    return false;
}

bool gui_redraw_none(Gui* gui) {
    canvas_frame_set(gui->canvas_api, 0, 9, 118, 44);
    Widget* widget = gui_widget_find_enabled(gui->layers[GuiLayerNone]);
    if(widget) {
        widget_draw(widget, gui->canvas_api);
        return true;
    }

    return false;
}

void gui_redraw(Gui* gui) {
    furi_assert(gui);
    gui_lock(gui);

    if(!gui_redraw_fs(gui)) {
        if(!gui_redraw_normal(gui)) {
            gui_redraw_none(gui);
        }
        gui_redraw_status_bar(gui);
    }

    canvas_commit(gui->canvas_api);
    gui_unlock(gui);
}

void gui_input(Gui* gui, InputEvent* input_event) {
    furi_assert(gui);
    furi_assert(input_event);

    gui_lock(gui);

    Widget* widget = gui_widget_find_enabled(gui->layers[GuiLayerFullscreen]);
    if(!widget) widget = gui_widget_find_enabled(gui->layers[GuiLayerMain]);
    if(!widget) widget = gui_widget_find_enabled(gui->layers[GuiLayerNone]);

    if(widget) {
        widget_input(widget, input_event);
    }

    gui_unlock(gui);
}

void gui_lock(Gui* gui) {
    furi_assert(gui);
    furi_check(osMutexAcquire(gui->mutex, osWaitForever) == osOK);
}

void gui_unlock(Gui* gui) {
    furi_assert(gui);
    furi_check(osMutexRelease(gui->mutex) == osOK);
}

void gui_add_widget(GuiApi* gui_api, Widget* widget, GuiLayer layer) {
    furi_assert(gui_api);
    furi_assert(widget);
    furi_check(layer < GuiLayerMAX);
    Gui* gui = (Gui*)gui_api;

    gui_lock(gui);
    WidgetArray_push_back(gui->layers[layer], widget);
    widget_gui_set(widget, gui);
    gui_unlock(gui);
    gui_update(gui);
}

void gui_remove_widget(GuiApi* gui_api, Widget* widget) {
    furi_assert(gui_api);
    furi_assert(widget);
    Gui* gui = (Gui*)gui_api;

    gui_lock(gui);

    widget_gui_set(widget, NULL);
    WidgetArray_it_t it;
    for(size_t i = 0; i < GuiLayerMAX; i++) {
        WidgetArray_it(it, gui->layers[i]);
        while(!WidgetArray_end_p(it)) {
            if(*WidgetArray_ref(it) == widget) {
                WidgetArray_remove(gui->layers[i], it);
            }
            WidgetArray_next(it);
        }
    }

    gui_unlock(gui);
}

Gui* gui_alloc() {
    Gui* gui = furi_alloc(sizeof(Gui));
    // Set API functions
    gui->api.add_widget = gui_add_widget;
    gui->api.remove_widget = gui_remove_widget;
    // Allocate mutex
    gui->mutex = osMutexNew(NULL);
    furi_check(gui->mutex);
    // Event dispatcher
    gui->event = gui_event_alloc();
    // Drawing canvas api
    gui->canvas_api = canvas_api_init();
    // Compose Layers
    for(size_t i = 0; i < GuiLayerMAX; i++) {
        WidgetArray_init(gui->layers[i]);
    }

    return gui;
}

void gui_task(void* p) {
    Gui* gui = gui_alloc();
    // Create FURI record
    if(!furi_create("gui", gui)) {
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
