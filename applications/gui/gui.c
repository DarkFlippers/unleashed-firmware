#include "gui.h"
#include "gui_i.h"

#include <furi.h>
#include <m-array.h>
#include <stdio.h>

#include "gui_event.h"
#include "canvas.h"
#include "canvas_i.h"
#include "view_port.h"
#include "view_port_i.h"

ARRAY_DEF(ViewPortArray, ViewPort*, M_PTR_OPLIST);

struct Gui {
    GuiEvent* event;
    Canvas* canvas;
    ViewPortArray_t layers[GuiLayerMAX];
    osMutexId_t mutex;
};

ViewPort* gui_view_port_find_enabled(ViewPortArray_t array) {
    size_t view_ports_count = ViewPortArray_size(array);
    for(size_t i = 0; i < view_ports_count; i++) {
        ViewPort* view_port = *ViewPortArray_get(array, view_ports_count - i - 1);
        if(view_port_is_enabled(view_port)) {
            return view_port;
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
    ViewPort* view_port = gui_view_port_find_enabled(gui->layers[GuiLayerFullscreen]);
    if(view_port) {
        view_port_draw(view_port, gui->canvas);
        return true;
    } else {
        return false;
    }
}

void gui_redraw_status_bar(Gui* gui) {
    ViewPortArray_it_t it;
    uint8_t x;
    uint8_t x_used = 0;
    uint8_t width;
    ViewPort* view_port;
    // Right side
    x = 128;
    ViewPortArray_it(it, gui->layers[GuiLayerStatusBarRight]);
    while(!ViewPortArray_end_p(it) && x_used < GUI_STATUS_BAR_WIDTH) {
        // Render view_port;
        view_port = *ViewPortArray_ref(it);
        if(view_port_is_enabled(view_port)) {
            width = view_port_get_width(view_port);
            if(!width) width = 8;
            x_used += width;
            x -= (width + 2);
            canvas_frame_set(gui->canvas, x, GUI_STATUS_BAR_Y, width, GUI_STATUS_BAR_HEIGHT);
            view_port_draw(view_port, gui->canvas);
        }
        ViewPortArray_next(it);
    }
    // Left side
    x = 0;
    ViewPortArray_it(it, gui->layers[GuiLayerStatusBarLeft]);
    while(!ViewPortArray_end_p(it) && x_used < GUI_STATUS_BAR_WIDTH) {
        // Render view_port;
        view_port = *ViewPortArray_ref(it);
        if(view_port_is_enabled(view_port)) {
            width = view_port_get_width(view_port);
            if(!width) width = 8;
            x_used += width;
            canvas_frame_set(gui->canvas, x, GUI_STATUS_BAR_Y, width, GUI_STATUS_BAR_HEIGHT);
            view_port_draw(view_port, gui->canvas);
            x += (width + 2);
        }
        ViewPortArray_next(it);
    }
}

bool gui_redraw_normal(Gui* gui) {
    canvas_frame_set(gui->canvas, GUI_MAIN_X, GUI_MAIN_Y, GUI_MAIN_WIDTH, GUI_MAIN_HEIGHT);
    ViewPort* view_port = gui_view_port_find_enabled(gui->layers[GuiLayerMain]);
    if(view_port) {
        view_port_draw(view_port, gui->canvas);
        return true;
    }
    return false;
}

bool gui_redraw_none(Gui* gui) {
    canvas_frame_set(gui->canvas, GUI_MAIN_X, GUI_MAIN_Y, GUI_MAIN_WIDTH, GUI_MAIN_HEIGHT);
    ViewPort* view_port = gui_view_port_find_enabled(gui->layers[GuiLayerNone]);
    if(view_port) {
        view_port_draw(view_port, gui->canvas);
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

    ViewPort* view_port = gui_view_port_find_enabled(gui->layers[GuiLayerFullscreen]);
    if(!view_port) view_port = gui_view_port_find_enabled(gui->layers[GuiLayerMain]);
    if(!view_port) view_port = gui_view_port_find_enabled(gui->layers[GuiLayerNone]);

    if(view_port) {
        view_port_input(view_port, input_event);
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

void gui_add_view_port(Gui* gui, ViewPort* view_port, GuiLayer layer) {
    furi_assert(gui);
    furi_assert(view_port);
    furi_check(layer < GuiLayerMAX);

    gui_lock(gui);
    ViewPortArray_push_back(gui->layers[layer], view_port);
    view_port_gui_set(view_port, gui);
    gui_unlock(gui);
    gui_update(gui);
}

void gui_remove_view_port(Gui* gui, ViewPort* view_port) {
    furi_assert(gui);
    furi_assert(view_port);

    gui_lock(gui);

    view_port_gui_set(view_port, NULL);
    ViewPortArray_it_t it;
    for(size_t i = 0; i < GuiLayerMAX; i++) {
        ViewPortArray_it(it, gui->layers[i]);
        while(!ViewPortArray_end_p(it)) {
            if(*ViewPortArray_ref(it) == view_port) {
                ViewPortArray_remove(gui->layers[i], it);
            }
            ViewPortArray_next(it);
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
        ViewPortArray_init(gui->layers[i]);
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
