#include "view_dispatcher_i.h"

ViewDispatcher* view_dispatcher_alloc() {
    ViewDispatcher* view_dispatcher = furi_alloc(sizeof(ViewDispatcher));

    view_dispatcher->view_port = view_port_alloc();
    view_port_draw_callback_set(
        view_dispatcher->view_port, view_dispatcher_draw_callback, view_dispatcher);
    view_port_input_callback_set(
        view_dispatcher->view_port, view_dispatcher_input_callback, view_dispatcher);
    view_port_enabled_set(view_dispatcher->view_port, false);

    ViewDict_init(view_dispatcher->views);

    return view_dispatcher;
}

void view_dispatcher_free(ViewDispatcher* view_dispatcher) {
    // Detach from gui
    if(view_dispatcher->gui) {
        gui_remove_view_port(view_dispatcher->gui, view_dispatcher->view_port);
    }
    // Free views
    ViewDict_it_t it;
    ViewDict_it(it, view_dispatcher->views);
    while(!ViewDict_end_p(it)) {
        ViewDict_itref_t* ref = ViewDict_ref(it);
        view_free(ref->value);
        ViewDict_next(it);
    }
    ViewDict_clear(view_dispatcher->views);
    // Free dispatcher
    free(view_dispatcher);
}

void view_dispatcher_add_view(ViewDispatcher* view_dispatcher, uint32_t view_id, View* view) {
    furi_assert(view_dispatcher);
    furi_assert(view);
    // Check if view id is not used and resgister view
    furi_check(ViewDict_get(view_dispatcher->views, view_id) == NULL);
    ViewDict_set_at(view_dispatcher->views, view_id, view);
    view_set_dispatcher(view, view_dispatcher);
}

void view_dispatcher_switch_to_view(ViewDispatcher* view_dispatcher, uint32_t view_id) {
    furi_assert(view_dispatcher);
    if(view_id == VIEW_NONE) {
        view_dispatcher->current_view = NULL;
        view_port_enabled_set(view_dispatcher->view_port, false);
    } else if(view_id == VIEW_IGNORE) {
    } else if(view_id == VIEW_DESTROY) {
        view_dispatcher_free(view_dispatcher);
    } else {
        View** view_pp = ViewDict_get(view_dispatcher->views, view_id);
        furi_check(view_pp != NULL);
        view_dispatcher->current_view = *view_pp;
        view_port_enabled_set(view_dispatcher->view_port, true);
        view_port_update(view_dispatcher->view_port);
    }
}

void view_dispatcher_attach_to_gui(
    ViewDispatcher* view_dispatcher,
    Gui* gui,
    ViewDispatcherType type) {
    furi_assert(view_dispatcher);
    furi_assert(view_dispatcher->gui == NULL);
    furi_assert(gui);

    if(type == ViewDispatcherTypeNone) {
        gui_add_view_port(gui, view_dispatcher->view_port, GuiLayerNone);
    } else if(type == ViewDispatcherTypeFullscreen) {
        gui_add_view_port(gui, view_dispatcher->view_port, GuiLayerFullscreen);
    } else if(type == ViewDispatcherTypeWindow) {
        gui_add_view_port(gui, view_dispatcher->view_port, GuiLayerMain);
    } else {
        furi_check(NULL);
    }
    view_dispatcher->gui = gui;
}

void view_dispatcher_draw_callback(Canvas* canvas, void* context) {
    ViewDispatcher* view_dispatcher = context;
    if(view_dispatcher->current_view) {
        view_draw(view_dispatcher->current_view, canvas);
    }
}

void view_dispatcher_input_callback(InputEvent* event, void* context) {
    ViewDispatcher* view_dispatcher = context;
    bool is_consumed = false;
    if(view_dispatcher->current_view) {
        is_consumed = view_input(view_dispatcher->current_view, event);
    }
    if(!is_consumed && event->state) {
        uint32_t view_id = VIEW_IGNORE;
        if(event->input == InputBack) {
            view_id = view_previous(view_dispatcher->current_view);
        } else if(event->input == InputOk) {
            view_id = view_next(view_dispatcher->current_view);
        }
        view_dispatcher_switch_to_view(view_dispatcher, view_id);
    }
}

void view_dispatcher_update(ViewDispatcher* view_dispatcher, View* view) {
    furi_assert(view_dispatcher);
    furi_assert(view);

    if(view_dispatcher->current_view == view) {
        view_port_update(view_dispatcher->view_port);
    }
}
