#include "flipper_v2.h"

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    union {
        InputEvent input;
    } value;
    EventType type;
} AppEvent;

typedef struct {
    uint32_t freq_khz;
    bool on;
} State;

static void render_callback(CanvasApi* canvas, void* ctx) {
    State* state = (State*)acquire_mutex((ValueMutex*)ctx, 25);

    canvas->clear(canvas);

    canvas->set_color(canvas, ColorBlack);
    canvas->set_font(canvas, FontPrimary);
    canvas->draw_str(canvas, 2, 12, "LF RFID");

    canvas->draw_str(canvas, 2, 24, state->on ? "ON" : "OFF");
    char buf[12];
    sprintf(buf, "%d kHz", state->freq_khz);
    canvas->draw_str(canvas, 2, 36, buf);

    release_mutex((ValueMutex*)ctx, state);
}

static void input_callback(InputEvent* input_event, void* ctx) {
    osMessageQueueId_t event_queue = (QueueHandle_t)ctx;

    AppEvent event;
    event.type = EventTypeKey;
    event.value.input = *input_event;
    osMessageQueuePut(event_queue, &event, 0, 0);
}

extern TIM_HandleTypeDef htim15;

void lf_rfid_workaround(void* p) {
    osMessageQueueId_t event_queue = osMessageQueueNew(1, sizeof(AppEvent), NULL);

    State _state;
    _state.freq_khz = 125;
    _state.on = false;

    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, &_state, sizeof(State))) {
        printf("cannot create mutex\n");
        furiac_exit(NULL);
    }

    Widget* widget = widget_alloc();

    widget_draw_callback_set(widget, render_callback, &state_mutex);
    widget_input_callback_set(widget, input_callback, event_queue);

    // Open GUI and register widget
    GuiApi* gui = (GuiApi*)furi_open("gui");
    if(gui == NULL) {
        printf("gui is not available\n");
        furiac_exit(NULL);
    }
    gui->add_widget(gui, widget, WidgetLayerFullscreen);

    AppEvent event;
    while(1) {
        osStatus_t event_status = osMessageQueueGet(event_queue, &event, NULL, 10000);
        State* state = (State*)acquire_mutex_block(&state_mutex);

        if(event_status == osOK) {
            if(event.type == EventTypeKey) {
                // press events
                if(event.value.input.state && event.value.input.input == InputBack) {
                    hal_pwmn_stop(&htim15, TIM_CHANNEL_1); // TODO: move to furiac_onexit
                    // TODO remove all widgets create by app
                    widget_enabled_set(widget, false);
                    furiac_exit(NULL);
                }

                if(event.value.input.state && event.value.input.input == InputUp) {
                    state->freq_khz += 10;
                }

                if(event.value.input.state && event.value.input.input == InputDown) {
                    state->freq_khz -= 10;
                }

                if(event.value.input.state && event.value.input.input == InputLeft) {
                }

                if(event.value.input.state && event.value.input.input == InputRight) {
                }

                if(event.value.input.state && event.value.input.input == InputOk) {
                    state->on = !state->on;
                }
            }
        } else {
            // event timeout
        }

        hal_pwmn_set(
            state->on ? 0.5 : 0.0, (float)(state->freq_khz * 1000), &htim15, TIM_CHANNEL_1);

        // common code, for example, force update UI
        widget_update(widget);

        release_mutex(&state_mutex, state);
    }
}
