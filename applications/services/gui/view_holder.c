#include "view_holder.h"
#include <gui/view_i.h>

#define TAG "ViewHolder"

struct ViewHolder {
    View* view;
    ViewPort* view_port;
    Gui* gui;

    FreeCallback free_callback;
    void* free_context;

    BackCallback back_callback;
    void* back_context;

    uint8_t ongoing_input;
};

static void view_holder_draw_callback(Canvas* canvas, void* context);
static void view_holder_input_callback(InputEvent* event, void* context);

ViewHolder* view_holder_alloc() {
    ViewHolder* view_holder = malloc(sizeof(ViewHolder));

    view_holder->view_port = view_port_alloc();
    view_port_draw_callback_set(view_holder->view_port, view_holder_draw_callback, view_holder);
    view_port_input_callback_set(view_holder->view_port, view_holder_input_callback, view_holder);
    view_port_enabled_set(view_holder->view_port, false);

    return view_holder;
}

void view_holder_free(ViewHolder* view_holder) {
    furi_assert(view_holder);

    if(view_holder->gui) {
        gui_remove_view_port(view_holder->gui, view_holder->view_port);
    }

    view_port_free(view_holder->view_port);

    if(view_holder->free_callback) {
        view_holder->free_callback(view_holder->free_context);
    }

    free(view_holder);
}

void view_holder_set_view(ViewHolder* view_holder, View* view) {
    furi_assert(view_holder);
    if(view_holder->view) {
        if(view_holder->view->exit_callback) {
            view_holder->view->exit_callback(view_holder->view->context);
        }

        view_set_update_callback(view_holder->view, NULL);
        view_set_update_callback_context(view_holder->view, NULL);
    }

    view_holder->view = view;

    if(view_holder->view) {
        view_set_update_callback(view_holder->view, view_holder_update);
        view_set_update_callback_context(view_holder->view, view_holder);

        if(view_holder->view->enter_callback) {
            view_holder->view->enter_callback(view_holder->view->context);
        }
    }
}

void view_holder_set_free_callback(
    ViewHolder* view_holder,
    FreeCallback free_callback,
    void* free_context) {
    furi_assert(view_holder);
    view_holder->free_callback = free_callback;
    view_holder->free_context = free_context;
}

void* view_holder_get_free_context(ViewHolder* view_holder) {
    return view_holder->free_context;
}

void view_holder_set_back_callback(
    ViewHolder* view_holder,
    BackCallback back_callback,
    void* back_context) {
    furi_assert(view_holder);
    view_holder->back_callback = back_callback;
    view_holder->back_context = back_context;
}

void view_holder_attach_to_gui(ViewHolder* view_holder, Gui* gui) {
    furi_assert(gui);
    furi_assert(view_holder);
    view_holder->gui = gui;
    gui_add_view_port(gui, view_holder->view_port, GuiLayerFullscreen);
}

void view_holder_start(ViewHolder* view_holder) {
    view_port_enabled_set(view_holder->view_port, true);
}

void view_holder_stop(ViewHolder* view_holder) {
    while(view_holder->ongoing_input) furi_delay_tick(1);
    view_port_enabled_set(view_holder->view_port, false);
}

void view_holder_update(View* view, void* context) {
    furi_assert(view);
    furi_assert(context);

    ViewHolder* view_holder = context;
    if(view == view_holder->view) {
        view_port_update(view_holder->view_port);
    }
}

static void view_holder_draw_callback(Canvas* canvas, void* context) {
    ViewHolder* view_holder = context;
    if(view_holder->view) {
        view_draw(view_holder->view, canvas);
    }
}

static void view_holder_input_callback(InputEvent* event, void* context) {
    ViewHolder* view_holder = context;

    uint8_t key_bit = (1 << event->key);
    if(event->type == InputTypePress) {
        view_holder->ongoing_input |= key_bit;
    } else if(event->type == InputTypeRelease) {
        view_holder->ongoing_input &= ~key_bit;
    } else if(!(view_holder->ongoing_input & key_bit)) {
        FURI_LOG_W(
            TAG,
            "non-complementary input, discarding key: %s, type: %s",
            input_get_key_name(event->key),
            input_get_type_name(event->type));
        return;
    }

    bool is_consumed = false;

    if(view_holder->view) {
        is_consumed = view_input(view_holder->view, event);
    }

    if(!is_consumed && event->type == InputTypeShort) {
        if(event->key == InputKeyBack) {
            if(view_holder->back_callback) {
                view_holder->back_callback(view_holder->back_context);
            }
        }
    }
}
