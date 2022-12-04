#include <furi.h>
#include <math.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} PluginEvent;

typedef struct {
    float xZoom;
    float yZoom;
    float xOffset;
    float yOffset;
    float zoom;
} PluginState;

bool mandelbrot_pixel(int x, int y, float xZoom, float yZoom, float xOffset, float yOffset) {
    float ratio = 128.0 / 64.0;
    //x0 := scaled x coordinate of pixel (scaled to lie in the Mandelbrot X scale (-2.00, 0.47))
    float x0 = (((x / 128.0) * ratio * xZoom)) - xOffset;
    //y0 := scaled y coordinate of pixel (scaled to lie in the Mandelbrot Y scale (-1.12, 1.12))
    float y0 = ((y / 64.0) * yZoom) - yOffset;
    float x1 = 0.0;
    float y1 = 0.0;
    float x2 = 0.0;
    float y2 = 0.0;

    int iteration = 0;
    int max_iteration = 50;

    while(x2 + y2 <= 4.0 && iteration < max_iteration) {
        y1 = 2.0 * x1 * y1 + y0;
        x1 = x2 - y2 + x0;
        x2 = x1 * x1;
        y2 = y1 * y1;
        iteration++;
    }

    if(iteration > 49) {
        return true;
    }

    return false;
}

static void render_callback(Canvas* const canvas, void* ctx) {
    const PluginState* plugin_state = acquire_mutex((ValueMutex*)ctx, 25);
    if(plugin_state == NULL) {
        return;
    }
    // border around the edge of the screen
    canvas_draw_frame(canvas, 0, 0, 128, 64);

    for(int y = 0; y < 64; y++) {
        for(int x = 0; x < 128; x++) {
            // did you know if you just pass the indivdiual bits of plugin_state instead of plugin_state
            // you dont get any compiler warnings :)
            if(mandelbrot_pixel(
                   x,
                   y,
                   plugin_state->xZoom,
                   plugin_state->yZoom,
                   plugin_state->xOffset,
                   plugin_state->yOffset)) {
                canvas_draw_dot(canvas, x, y);
            }
        }
    }

    release_mutex((ValueMutex*)ctx, plugin_state);
}

static void input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    PluginEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void mandelbrot_state_init(PluginState* const plugin_state) {
    plugin_state->xOffset = 3.0;
    plugin_state->yOffset = 1.12;
    plugin_state->xZoom = 2.47;
    plugin_state->yZoom = 2.24;
    plugin_state->zoom = 1; // this controls the camera when
}

int32_t mandelbrot_app(void* p) {
    UNUSED(p);

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(PluginEvent));

    PluginState* plugin_state = malloc(sizeof(PluginState));
    mandelbrot_state_init(plugin_state);
    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, plugin_state, sizeof(PluginState))) {
        FURI_LOG_E("mandelbrot", "cannot create mutex\r\n");
        furi_message_queue_free(event_queue);
        free(plugin_state);
        return 255;
    }

    // Set system callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, render_callback, &state_mutex);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    // Open GUI and register view_port
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    PluginEvent event;
    for(bool processing = true; processing;) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);
        PluginState* plugin_state = (PluginState*)acquire_mutex_block(&state_mutex);

        if(event_status == FuriStatusOk) {
            // press events
            if(event.type == EventTypeKey) {
                if(event.input.type == InputTypePress) {
                    switch(event.input.key) {
                    case InputKeyUp:
                        plugin_state->yOffset += 0.1 / plugin_state->zoom;
                        break;
                    case InputKeyDown:
                        plugin_state->yOffset += -0.1 / plugin_state->zoom;
                        break;
                    case InputKeyRight:
                        plugin_state->xOffset += -0.1 / plugin_state->zoom;
                        break;
                    case InputKeyLeft:
                        plugin_state->xOffset += 0.1 / plugin_state->zoom;
                        break;
                    case InputKeyOk:
                        plugin_state->xZoom -= (2.47 / 10) / plugin_state->zoom;
                        plugin_state->yZoom -= (2.24 / 10) / plugin_state->zoom;
                        // used to make camera control finer the more zoomed you are
                        // this needs to be some sort of curve
                        plugin_state->zoom += 0.15;
                        break;
                    case InputKeyBack:
                        processing = false;
                        break;
                    default:
                        break;
                    }
                }
            }
        } else {
            FURI_LOG_D("mandelbrot", "osMessageQueue: event timeout");
            // event timeout
        }
        view_port_update(view_port);
        release_mutex(&state_mutex, plugin_state);
    }

    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);

    return 0;
}