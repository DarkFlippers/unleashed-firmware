#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <input/input.h>

#define MOUSE_MOVE_SHORT 5
#define MOUSE_MOVE_LONG 20

typedef enum {
    EventTypeInput,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} UsbMouseEvent;

typedef struct {
    bool running;
} MouseJigglerState;

static void mouse_jiggler_render_callback(Canvas* canvas, void* ctx) {
    const MouseJigglerState* plugin_state = acquire_mutex((ValueMutex*)ctx, 25);
    if(plugin_state == NULL) {
        return;
    }

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 12, "USB Mouse Jiggler");
    if(!plugin_state->running) {
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 2, 27, " -> STOPPED");
        canvas_draw_str(canvas, 2, 51, "Press [ok] to start");
        canvas_draw_str(canvas, 2, 63, "Press [back] to exit");
    } else {
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 2, 27, " -> RUNNING");
        canvas_draw_str(canvas, 2, 51, "Press [back] to stop");
    }

    release_mutex((ValueMutex*)ctx, plugin_state);
}

static void mouse_jiggler_input_callback(InputEvent* input_event, void* ctx) {
    FuriMessageQueue* event_queue = ctx;
    furi_assert(event_queue);

    UsbMouseEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void mouse_jiggler_state_init(MouseJigglerState* const plugin_state) {
    plugin_state->running = false;
}

int32_t mouse_jiggler_app(void* p) {
    UNUSED(p);

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(UsbMouseEvent));

    MouseJigglerState* plugin_state = malloc(sizeof(MouseJigglerState));
    if(plugin_state == NULL) {
        FURI_LOG_E("MouseJiggler", "MouseJigglerState: malloc error\r\n");
        return 255;
    }
    mouse_jiggler_state_init(plugin_state);

    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, plugin_state, sizeof(MouseJigglerState))) {
        FURI_LOG_E("MouseJiggler", "cannot create mutex\r\n");
        furi_message_queue_free(event_queue);
        free(plugin_state);
        return 255;
    }

    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, mouse_jiggler_render_callback, &state_mutex);
    view_port_input_callback_set(view_port, mouse_jiggler_input_callback, event_queue);

    FuriHalUsbInterface* usb_mode_prev = furi_hal_usb_get_config();
    furi_hal_usb_set_config(&usb_hid, NULL);

    // Open GUI and register view_port
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    UsbMouseEvent event;
    //bool status = 0;

    for(bool processing = true; processing;) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);

        MouseJigglerState* plugin_state = (MouseJigglerState*)acquire_mutex_block(&state_mutex);

        if(event_status == FuriStatusOk) {
            if(event.type == EventTypeKey) {
                if(event.input.type == InputTypePress) {
                    switch(event.input.key) {
                    case InputKeyOk:
                        if(!plugin_state->running) {
                            plugin_state->running = true;
                        }
                        break;
                    case InputKeyBack:
                        if(!plugin_state->running) {
                            processing = false;
                        } else {
                            plugin_state->running = false;
                        }
                        break;
                    default:
                        break;
                    }
                }
            }
        }

        if(plugin_state->running) {
            furi_hal_hid_mouse_move(MOUSE_MOVE_SHORT, 0);
            furi_delay_ms(500);
            furi_hal_hid_mouse_move(-MOUSE_MOVE_SHORT, 0);
            furi_delay_ms(500);
        }

        view_port_update(view_port);
        release_mutex(&state_mutex, plugin_state);
    }

    furi_hal_usb_set_config(usb_mode_prev, NULL);

    // remove & free all stuff created by app
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    delete_mutex(&state_mutex);

    return 0;
}
