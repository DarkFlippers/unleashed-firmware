#include "gui_i.h"

static void gui_rotate_buttons(InputEvent* event) {
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

static void gui_setup_fs_orientation(const ViewPort* view_port, Canvas* canvas) {
    ViewPortOrientation view_port_orientation = view_port_get_orientation(view_port);
    CanvasOrientation canvas_orientation = canvas_get_orientation(canvas);
    if(view_port_orientation == ViewPortOrientationHorizontal) {
        canvas_frame_set(canvas, 0, 0, GUI_DISPLAY_WIDTH, GUI_DISPLAY_HEIGHT);
        if(canvas_orientation != CanvasOrientationHorizontal) {
            canvas_set_orientation(canvas, CanvasOrientationHorizontal);
        }
    } else if(view_port_orientation == ViewPortOrientationVertical) {
        canvas_frame_set(canvas, 0, 0, GUI_DISPLAY_HEIGHT, GUI_DISPLAY_WIDTH);
        if(canvas_orientation != CanvasOrientationVertical) {
            canvas_set_orientation(canvas, CanvasOrientationVertical);
        }
    }
}

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
    osThreadFlagsSet(gui->thread, GUI_THREAD_FLAG_DRAW);
}

void gui_input_events_callback(const void* value, void* ctx) {
    furi_assert(value);
    furi_assert(ctx);

    Gui* gui = ctx;

    osMessageQueuePut(gui->input_queue, value, 0, osWaitForever);
    osThreadFlagsSet(gui->thread, GUI_THREAD_FLAG_INPUT);
}

// Only Fullscreen supports vertical display for now
bool gui_redraw_fs(Gui* gui) {
    ViewPort* view_port = gui_view_port_find_enabled(gui->layers[GuiLayerFullscreen]);
    if(view_port) {
        gui_setup_fs_orientation(view_port, gui->canvas);
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
    canvas_set_orientation(gui->canvas, CanvasOrientationHorizontal);
    canvas_frame_set(
        gui->canvas, GUI_STATUS_BAR_X, GUI_STATUS_BAR_Y, GUI_DISPLAY_WIDTH, GUI_STATUS_BAR_HEIGHT);
    canvas_draw_icon(gui->canvas, 0, 0, &I_Background_128x11);

    // Right side
    x = GUI_DISPLAY_WIDTH;
    ViewPortArray_it(it, gui->layers[GuiLayerStatusBarRight]);
    while(!ViewPortArray_end_p(it) && x_used < GUI_STATUS_BAR_WIDTH) {
        // Render view_port;
        view_port = *ViewPortArray_ref(it);
        if(view_port_is_enabled(view_port)) {
            width = view_port_get_width(view_port);
            if(!width) width = 8;
            x_used += width;
            x -= (width + 2);
            canvas_frame_set(gui->canvas, x - 3, GUI_STATUS_BAR_Y, width, GUI_STATUS_BAR_HEIGHT);

            canvas_set_color(gui->canvas, ColorWhite);
            canvas_draw_box(gui->canvas, 1, 1, width + 3, 11);

            canvas_set_color(gui->canvas, ColorBlack);

            canvas_draw_box(gui->canvas, 1, 0, 1, 12);
            canvas_draw_box(gui->canvas, 0, 1, 1, 11);

            canvas_draw_box(gui->canvas, 1, 11, width + 4, 2);
            canvas_draw_box(gui->canvas, 1, 0, width + 4, 1);
            canvas_draw_box(gui->canvas, width + 4, 1, 1, 11);

            canvas_set_color(gui->canvas, ColorWhite);
            canvas_draw_dot(gui->canvas, width + 4, 0);
            canvas_draw_dot(gui->canvas, width + 4, 12);

            canvas_set_color(gui->canvas, ColorBlack);

            canvas_frame_set(gui->canvas, x, GUI_STATUS_BAR_Y + 2, width, GUI_STATUS_BAR_HEIGHT);

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

            canvas_set_color(gui->canvas, ColorWhite);
            canvas_draw_box(gui->canvas, 1, 1, width + 3, 11);

            canvas_set_color(gui->canvas, ColorBlack);

            if(x == 0) { // Frame start
                canvas_draw_box(gui->canvas, 1, 0, 1, 12);
                canvas_draw_box(gui->canvas, 0, 1, 1, 11);
            }

            canvas_draw_box(gui->canvas, 1, 11, width + 4, 2);
            canvas_draw_box(gui->canvas, 1, 0, width + 4, 1);
            canvas_draw_box(gui->canvas, width + 4, 0, 1, 12);

            canvas_set_color(gui->canvas, ColorWhite);
            canvas_draw_dot(gui->canvas, width + 4, 0);
            canvas_draw_dot(gui->canvas, width + 4, 12);

            canvas_set_color(gui->canvas, ColorBlack);

            canvas_frame_set(
                gui->canvas, x + 3, GUI_STATUS_BAR_Y + 2, width, GUI_STATUS_BAR_HEIGHT);
            view_port_draw(view_port, gui->canvas);

            x += (width + 2);
        }
        ViewPortArray_next(it);
    }
}

bool gui_redraw_normal(Gui* gui) {
    canvas_set_orientation(gui->canvas, CanvasOrientationHorizontal);
    canvas_frame_set(gui->canvas, GUI_MAIN_X, GUI_MAIN_Y, GUI_MAIN_WIDTH, GUI_MAIN_HEIGHT);
    ViewPort* view_port = gui_view_port_find_enabled(gui->layers[GuiLayerMain]);
    if(view_port) {
        view_port_draw(view_port, gui->canvas);
        return true;
    }
    return false;
}

bool gui_redraw_none(Gui* gui) {
    canvas_set_orientation(gui->canvas, CanvasOrientationHorizontal);
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
    if(gui->canvas_callback) {
        gui->canvas_callback(
            canvas_get_buffer(gui->canvas),
            canvas_get_buffer_size(gui->canvas),
            gui->canvas_callback_context);
    }
    gui_unlock(gui);
}

void gui_input(Gui* gui, InputEvent* input_event) {
    furi_assert(gui);
    furi_assert(input_event);

    // Check input complementarity
    uint8_t key_bit = (1 << input_event->key);
    if(input_event->type == InputTypeRelease) {
        gui->ongoing_input &= ~key_bit;
    } else if(input_event->type == InputTypePress) {
        gui->ongoing_input |= key_bit;
    } else if(!(gui->ongoing_input & key_bit)) {
        FURI_LOG_W(
            "Gui",
            "non-complementary input, discarding key %s type %s",
            input_get_key_name(input_event->key),
            input_get_type_name(input_event->type));
        return;
    }

    gui_lock(gui);

    ViewPort* view_port = gui_view_port_find_enabled(gui->layers[GuiLayerFullscreen]);
    if(!view_port) view_port = gui_view_port_find_enabled(gui->layers[GuiLayerMain]);
    if(!view_port) view_port = gui_view_port_find_enabled(gui->layers[GuiLayerNone]);

    if(!(gui->ongoing_input & ~key_bit) && input_event->type == InputTypePress) {
        gui->ongoing_input_view_port = view_port;
    }

    if(view_port) {
        if(view_port == gui->ongoing_input_view_port) {
            if(view_port_get_orientation(view_port) == ViewPortOrientationVertical) {
                gui_rotate_buttons(input_event);
            }
            view_port_input(view_port, input_event);
        } else {
            FURI_LOG_W(
                "Gui",
                "ViewPort change while key press %x -> %x. Discarding key: %s, type: %s",
                gui->ongoing_input_view_port,
                view_port,
                input_get_key_name(input_event->key),
                input_get_type_name(input_event->type));
        }
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

void gui_cli_screen_stream_callback(uint8_t* data, size_t size, void* context) {
    furi_assert(data);
    furi_assert(size == 1024);
    furi_assert(context);

    Gui* gui = context;
    const uint8_t magic[] = {0xF0, 0xE1, 0xD2, 0xC3};
    cli_write(gui->cli, magic, sizeof(magic));
    cli_write(gui->cli, data, size);
}

void gui_cli_screen_stream(Cli* cli, string_t args, void* context) {
    furi_assert(context);
    Gui* gui = context;
    gui_set_framebuffer_callback_context(gui, gui);
    gui_set_framebuffer_callback(gui, gui_cli_screen_stream_callback);
    gui_redraw(gui);

    // Wait for control events
    while(true) {
        char c = cli_getc(gui->cli);
        if(c == CliSymbolAsciiEsc) {
            c = cli_getc(gui->cli);
            if(c == 'i') {
                InputEvent input_event;
                input_event.key = cli_getc(gui->cli);
                input_event.type = cli_getc(gui->cli);
                osMessageQueuePut(gui->input_queue, &input_event, 0, osWaitForever);
                osThreadFlagsSet(gui->thread, GUI_THREAD_FLAG_INPUT);
            }
        } else {
            break;
        }
    }

    gui_set_framebuffer_callback(gui, NULL);
    gui_set_framebuffer_callback_context(gui, NULL);
}

void gui_add_view_port(Gui* gui, ViewPort* view_port, GuiLayer layer) {
    furi_assert(gui);
    furi_assert(view_port);
    furi_check(layer < GuiLayerMAX);
    // Only fullscreen supports Vertical orientation for now
    furi_assert(
        (layer == GuiLayerFullscreen) || (view_port->orientation != ViewPortOrientationVertical));

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

    gui_unlock(gui);
}

void gui_send_view_port_front(Gui* gui, ViewPort* view_port) {
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
}

void gui_send_view_port_back(Gui* gui, ViewPort* view_port) {
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
}

void gui_set_framebuffer_callback(Gui* gui, GuiCanvasCommitCallback callback) {
    furi_assert(gui);
    gui->canvas_callback = callback;
}

void gui_set_framebuffer_callback_context(Gui* gui, void* context) {
    furi_assert(gui);
    gui->canvas_callback_context = context;
}

Gui* gui_alloc() {
    Gui* gui = furi_alloc(sizeof(Gui));
    // Thread ID
    gui->thread = osThreadGetId();
    // Allocate mutex
    gui->mutex = osMutexNew(NULL);
    furi_check(gui->mutex);
    // Layers
    for(size_t i = 0; i < GuiLayerMAX; i++) {
        ViewPortArray_init(gui->layers[i]);
    }
    // Drawing canvas
    gui->canvas = canvas_init();
    // Input
    gui->input_queue = osMessageQueueNew(8, sizeof(InputEvent), NULL);
    gui->input_events = furi_record_open("input_events");
    furi_check(gui->input_events);
    subscribe_pubsub(gui->input_events, gui_input_events_callback, gui);
    // Cli
    gui->cli = furi_record_open("cli");
    cli_add_command(
        gui->cli, "screen_stream", CliCommandFlagParallelSafe, gui_cli_screen_stream, gui);

    return gui;
}

int32_t gui_srv(void* p) {
    Gui* gui = gui_alloc();

    furi_record_create("gui", gui);

    while(1) {
        uint32_t flags = osThreadFlagsWait(GUI_THREAD_FLAG_ALL, osFlagsWaitAny, osWaitForever);
        // Process and dispatch input
        if(flags & GUI_THREAD_FLAG_INPUT) {
            // Process till queue become empty
            InputEvent input_event;
            while(osMessageQueueGet(gui->input_queue, &input_event, NULL, 0) == osOK) {
                gui_input(gui, &input_event);
            }
        }
        // Process and dispatch draw call
        if(flags & GUI_THREAD_FLAG_DRAW) {
            // Clear flags that arrived on input step
            osThreadFlagsClear(GUI_THREAD_FLAG_DRAW);
            gui_redraw(gui);
        }
    }

    return 0;
}
