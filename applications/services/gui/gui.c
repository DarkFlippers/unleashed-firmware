#include "gui_i.h"
#include <assets_icons.h>

#define TAG "GuiSrv"

ViewPort* gui_view_port_find_enabled(ViewPortArray_t array) {
    // Iterating backward
    ViewPortArray_it_t it;
    ViewPortArray_it_last(it, array);
    while(!ViewPortArray_end_p(it)) {
        ViewPort* view_port = *ViewPortArray_ref(it);
        if(view_port_is_enabled(view_port)) {
            return view_port;
        }
        ViewPortArray_previous(it);
    }
    return NULL;
}

void gui_update(Gui* gui) {
    furi_assert(gui);
    if(!gui->direct_draw) furi_thread_flags_set(gui->thread_id, GUI_THREAD_FLAG_DRAW);
}

void gui_input_events_callback(const void* value, void* ctx) {
    furi_assert(value);
    furi_assert(ctx);

    Gui* gui = ctx;

    furi_message_queue_put(gui->input_queue, value, FuriWaitForever);
    furi_thread_flags_set(gui->thread_id, GUI_THREAD_FLAG_INPUT);
}

// Only Fullscreen supports vertical display for now
static bool gui_redraw_fs(Gui* gui) {
    canvas_set_orientation(gui->canvas, CanvasOrientationHorizontal);
    canvas_frame_set(gui->canvas, 0, 0, GUI_DISPLAY_WIDTH, GUI_DISPLAY_HEIGHT);
    ViewPort* view_port = gui_view_port_find_enabled(gui->layers[GuiLayerFullscreen]);
    if(view_port) {
        view_port_draw(view_port, gui->canvas);
        return true;
    } else {
        return false;
    }
}

static void gui_redraw_status_bar(Gui* gui, bool need_attention) {
    ViewPortArray_it_t it;
    uint8_t left_used = 0;
    uint8_t right_used = 0;
    uint8_t width;
    canvas_set_orientation(gui->canvas, CanvasOrientationHorizontal);
    canvas_frame_set(
        gui->canvas, GUI_STATUS_BAR_X, GUI_STATUS_BAR_Y, GUI_DISPLAY_WIDTH, GUI_STATUS_BAR_HEIGHT);

    /* for support black theme - paint white area and
     * draw icon with transparent white color
     */
    canvas_set_color(gui->canvas, ColorWhite);
    canvas_draw_box(gui->canvas, 1, 1, 9, 7);
    canvas_draw_box(gui->canvas, 7, 3, 58, 6);
    canvas_draw_box(gui->canvas, 61, 1, 32, 7);
    canvas_draw_box(gui->canvas, 89, 3, 38, 6);
    canvas_set_color(gui->canvas, ColorBlack);
    canvas_set_bitmap_mode(gui->canvas, 1);
    canvas_draw_icon(gui->canvas, 0, 0, &I_Background_128x11);
    canvas_set_bitmap_mode(gui->canvas, 0);

    // Right side
    uint8_t x = GUI_DISPLAY_WIDTH - 1;
    ViewPortArray_it(it, gui->layers[GuiLayerStatusBarRight]);
    while(!ViewPortArray_end_p(it) && right_used < GUI_STATUS_BAR_WIDTH) {
        ViewPort* view_port = *ViewPortArray_ref(it);
        if(view_port_is_enabled(view_port)) {
            width = view_port_get_width(view_port);
            if(!width) width = 8;
            // Recalculate next position
            right_used += (width + 2);
            x -= (width + 2);
            // Prepare work area background
            canvas_frame_set(
                gui->canvas,
                x - 1,
                GUI_STATUS_BAR_Y + 1,
                width + 2,
                GUI_STATUS_BAR_WORKAREA_HEIGHT + 2);
            canvas_set_color(gui->canvas, ColorWhite);
            canvas_draw_box(
                gui->canvas, 0, 0, canvas_width(gui->canvas), canvas_height(gui->canvas));
            canvas_set_color(gui->canvas, ColorBlack);
            // ViewPort draw
            canvas_frame_set(
                gui->canvas, x, GUI_STATUS_BAR_Y + 2, width, GUI_STATUS_BAR_WORKAREA_HEIGHT);
            view_port_draw(view_port, gui->canvas);
        }
        ViewPortArray_next(it);
    }
    // Draw frame around icons on the right
    if(right_used) {
        canvas_frame_set(
            gui->canvas,
            GUI_DISPLAY_WIDTH - 3 - right_used,
            GUI_STATUS_BAR_Y,
            right_used + 3,
            GUI_STATUS_BAR_HEIGHT);
        canvas_set_color(gui->canvas, ColorBlack);
        canvas_draw_rframe(
            gui->canvas, 0, 0, canvas_width(gui->canvas), canvas_height(gui->canvas), 1);
        canvas_draw_line(
            gui->canvas,
            canvas_width(gui->canvas) - 2,
            1,
            canvas_width(gui->canvas) - 2,
            canvas_height(gui->canvas) - 2);
        canvas_draw_line(
            gui->canvas,
            1,
            canvas_height(gui->canvas) - 2,
            canvas_width(gui->canvas) - 2,
            canvas_height(gui->canvas) - 2);
    }

    // Left side
    x = 2;
    ViewPortArray_it(it, gui->layers[GuiLayerStatusBarLeft]);
    while(!ViewPortArray_end_p(it) && (right_used + left_used) < GUI_STATUS_BAR_WIDTH) {
        ViewPort* view_port = *ViewPortArray_ref(it);
        if(view_port_is_enabled(view_port)) {
            width = view_port_get_width(view_port);
            if(!width) width = 8;
            // Prepare work area background
            canvas_frame_set(
                gui->canvas,
                x - 1,
                GUI_STATUS_BAR_Y + 1,
                width + 2,
                GUI_STATUS_BAR_WORKAREA_HEIGHT + 2);
            canvas_set_color(gui->canvas, ColorWhite);
            canvas_draw_box(
                gui->canvas, 0, 0, canvas_width(gui->canvas), canvas_height(gui->canvas));
            canvas_set_color(gui->canvas, ColorBlack);
            // ViewPort draw
            canvas_frame_set(
                gui->canvas, x, GUI_STATUS_BAR_Y + 2, width, GUI_STATUS_BAR_WORKAREA_HEIGHT);
            view_port_draw(view_port, gui->canvas);
            // Recalculate next position
            left_used += (width + 2);
            x += (width + 2);
        }
        ViewPortArray_next(it);
    }
    // Extra notification
    if(need_attention) {
        width = icon_get_width(&I_Hidden_window_9x8);
        // Prepare work area background
        canvas_frame_set(
            gui->canvas,
            x - 1,
            GUI_STATUS_BAR_Y + 1,
            width + 2,
            GUI_STATUS_BAR_WORKAREA_HEIGHT + 2);
        canvas_set_color(gui->canvas, ColorWhite);
        canvas_draw_box(gui->canvas, 0, 0, canvas_width(gui->canvas), canvas_height(gui->canvas));
        canvas_set_color(gui->canvas, ColorBlack);
        // Draw Icon
        canvas_frame_set(
            gui->canvas, x, GUI_STATUS_BAR_Y + 2, width, GUI_STATUS_BAR_WORKAREA_HEIGHT);
        canvas_draw_icon(gui->canvas, 0, 0, &I_Hidden_window_9x8);
        // Recalculate next position
        left_used += (width + 2);
        x += (width + 2);
    }
    // Draw frame around icons on the left
    if(left_used) {
        canvas_frame_set(gui->canvas, 0, 0, left_used + 3, GUI_STATUS_BAR_HEIGHT);
        canvas_draw_rframe(
            gui->canvas, 0, 0, canvas_width(gui->canvas), canvas_height(gui->canvas), 1);
        canvas_draw_line(
            gui->canvas,
            canvas_width(gui->canvas) - 2,
            1,
            canvas_width(gui->canvas) - 2,
            canvas_height(gui->canvas) - 2);
        canvas_draw_line(
            gui->canvas,
            1,
            canvas_height(gui->canvas) - 2,
            canvas_width(gui->canvas) - 2,
            canvas_height(gui->canvas) - 2);
    }
}

static bool gui_redraw_window(Gui* gui) {
    canvas_set_orientation(gui->canvas, CanvasOrientationHorizontal);
    canvas_frame_set(gui->canvas, GUI_WINDOW_X, GUI_WINDOW_Y, GUI_WINDOW_WIDTH, GUI_WINDOW_HEIGHT);
    ViewPort* view_port = gui_view_port_find_enabled(gui->layers[GuiLayerWindow]);
    if(view_port) {
        view_port_draw(view_port, gui->canvas);
        return true;
    }
    return false;
}

static bool gui_redraw_desktop(Gui* gui) {
    canvas_set_orientation(gui->canvas, CanvasOrientationHorizontal);
    canvas_frame_set(gui->canvas, 0, 0, GUI_DISPLAY_WIDTH, GUI_DISPLAY_HEIGHT);
    ViewPort* view_port = gui_view_port_find_enabled(gui->layers[GuiLayerDesktop]);
    if(view_port) {
        view_port_draw(view_port, gui->canvas);
        return true;
    }

    return false;
}

static void gui_redraw(Gui* gui) {
    furi_assert(gui);
    gui_lock(gui);

    do {
        if(gui->direct_draw) break;

        canvas_reset(gui->canvas);

        if(gui->lockdown) {
            gui_redraw_desktop(gui);
            bool need_attention =
                (gui_view_port_find_enabled(gui->layers[GuiLayerWindow]) != 0 ||
                 gui_view_port_find_enabled(gui->layers[GuiLayerFullscreen]) != 0);
            gui_redraw_status_bar(gui, need_attention);
        } else {
            if(!gui_redraw_fs(gui)) {
                if(!gui_redraw_window(gui)) {
                    gui_redraw_desktop(gui);
                }
                gui_redraw_status_bar(gui, false);
            }
        }

        canvas_commit(gui->canvas);
        for
            M_EACH(p, gui->canvas_callback_pair, CanvasCallbackPairArray_t) {
                p->callback(
                    canvas_get_buffer(gui->canvas),
                    canvas_get_buffer_size(gui->canvas),
                    p->context);
            }
    } while(false);

    gui_unlock(gui);
}

static void gui_input(Gui* gui, InputEvent* input_event) {
    furi_assert(gui);
    furi_assert(input_event);

    // Check input complementarity
    uint8_t key_bit = (1 << input_event->key);
    if(input_event->type == InputTypeRelease) {
        gui->ongoing_input &= ~key_bit;
    } else if(input_event->type == InputTypePress) {
        gui->ongoing_input |= key_bit;
    } else if(!(gui->ongoing_input & key_bit)) {
        FURI_LOG_D(
            TAG,
            "non-complementary input, discarding key: %s type: %s, sequence: %p",
            input_get_key_name(input_event->key),
            input_get_type_name(input_event->type),
            (void*)input_event->sequence);
        return;
    }

    gui_lock(gui);

    do {
        if(gui->direct_draw && !gui->ongoing_input_view_port) {
            break;
        }

        ViewPort* view_port = NULL;

        if(gui->lockdown) {
            view_port = gui_view_port_find_enabled(gui->layers[GuiLayerDesktop]);
        } else {
            view_port = gui_view_port_find_enabled(gui->layers[GuiLayerFullscreen]);
            if(!view_port) view_port = gui_view_port_find_enabled(gui->layers[GuiLayerWindow]);
            if(!view_port) view_port = gui_view_port_find_enabled(gui->layers[GuiLayerDesktop]);
        }

        if(!(gui->ongoing_input & ~key_bit) && input_event->type == InputTypePress) {
            gui->ongoing_input_view_port = view_port;
        }

        if(view_port && view_port == gui->ongoing_input_view_port) {
            view_port_input(view_port, input_event);
        } else if(gui->ongoing_input_view_port && input_event->type == InputTypeRelease) {
            FURI_LOG_D(
                TAG,
                "ViewPort changed while key press %p -> %p. Sending key: %s, type: %s, sequence: %p to previous view port",
                gui->ongoing_input_view_port,
                view_port,
                input_get_key_name(input_event->key),
                input_get_type_name(input_event->type),
                (void*)input_event->sequence);
            view_port_input(gui->ongoing_input_view_port, input_event);
        } else {
            FURI_LOG_D(
                TAG,
                "ViewPort changed while key press %p -> %p. Discarding key: %s, type: %s, sequence: %p",
                gui->ongoing_input_view_port,
                view_port,
                input_get_key_name(input_event->key),
                input_get_type_name(input_event->type),
                (void*)input_event->sequence);
        }
    } while(false);

    gui_unlock(gui);
}

void gui_lock(Gui* gui) {
    furi_assert(gui);
    furi_check(furi_mutex_acquire(gui->mutex, FuriWaitForever) == FuriStatusOk);
}

void gui_unlock(Gui* gui) {
    furi_assert(gui);
    furi_check(furi_mutex_release(gui->mutex) == FuriStatusOk);
}

void gui_add_view_port(Gui* gui, ViewPort* view_port, GuiLayer layer) {
    furi_assert(gui);
    furi_assert(view_port);
    furi_check(layer < GuiLayerMAX);
    // Only fullscreen supports Vertical orientation for now
    furi_assert(
        (layer == GuiLayerFullscreen) ||
        ((view_port->orientation != ViewPortOrientationVertical) &&
         (view_port->orientation != ViewPortOrientationVerticalFlip)));

    gui_lock(gui);
    // Verify that view port is not yet added
    ViewPortArray_it_t it;
    for(size_t i = 0; i < GuiLayerMAX; i++) {
        ViewPortArray_it(it, gui->layers[i]);
        while(!ViewPortArray_end_p(it)) {
            furi_assert(*ViewPortArray_ref(it) != view_port);
            ViewPortArray_next(it);
        }
    }
    // Add view port and link with gui
    ViewPortArray_push_back(gui->layers[layer], view_port);
    view_port_gui_set(view_port, gui);
    gui_unlock(gui);

    // Request redraw
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
            } else {
                ViewPortArray_next(it);
            }
        }
    }
    if(gui->ongoing_input_view_port == view_port) {
        gui->ongoing_input_view_port = NULL;
    }
    gui_unlock(gui);

    // Request redraw
    gui_update(gui);
}

void gui_view_port_send_to_front(Gui* gui, ViewPort* view_port) {
    furi_assert(gui);
    furi_assert(view_port);

    gui_lock(gui);
    // Remove
    GuiLayer layer = GuiLayerMAX;
    ViewPortArray_it_t it;
    for(size_t i = 0; i < GuiLayerMAX; i++) {
        ViewPortArray_it(it, gui->layers[i]);
        while(!ViewPortArray_end_p(it)) {
            if(*ViewPortArray_ref(it) == view_port) {
                ViewPortArray_remove(gui->layers[i], it);
                furi_assert(layer == GuiLayerMAX);
                layer = i;
            } else {
                ViewPortArray_next(it);
            }
        }
    }
    furi_assert(layer != GuiLayerMAX);
    // Return to the top
    ViewPortArray_push_back(gui->layers[layer], view_port);
    gui_unlock(gui);

    // Request redraw
    gui_update(gui);
}

void gui_view_port_send_to_back(Gui* gui, ViewPort* view_port) {
    furi_assert(gui);
    furi_assert(view_port);

    gui_lock(gui);
    // Remove
    GuiLayer layer = GuiLayerMAX;
    ViewPortArray_it_t it;
    for(size_t i = 0; i < GuiLayerMAX; i++) {
        ViewPortArray_it(it, gui->layers[i]);
        while(!ViewPortArray_end_p(it)) {
            if(*ViewPortArray_ref(it) == view_port) {
                ViewPortArray_remove(gui->layers[i], it);
                furi_assert(layer == GuiLayerMAX);
                layer = i;
            } else {
                ViewPortArray_next(it);
            }
        }
    }
    furi_assert(layer != GuiLayerMAX);
    // Return to the top
    ViewPortArray_push_at(gui->layers[layer], 0, view_port);
    gui_unlock(gui);

    // Request redraw
    gui_update(gui);
}

void gui_add_framebuffer_callback(Gui* gui, GuiCanvasCommitCallback callback, void* context) {
    furi_assert(gui);

    const CanvasCallbackPair p = {callback, context};

    gui_lock(gui);
    furi_assert(!CanvasCallbackPairArray_count(gui->canvas_callback_pair, p));
    CanvasCallbackPairArray_push_back(gui->canvas_callback_pair, p);
    gui_unlock(gui);

    // Request redraw
    gui_update(gui);
}

void gui_remove_framebuffer_callback(Gui* gui, GuiCanvasCommitCallback callback, void* context) {
    furi_assert(gui);

    const CanvasCallbackPair p = {callback, context};

    gui_lock(gui);
    furi_assert(CanvasCallbackPairArray_count(gui->canvas_callback_pair, p) == 1);
    CanvasCallbackPairArray_remove_val(gui->canvas_callback_pair, p);
    gui_unlock(gui);
}

size_t gui_get_framebuffer_size(const Gui* gui) {
    furi_assert(gui);
    return canvas_get_buffer_size(gui->canvas);
}

void gui_set_lockdown(Gui* gui, bool lockdown) {
    furi_assert(gui);

    gui_lock(gui);
    gui->lockdown = lockdown;
    gui_unlock(gui);

    // Request redraw
    gui_update(gui);
}

Canvas* gui_direct_draw_acquire(Gui* gui) {
    furi_assert(gui);
    gui_lock(gui);
    gui->direct_draw = true;
    gui_unlock(gui);

    canvas_reset(gui->canvas);
    canvas_commit(gui->canvas);

    return gui->canvas;
}

void gui_direct_draw_release(Gui* gui) {
    furi_assert(gui);

    canvas_reset(gui->canvas);
    canvas_commit(gui->canvas);

    gui_lock(gui);
    gui->direct_draw = false;
    gui_unlock(gui);

    gui_update(gui);
}

Gui* gui_alloc() {
    Gui* gui = malloc(sizeof(Gui));
    // Thread ID
    gui->thread_id = furi_thread_get_current_id();
    // Allocate mutex
    gui->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    furi_check(gui->mutex);
    // Layers
    for(size_t i = 0; i < GuiLayerMAX; i++) {
        ViewPortArray_init(gui->layers[i]);
    }
    // Drawing canvas
    gui->canvas = canvas_init();
    CanvasCallbackPairArray_init(gui->canvas_callback_pair);

    // Input
    gui->input_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    gui->input_events = furi_record_open(RECORD_INPUT_EVENTS);

    furi_check(gui->input_events);
    furi_pubsub_subscribe(gui->input_events, gui_input_events_callback, gui);

    return gui;
}

int32_t gui_srv(void* p) {
    UNUSED(p);
    Gui* gui = gui_alloc();

    furi_record_create(RECORD_GUI, gui);

    while(1) {
        uint32_t flags =
            furi_thread_flags_wait(GUI_THREAD_FLAG_ALL, FuriFlagWaitAny, FuriWaitForever);
        // Process and dispatch input
        if(flags & GUI_THREAD_FLAG_INPUT) {
            // Process till queue become empty
            InputEvent input_event;
            while(furi_message_queue_get(gui->input_queue, &input_event, 0) == FuriStatusOk) {
                gui_input(gui, &input_event);
            }
        }
        // Process and dispatch draw call
        if(flags & GUI_THREAD_FLAG_DRAW) {
            // Clear flags that arrived on input step
            furi_thread_flags_clear(GUI_THREAD_FLAG_DRAW);
            gui_redraw(gui);
        }
    }

    return 0;
}
