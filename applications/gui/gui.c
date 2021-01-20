#include "gui.h"
#include "gui_i.h"

#include <furi.h>
#include <m-array.h>
#include <stdio.h>

#include "gui_event.h"
#include "canvas.h"
#include "canvas_i.h"
#include "widget.h"
#include "widget_i.h"

ARRAY_DEF(WidgetArray, Widget*, M_PTR_OPLIST);

struct Gui {
    GuiEvent* event;
    Canvas* canvas;
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
    canvas_frame_set(gui->canvas, 0, 0, GUI_DISPLAY_WIDTH, GUI_DISPLAY_HEIGHT);
    Widget* widget = gui_widget_find_enabled(gui->layers[GuiLayerFullscreen]);
    if(widget) {
        widget_draw(widget, gui->canvas);
        return true;
    } else {
        return false;
    }
}

void gui_redraw_status_bar(Gui* gui) {
    WidgetArray_it_t it;
    uint8_t x;
    uint8_t x_used = 0;
    uint8_t width;
    Widget* widget;
    // Right side
    x = 128;
    WidgetArray_it(it, gui->layers[GuiLayerStatusBarRight]);
    while(!WidgetArray_end_p(it) && x_used < GUI_STATUS_BAR_WIDTH) {
        // Render widget;
        widget = *WidgetArray_ref(it);
        if(widget_is_enabled(widget)) {
            width = widget_get_width(widget);
            if(!width) width = 8;
            x_used += width;
            x -= (width + 2);
            canvas_frame_set(gui->canvas, x, GUI_STATUS_BAR_Y, width, GUI_STATUS_BAR_HEIGHT);
            widget_draw(widget, gui->canvas);
        }
        WidgetArray_next(it);
    }
    // Left side
    x = 0;
    WidgetArray_it(it, gui->layers[GuiLayerStatusBarLeft]);
    while(!WidgetArray_end_p(it) && x_used < GUI_STATUS_BAR_WIDTH) {
        // Render widget;
        widget = *WidgetArray_ref(it);
        if(widget_is_enabled(widget)) {
            width = widget_get_width(widget);
            if(!width) width = 8;
            x_used += width;
            canvas_frame_set(gui->canvas, x, GUI_STATUS_BAR_Y, width, GUI_STATUS_BAR_HEIGHT);
            widget_draw(widget, gui->canvas);
            x += (width + 2);
        }
        WidgetArray_next(it);
    }
}

bool gui_redraw_normal(Gui* gui) {
    canvas_frame_set(gui->canvas, GUI_MAIN_X, GUI_MAIN_Y, GUI_MAIN_WIDTH, GUI_MAIN_HEIGHT);
    Widget* widget = gui_widget_find_enabled(gui->layers[GuiLayerMain]);
    if(widget) {
        widget_draw(widget, gui->canvas);
        return true;
    }
    return false;
}

bool gui_redraw_none(Gui* gui) {
    canvas_frame_set(gui->canvas, GUI_MAIN_X, GUI_MAIN_Y, GUI_MAIN_WIDTH, GUI_MAIN_HEIGHT);
    Widget* widget = gui_widget_find_enabled(gui->layers[GuiLayerNone]);
    if(widget) {
        widget_draw(widget, gui->canvas);
        return true;
    }

    return false;
}

void gui_redraw(Gui* gui) {
    furi_assert(gui);
    gui_lock(gui);

    canvas_reset(gui->canvas);

    if(!gui_redraw_fs(gui)) {
        if(!gui_redraw_normal(gui)) {
            gui_redraw_none(gui);
        }
        gui_redraw_status_bar(gui);
    }

    canvas_commit(gui->canvas);
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

void gui_add_widget(Gui* gui, Widget* widget, GuiLayer layer) {
    furi_assert(gui);
    furi_assert(widget);
    furi_check(layer < GuiLayerMAX);

    gui_lock(gui);
    WidgetArray_push_back(gui->layers[layer], widget);
    widget_gui_set(widget, gui);
    gui_unlock(gui);
    gui_update(gui);
}

void gui_remove_widget(Gui* gui, Widget* widget) {
    furi_assert(gui);
    furi_assert(widget);

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
    // Allocate mutex
    gui->mutex = osMutexNew(NULL);
    furi_check(gui->mutex);
    // Event dispatcher
    gui->event = gui_event_alloc();
    // Drawing canvas
    gui->canvas = canvas_init();
    // Compose Layers
    for(size_t i = 0; i < GuiLayerMAX; i++) {
        WidgetArray_init(gui->layers[i]);
    }

    return gui;
}

void gui_task(void* p) {
    Gui* gui = gui_alloc();

    // Create FURI record
    furi_record_create("gui", gui);

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
