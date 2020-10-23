#include "flipper.h"
#include "flipper_v2.h"
#include "irda_nec.h"
#include "irda_protocols.h"

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    union {
        InputEvent input;
    } value;
    EventType type;
} Event;

typedef struct {
    uint8_t mode_id;
    uint16_t carrier_freq;
    uint8_t carrier_duty_cycle_id;
    uint8_t nec_packet_id;
} State;

typedef void (*ModeInput)(Event*, State*);
typedef void (*ModeRender)(CanvasApi*, State*);

void input_carrier(Event* event, State* state);
void render_carrier(CanvasApi* canvas, State* state);
void input_nec(Event* event, State* state);
void render_nec(CanvasApi* canvas, State* state);

typedef struct {
    ModeRender render;
    ModeInput input;
} Mode;

typedef struct {
    uint8_t addr;
    uint8_t data;
} NecPacket;

const Mode modes[] = {
    {.render = render_carrier, .input = input_carrier},
    {.render = render_nec, .input = input_nec},
};

const NecPacket packets[] = {
    {.addr = 0xF7, .data = 0x59},
    {.addr = 0xFF, .data = 0x01},
    {.addr = 0xFF, .data = 0x10},
    {.addr = 0xFF, .data = 0x15},
    {.addr = 0xFF, .data = 0x25},
    {.addr = 0xFF, .data = 0xF0},
};

const float duty_cycles[] = {0.1, 0.25, 0.333, 0.5, 1.0};

void render_carrier(CanvasApi* canvas, State* state) {
    canvas->set_font(canvas, FontSecondary);
    canvas->draw_str(canvas, 2, 25, "carrier mode >");
    canvas->draw_str(canvas, 2, 37, "? /\\ freq | \\/ duty cycle");
    {
        char buf[24];
        sprintf(buf, "frequency: %d Hz", state->carrier_freq);
        canvas->draw_str(canvas, 2, 50, buf);
        sprintf(
            buf,
            "duty cycle: %d/1000",
            (uint32_t)(duty_cycles[state->carrier_duty_cycle_id] * 1000));
        canvas->draw_str(canvas, 2, 62, buf);
    }
}

void render_nec(CanvasApi* canvas, State* state) {
    canvas->set_font(canvas, FontSecondary);
    canvas->draw_str(canvas, 2, 25, "< nec protocol mode");
    canvas->draw_str(canvas, 2, 37, "? /\\ \\/ packet");
    {
        char buf[24];
        sprintf(
            buf,
            "packet: %02X %02X",
            packets[state->nec_packet_id].addr,
            packets[state->nec_packet_id].data);
        canvas->draw_str(canvas, 2, 50, buf);
    }
}

void input_carrier(Event* event, State* state) {
    if(event->value.input.input == InputOk) {
        if(event->value.input.state) {
            pwm_set(
                duty_cycles[state->carrier_duty_cycle_id],
                state->carrier_freq,
                &htim2,
                TIM_CHANNEL_4);
        } else {
            pwm_stop(&htim2, TIM_CHANNEL_4);
        }
    }

    if(event->value.input.state && event->value.input.input == InputUp) {
        if(state->carrier_freq < 45000) {
            state->carrier_freq += 1000;
        } else {
            state->carrier_freq = 33000;
        }
    }

    if(event->value.input.state && event->value.input.input == InputDown) {
        uint8_t duty_cycles_count = sizeof(duty_cycles) / sizeof(duty_cycles[0]);
        if(state->carrier_duty_cycle_id < (duty_cycles_count - 1)) {
            state->carrier_duty_cycle_id++;
        } else {
            state->carrier_duty_cycle_id = 0;
        }
    }
}

void input_nec(Event* event, State* state) {
    if(event->value.input.input == InputOk) {
        if(event->value.input.state) {
            vTaskSuspendAll();
            ir_nec_send(packets[state->nec_packet_id].addr, packets[state->nec_packet_id].data);
            xTaskResumeAll();
        }
    }

    if(event->value.input.state && event->value.input.input == InputUp) {
        uint8_t packets_count = sizeof(packets) / sizeof(packets[0]);
        if(state->nec_packet_id < (packets_count - 1)) {
            state->nec_packet_id++;
        } else {
            state->nec_packet_id = 0;
        }
    }

    if(event->value.input.state && event->value.input.input == InputDown) {
        uint8_t packets_count = sizeof(packets) / sizeof(packets[0]);
        if(state->nec_packet_id > 0) {
            state->nec_packet_id--;
        } else {
            state->nec_packet_id = packets_count - 1;
        }
    }
}

static void render_callback(CanvasApi* canvas, void* ctx) {
    State* state = (State*)acquire_mutex((ValueMutex*)ctx, 25);

    canvas->clear(canvas);
    canvas->set_color(canvas, ColorBlack);
    canvas->set_font(canvas, FontPrimary);
    canvas->draw_str(canvas, 2, 12, "irda test");

    modes[state->mode_id].render(canvas, state);

    release_mutex((ValueMutex*)ctx, state);
}

static void input_callback(InputEvent* input_event, void* ctx) {
    osMessageQueueId_t event_queue = (QueueHandle_t)ctx;

    Event event;
    event.type = EventTypeKey;
    event.value.input = *input_event;
    osMessageQueuePut(event_queue, &event, 0, 0);
}

void irda(void* p) {
    osMessageQueueId_t event_queue = osMessageQueueNew(1, sizeof(Event), NULL);

    State _state;
    uint8_t mode_count = sizeof(modes) / sizeof(modes[0]);
    uint8_t duty_cycles_count = sizeof(duty_cycles) / sizeof(duty_cycles[0]);

    _state.carrier_duty_cycle_id = duty_cycles_count - 2;
    _state.carrier_freq = 36000;
    _state.mode_id = 0;
    _state.nec_packet_id = 0;

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

    Event event;
    while(1) {
        osStatus_t event_status = osMessageQueueGet(event_queue, &event, NULL, osWaitForever);
        State* state = (State*)acquire_mutex_block(&state_mutex);

        if(event_status == osOK) {
            if(event.type == EventTypeKey) {
                // press events
                if(event.value.input.state && event.value.input.input == InputBack) {
                    printf("[irda] bye!\n");
                    // TODO remove all widgets create by app
                    widget_enabled_set(widget, false);
                    furiac_exit(NULL);
                }

                if(event.value.input.state && event.value.input.input == InputLeft) {
                    if(state->mode_id > 0) {
                        state->mode_id--;
                    }
                }

                if(event.value.input.state && event.value.input.input == InputRight) {
                    if(state->mode_id < (mode_count - 1)) {
                        state->mode_id++;
                    }
                }

                modes[state->mode_id].input(&event, state);
            }
        } else {
            // event timeout
        }

        release_mutex(&state_mutex, state);
        widget_update(widget);
    }
}