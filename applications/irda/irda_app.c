#include <furi.h>
#include <api-hal.h>
#include <gui/gui.h>
#include <input/input.h>
#include <cli/cli.h>

#include <api-hal-irda.h>
#include "irda.h"

typedef enum {
    EventTypeTick,
    EventTypeKey,
    EventTypeRX,
} EventType;

typedef IrdaMessage IrDAPacket;

typedef struct {
    union {
        InputEvent input;
        IrDAPacket rx;
    } value;
    EventType type;
} AppEvent;

//typedef struct {
//    IrdaProtocol protocol;
//    uint32_t address;
//    uint32_t command;
//} IrDAPacket;

#define IRDA_PACKET_COUNT 8

typedef struct {
    osMessageQueueId_t cli_ir_rx_queue;
    Cli* cli;
    bool cli_cmd_is_active;
} IrDAApp;

typedef struct {
    uint8_t mode_id;
    uint16_t carrier_freq;
    uint8_t carrier_duty_cycle_id;

    uint8_t packet_id;
    IrDAPacket packets[IRDA_PACKET_COUNT];
} State;

typedef void (*ModeInput)(AppEvent*, State*);
typedef void (*ModeRender)(Canvas*, State*);

void input_carrier(AppEvent* event, State* state);
void render_carrier(Canvas* canvas, State* state);
void input_packet(AppEvent* event, State* state);
void render_packet(Canvas* canvas, State* state);

typedef struct {
    ModeRender render;
    ModeInput input;
} Mode;

const Mode modes[] = {
    {.render = render_carrier, .input = input_carrier},
    {.render = render_packet, .input = input_packet},
};

const float duty_cycles[] = {0.1, 0.25, 0.333, 0.5, 1.0};

void render_carrier(Canvas* canvas, State* state) {
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 25, "carrier mode >");
    canvas_draw_str(canvas, 2, 37, "? /\\ freq | \\/ duty cycle");
    {
        char buf[24];
        sprintf(buf, "frequency: %u Hz", state->carrier_freq);
        canvas_draw_str(canvas, 2, 50, buf);
        sprintf(
            buf, "duty cycle: %d/1000", (int)(duty_cycles[state->carrier_duty_cycle_id] * 1000));
        canvas_draw_str(canvas, 2, 62, buf);
    }
}

void input_carrier(AppEvent* event, State* state) {
    if(event->value.input.key == InputKeyOk) {
        if(event->value.input.type == InputTypePress) {
            api_hal_irda_pwm_set(duty_cycles[state->carrier_duty_cycle_id], state->carrier_freq);
        } else if(event->value.input.type == InputTypeRelease) {
            api_hal_irda_pwm_stop();
        }
    }

    if(event->value.input.type == InputTypeShort && event->value.input.key == InputKeyUp) {
        if(state->carrier_freq < 45000) {
            state->carrier_freq += 1000;
        } else {
            state->carrier_freq = 33000;
        }
    }

    if(event->value.input.type == InputTypeShort && event->value.input.key == InputKeyDown) {
        uint8_t duty_cycles_count = sizeof(duty_cycles) / sizeof(duty_cycles[0]);
        if(state->carrier_duty_cycle_id < (duty_cycles_count - 1)) {
            state->carrier_duty_cycle_id++;
        } else {
            state->carrier_duty_cycle_id = 0;
        }
    }
}

void render_packet(Canvas* canvas, State* state) {
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 25, "< packet mode");
    canvas_draw_str(canvas, 2, 37, "? /\\ \\/ packet");
    {
        char buf[30];
        sprintf(
            buf,
            "P[%d]: %s 0x%lX 0x%lX",
            state->packet_id,
            irda_get_protocol_name(state->packets[state->packet_id].protocol),
            state->packets[state->packet_id].address,
            state->packets[state->packet_id].command);
        canvas_draw_str(canvas, 2, 50, buf);
    }
}

void input_packet(AppEvent* event, State* state) {
    if(event->value.input.key == InputKeyOk) {
        if(event->value.input.type == InputTypeShort) {
            IrdaMessage message = {
                .protocol = state->packets[state->packet_id].protocol,
                .address = state->packets[state->packet_id].address,
                .command = state->packets[state->packet_id].command,
            };
            irda_send(&message, 1);
        }
    }

    if(event->value.input.type == InputTypeShort && event->value.input.key == InputKeyDown) {
        if(state->packet_id < (IRDA_PACKET_COUNT - 1)) {
            state->packet_id++;
        };
    }

    if(event->value.input.type == InputTypeShort && event->value.input.key == InputKeyUp) {
        if(state->packet_id > 0) {
            state->packet_id--;
        };
    }
}

static void render_callback(Canvas* canvas, void* ctx) {
    State* state = (State*)acquire_mutex((ValueMutex*)ctx, 25);

    if(state != NULL) {
        canvas_clear(canvas);
        canvas_set_color(canvas, ColorBlack);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 2, 12, "irda test");

        modes[state->mode_id].render(canvas, state);

        release_mutex((ValueMutex*)ctx, state);
    }
}

static void input_callback(InputEvent* input_event, void* ctx) {
    osMessageQueueId_t event_queue = ctx;

    AppEvent event;
    event.type = EventTypeKey;
    event.value.input = *input_event;
    osMessageQueuePut(event_queue, &event, 0, 0);
}

void init_packet(
    State* state,
    uint8_t index,
    IrdaProtocol protocol,
    uint32_t address,
    uint32_t command) {
    if(index >= IRDA_PACKET_COUNT) return;
    state->packets[index].protocol = protocol;
    state->packets[index].address = address;
    state->packets[index].command = command;
}

void irda_cli_cmd_rx(Cli* cli, string_t args, void* context) {
    furi_assert(context);
    IrDAPacket packet;
    IrDAApp* app = context;
    app->cli_cmd_is_active = true;
    bool exit = false;

    printf("Reading income packets...\r\nPress Ctrl+C to abort\r\n");
    while(!exit) {
        exit = cli_cmd_interrupt_received(app->cli);
        osStatus status = osMessageQueueGet(app->cli_ir_rx_queue, &packet, 0, 5);
        if(status == osOK) {
            printf(
                "%s "
                "Address:0x%02X Command: 0x%02X %s\r\n",
                irda_get_protocol_name(packet.protocol),
                (uint8_t)packet.address,
                (uint8_t)packet.command,
                packet.repeat ? "R" : "");
        }
    }
    printf("Interrupt command received");
    app->cli_cmd_is_active = false;
    return;
}

void irda_cli_cmd_tx(Cli* cli, string_t args, void* context) {
    furi_assert(context);
    ValueMutex* state_mutex = context;
    // Read protocol name
    IrdaProtocol protocol;
    string_t protocol_str;
    string_init(protocol_str);
    size_t ws = string_search_char(args, ' ');
    if(ws == STRING_FAILURE) {
        printf("Invalid input. Use ir_tx PROTOCOL ADDRESS COMMAND");
        string_clear(protocol_str);
        return;
    } else {
        string_set_n(protocol_str, args, 0, ws);
        string_right(args, ws);
        string_strim(args);
    }
    if(!string_cmp_str(protocol_str, "NEC")) {
        protocol = IrdaProtocolNEC;
    } else if(!string_cmp_str(protocol_str, "SAMSUNG")) {
        protocol = IrdaProtocolSamsung32;
    } else {
        printf("Incorrect protocol. Valid protocols: `NEC`, `SAMSUNG`");
        string_clear(protocol_str);
        return;
    }
    string_clear(protocol_str);
    // Read address
    uint16_t address = strtoul(string_get_cstr(args), NULL, 16);
    ws = string_search_char(args, ' ');
    if(!(ws == 4 || ws == 6)) {
        printf("Invalid address format. Use 4 [0-F] hex digits in 0xXXXX or XXXX formats");
        return;
    }
    string_right(args, ws);
    string_strim(args);
    // Read command
    uint16_t command = strtoul(string_get_cstr(args), NULL, 16);
    ws = string_search_char(args, '\0');
    if(!(ws == 4 || ws == 6)) {
        printf("Invalid command format. Use 4 [0-F] hex digits in 0xXXXX or XXXX formats");
        return;
    }

    State* state = (State*)acquire_mutex(state_mutex, 25);
    if(state == NULL) {
        printf("IRDA resources busy\r\n");
        return;
    }

    IrdaMessage message = {
        .protocol = protocol,
        .address = address,
        .command = command,
    };
    irda_send(&message, 1);
    release_mutex(state_mutex, state);
    return;
}

typedef struct {
    osMessageQueueId_t event_queue;
    IrdaHandler* handler;
} IsrContext;

void irda_rx_callback(void* ctx, bool level, uint32_t duration) {
    IsrContext* isr_context = ctx;
    const IrdaMessage* message = irda_decode(isr_context->handler, level, duration);
    AppEvent event;
    event.type = EventTypeRX;

    if(message) {
        event.value.rx = *message;
        furi_assert(osOK == osMessageQueuePut(isr_context->event_queue, &event, 0, 0));
    }
}

int32_t irda(void* p) {
    osMessageQueueId_t event_queue = osMessageQueueNew(32, sizeof(AppEvent), NULL);

    State _state;
    uint8_t mode_count = sizeof(modes) / sizeof(modes[0]);
    uint8_t duty_cycles_count = sizeof(duty_cycles) / sizeof(duty_cycles[0]);

    _state.carrier_duty_cycle_id = duty_cycles_count - 2;
    _state.carrier_freq = 36000;
    _state.mode_id = 0;
    _state.packet_id = 0;

    IrDAApp irda_app;
    irda_app.cli = furi_record_open("cli");
    irda_app.cli_ir_rx_queue = osMessageQueueNew(1, sizeof(IrDAPacket), NULL);
    irda_app.cli_cmd_is_active = false;

    for(uint8_t i = 0; i < IRDA_PACKET_COUNT; i++) {
        init_packet(&_state, i, 0, 0, 0);
    }

    init_packet(&_state, 0, IrdaProtocolNEC, 0x00, 0x11);
    init_packet(&_state, 1, IrdaProtocolNEC, 0x08, 0x59);
    init_packet(&_state, 2, IrdaProtocolNEC, 0x00, 0x10);
    init_packet(&_state, 3, IrdaProtocolNEC, 0x00, 0x15);
    init_packet(&_state, 4, IrdaProtocolNEC, 0x00, 0x25);
    init_packet(&_state, 5, IrdaProtocolSamsung32, 0x0E, 0x0C);
    init_packet(&_state, 6, IrdaProtocolSamsung32, 0x0E, 0x0D);
    init_packet(&_state, 7, IrdaProtocolSamsung32, 0x0E, 0x0E);

    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, &_state, sizeof(State))) {
        printf("cannot create mutex\r\n");
        return 255;
    }

    ViewPort* view_port = view_port_alloc();

    view_port_draw_callback_set(view_port, render_callback, &state_mutex);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    cli_add_command(irda_app.cli, "ir_rx", irda_cli_cmd_rx, &irda_app);
    cli_add_command(irda_app.cli, "ir_tx", irda_cli_cmd_tx, &state_mutex);

    // Open GUI and register view_port
    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    IsrContext isr_context = {
        .handler = irda_alloc_decoder(),
        .event_queue = event_queue,
    };
    api_hal_irda_rx_irq_init();
    api_hal_irda_rx_irq_set_callback(irda_rx_callback, &isr_context);

    AppEvent event;
    while(1) {
        osStatus_t event_status = osMessageQueueGet(event_queue, &event, NULL, 500);

        if(event_status == osOK) {
            if(event.type == EventTypeKey) {
                State* state = (State*)acquire_mutex_block(&state_mutex);

                // press events
                if(event.value.input.type == InputTypeShort &&
                   event.value.input.key == InputKeyBack) {
                    release_mutex(&state_mutex, state);

                    // remove all view_ports create by app
                    gui_remove_view_port(gui, view_port);
                    view_port_free(view_port);

                    // free decoder
                    delete_mutex(&state_mutex);
                    osMessageQueueDelete(event_queue);
                    osMessageQueueDelete(irda_app.cli_ir_rx_queue);
                    cli_delete_command(irda_app.cli, "ir_rx");
                    cli_delete_command(irda_app.cli, "ir_tx");
                    furi_record_close("cli");
                    api_hal_irda_rx_irq_deinit();
                    irda_free_decoder(isr_context.handler);

                    // exit
                    return 0;
                }

                if(event.value.input.type == InputTypeShort &&
                   event.value.input.key == InputKeyLeft) {
                    if(state->mode_id > 0) {
                        state->mode_id--;
                    }
                }

                if(event.value.input.type == InputTypeShort &&
                   event.value.input.key == InputKeyRight) {
                    if(state->mode_id < (mode_count - 1)) {
                        state->mode_id++;
                    }
                }

                modes[state->mode_id].input(&event, state);

                release_mutex(&state_mutex, state);
                view_port_update(view_port);

            } else if(event.type == EventTypeRX) {
                api_hal_light_set(LightRed, 0xFF);
                delay(60);
                api_hal_light_set(LightRed, 0xFF);

                // save only if we in packet mode
                State* state = (State*)acquire_mutex_block(&state_mutex);
                IrDAPacket packet = event.value.rx;

                if(state->mode_id == 1) {
                    printf("P=%s ", irda_get_protocol_name(packet.protocol));
                    printf("A=0x%02lX ", packet.address);
                    printf("C=0x%02lX ", packet.command);
                    if(packet.repeat) {
                        printf("R");
                    }
                    printf("\r\n");
                    // Save packet to state
                    memcpy(&(state->packets[state->packet_id]), &packet, sizeof(IrDAPacket));
                }
                if(irda_app.cli_cmd_is_active) {
                    // Send decoded packet to cli
                    osMessageQueuePut(irda_app.cli_ir_rx_queue, &packet, 0, 0);
                }

                release_mutex(&state_mutex, state);
                view_port_update(view_port);

                // blink anyway
                api_hal_light_set(LightGreen, 0xFF);
                api_hal_light_set(LightGreen, 0x00);
            }

        } else {
            // event timeout
        }
    }
}
