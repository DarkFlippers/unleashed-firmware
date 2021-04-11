#include <furi.h>
#include <api-hal.h>
#include <gui/gui.h>
#include <stream_buffer.h>

typedef enum { EventTypeTick, EventTypeKey, EventTypeRx } EventType;

typedef struct {
    uint8_t dummy;
} RxEvent;

typedef struct {
    union {
        InputEvent input;
        RxEvent rx;
    } value;
    EventType type;
} AppEvent;

typedef struct {
    uint32_t freq_khz;
    bool on;
    uint8_t customer_id;
    uint32_t em_data;
    bool dirty;
    bool dirty_freq;
} State;

static void render_callback(Canvas* canvas, void* ctx) {
    State* state = (State*)acquire_mutex((ValueMutex*)ctx, 25);

    canvas_clear(canvas);

    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 12, "LF RFID");

    canvas_draw_str(canvas, 2, 24, state->on ? "Reading" : "Emulating");

    char buf[30];

    snprintf(buf, sizeof(buf), "%d kHz", (int)state->freq_khz);
    canvas_draw_str(canvas, 2, 36, buf);

    snprintf(buf, sizeof(buf), "%02d:%010ld", state->customer_id, state->em_data);
    canvas_draw_str(canvas, 2, 45, buf);

    release_mutex((ValueMutex*)ctx, state);
}

static void input_callback(InputEvent* input_event, void* ctx) {
    osMessageQueueId_t event_queue = ctx;

    if(input_event->type != InputTypeShort) return;

    AppEvent event;
    event.type = EventTypeKey;
    event.value.input = *input_event;
    osMessageQueuePut(event_queue, &event, 1, osWaitForever);
}

extern TIM_HandleTypeDef TIM_C;
void em4100_emulation(uint8_t* data, GpioPin* pin);
void prepare_data(uint32_t ID, uint32_t VENDOR, uint8_t* data);

GpioPin debug_0 = {.pin = GPIO_PIN_2, .port = GPIOB};
GpioPin debug_1 = {.pin = GPIO_PIN_3, .port = GPIOC};

extern COMP_HandleTypeDef hcomp1;

typedef struct {
    osMessageQueueId_t event_queue;
    uint32_t prev_dwt;
    int8_t symbol;
    bool center;
    size_t symbol_cnt;
    StreamBufferHandle_t stream_buffer;
    uint8_t* int_buffer;
} ComparatorCtx;

void init_comp_ctx(ComparatorCtx* ctx) {
    ctx->prev_dwt = 0;
    ctx->symbol = -1; // init state
    ctx->center = false;
    ctx->symbol_cnt = 0;
    xStreamBufferReset(ctx->stream_buffer);

    for(size_t i = 0; i < 64; i++) {
        ctx->int_buffer[i] = 0;
    }
}

void comparator_trigger_callback(void* hcomp, void* comp_ctx) {
    ComparatorCtx* ctx = (ComparatorCtx*)comp_ctx;

    uint32_t dt = (DWT->CYCCNT - ctx->prev_dwt) / (SystemCoreClock / 1000000.0f);
    ctx->prev_dwt = DWT->CYCCNT;

    if(dt < 150) return; // supress noise

    // wait message will be consumed
    if(xStreamBufferBytesAvailable(ctx->stream_buffer) == 64) return;

    gpio_write(&debug_0, true);

    // TOOD F4 and F5 differ
    bool rx_value = get_rfid_in_level();

    if(dt > 384) {
        // change symbol 0->1 or 1->0
        ctx->symbol = rx_value;
        ctx->center = true;
    } else {
        // same symbol as prev or center
        ctx->center = !ctx->center;
    }

    /*
    gpio_write(&debug_1, true);
    delay_us(center ? 10 : 30);
    gpio_write(&debug_1, false);
    */

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if(ctx->center && ctx->symbol != -1) {
        /*
        gpio_write(&debug_0, true);
        delay_us(symbol ? 10 : 30);
        gpio_write(&debug_0, false);
        */

        ctx->int_buffer[ctx->symbol_cnt] = ctx->symbol;
        ctx->symbol_cnt++;
    }

    // check preamble
    if(ctx->symbol_cnt <= 9 && ctx->symbol == 0) {
        ctx->symbol_cnt = 0;
        ctx->symbol = -1;
    }

    // check stop bit
    if(ctx->symbol_cnt == 64 && ctx->symbol == 1) {
        ctx->symbol_cnt = 0;
        ctx->symbol = -1;
    }

    // TODO
    // write only 9..64 symbols directly to streambuffer

    if(ctx->symbol_cnt == 64) {
        if(xStreamBufferSendFromISR(
               ctx->stream_buffer, ctx->int_buffer, 64, &xHigherPriorityTaskWoken) == 64) {
            AppEvent event;
            event.type = EventTypeRx;
            osMessageQueuePut(ctx->event_queue, &event, 0, 0);
        }

        ctx->symbol_cnt = 0;
    }

    gpio_write(&debug_0, false);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

const uint8_t ROW_SIZE = 4;
const uint8_t LINE_SIZE = 10;

static bool even_check(uint8_t* buf) {
    uint8_t col_parity_sum[ROW_SIZE];
    for(uint8_t col = 0; col < ROW_SIZE; col++) {
        col_parity_sum[col] = 0;
    }

    // line parity
    for(uint8_t line = 0; line < LINE_SIZE; line++) {
        printf("%d: ", line);
        uint8_t parity_sum = 0;
        for(uint8_t col = 0; col < ROW_SIZE; col++) {
            parity_sum += buf[line * (ROW_SIZE + 1) + col];
            col_parity_sum[col] += buf[line * (ROW_SIZE + 1) + col];
            printf("%d ", buf[line * (ROW_SIZE + 1) + col]);
        }
        if((1 & parity_sum) != buf[line * (ROW_SIZE + 1) + ROW_SIZE]) {
            printf(
                "line parity fail at %d (%d : %d)\n",
                line,
                parity_sum,
                buf[line * (ROW_SIZE + 1) + ROW_SIZE]);
            return false;
        }
        printf("\r\n");
    }

    for(uint8_t col = 0; col < ROW_SIZE; col++) {
        if((1 & col_parity_sum[col]) != buf[LINE_SIZE * (ROW_SIZE + 1) + col]) {
            printf(
                "col parity fail at %d (%d : %d)\n",
                col,
                col_parity_sum[col],
                buf[LINE_SIZE * (ROW_SIZE + 1) + col]);
            return false;
        }
    }

    return true;
}

static void extract_data(uint8_t* buf, uint8_t* customer, uint32_t* em_data) {
    uint32_t data = 0;
    uint8_t offset = 0;

    printf("customer: ");
    for(uint8_t line = 0; line < 2; line++) {
        for(uint8_t col = 0; col < ROW_SIZE; col++) {
            uint32_t bit = buf[line * (ROW_SIZE + 1) + col];

            data |= bit << (7 - offset);
            printf("%ld ", bit);

            offset++;
        }
    }
    printf("\r\n");

    *customer = data;

    data = 0;
    offset = 0;
    printf("data: ");
    for(uint8_t line = 2; line < LINE_SIZE; line++) {
        for(uint8_t col = 0; col < ROW_SIZE; col++) {
            uint32_t bit = buf[line * (ROW_SIZE + 1) + col];

            data |= bit << (31 - offset);
            printf("%ld ", bit);

            offset++;
        }
    }
    printf("\r\n");

    *em_data = data;
}

int32_t lf_rfid_workaround(void* p) {
    osMessageQueueId_t event_queue = osMessageQueueNew(8, sizeof(AppEvent), NULL);

    // create pin
    GpioPin pull_pin = {.pin = RFID_PULL_Pin, .port = RFID_PULL_GPIO_Port};
    // TODO open record
    GpioPin* pull_pin_record = &pull_pin;

    gpio_init(pull_pin_record, GpioModeOutputPushPull);

    gpio_init(&debug_0, GpioModeOutputPushPull);
    gpio_init(&debug_1, GpioModeOutputPushPull);

    // pulldown iBtn pin to prevent interference from ibutton
    gpio_init((GpioPin*)&ibutton_gpio, GpioModeOutputOpenDrain);
    gpio_write((GpioPin*)&ibutton_gpio, false);

    // init ctx
    ComparatorCtx comp_ctx;

    // internal buffer
    uint8_t int_bufer[64];

    comp_ctx.stream_buffer = xStreamBufferCreate(64, 64);
    comp_ctx.int_buffer = int_bufer;
    comp_ctx.event_queue = event_queue;
    init_comp_ctx(&comp_ctx);

    if(comp_ctx.stream_buffer == NULL) {
        printf("cannot create stream buffer\r\n");
        return 255;
    }

    // start comp
    HAL_COMP_Start(&hcomp1);

    uint8_t raw_data[64];
    for(size_t i = 0; i < 64; i++) {
        raw_data[i] = 0;
    }

    State _state;
    _state.freq_khz = 125;
    _state.on = false;
    _state.customer_id = 00;
    _state.em_data = 4378151;
    _state.dirty = true;
    _state.dirty_freq = true;

    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, &_state, sizeof(State))) {
        printf("cannot create mutex\r\n");
        return 255;
    }

    ViewPort* view_port = view_port_alloc();

    view_port_draw_callback_set(view_port, render_callback, &state_mutex);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    // Open GUI and register view_port
    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    AppEvent event;

    while(1) {
        osStatus_t event_status = osMessageQueueGet(event_queue, &event, NULL, 1024 / 8);

        if(event.type == EventTypeRx && event_status == osOK) {
            size_t received = xStreamBufferReceive(comp_ctx.stream_buffer, raw_data, 64, 0);
            printf("received: %d\r\n", received);
            if(received == 64) {
                if(even_check(&raw_data[9])) {
                    State* state = (State*)acquire_mutex_block(&state_mutex);
                    extract_data(&raw_data[9], &state->customer_id, &state->em_data);

                    printf("customer: %02d, data: %010lu\n", state->customer_id, state->em_data);

                    release_mutex(&state_mutex, state);

                    view_port_update(view_port);

                    api_hal_light_set(LightGreen, 0xFF);
                    osDelay(50);
                    api_hal_light_set(LightGreen, 0x00);
                }
            }
        } else {
            State* state = (State*)acquire_mutex_block(&state_mutex);

            if(event_status == osOK) {
                if(event.type == EventTypeKey) {
                    // press events
                    if(event.value.input.key == InputKeyBack) {
                        break;
                    }

                    if(event.value.input.key == InputKeyUp) {
                        state->dirty_freq = true;
                        state->freq_khz += 10;
                    }

                    if(event.value.input.key == InputKeyDown) {
                        state->dirty_freq = true;
                        state->freq_khz -= 10;
                    }

                    if(event.value.input.key == InputKeyLeft) {
                    }

                    if(event.value.input.key == InputKeyRight) {
                    }

                    if(event.value.input.key == InputKeyOk) {
                        state->dirty = true;
                        state->on = !state->on;
                    }
                }
            } else {
                // event timeout
            }

            if(state->dirty) {
                if(state->on) {
                    gpio_write(pull_pin_record, false);
                    init_comp_ctx(&comp_ctx);
                    api_interrupt_add(
                        comparator_trigger_callback, InterruptTypeComparatorTrigger, &comp_ctx);
                } else {
                    prepare_data(state->em_data, state->customer_id, raw_data);
                    api_interrupt_remove(
                        comparator_trigger_callback, InterruptTypeComparatorTrigger);
                }

                state->dirty_freq = true; // config new PWM next

                state->dirty = false;
            }

            if(state->dirty_freq) {
                hal_pwmn_set(
                    state->on ? 0.5 : 0.0, (float)(state->freq_khz * 1000), &LFRFID_TIM, LFRFID_CH);

                state->dirty_freq = false;
            }

            if(!state->on) {
                em4100_emulation(raw_data, pull_pin_record);
            }
            release_mutex(&state_mutex, state);
            view_port_update(view_port);
        }
    }

    hal_pwmn_stop(&TIM_C, TIM_CHANNEL_1); // TODO: move to furiac_onexit
    api_interrupt_remove(comparator_trigger_callback, InterruptTypeComparatorTrigger);

    gpio_init(pull_pin_record, GpioModeInput);
    gpio_init((GpioPin*)&ibutton_gpio, GpioModeInput);

    // TODO remove all view_ports create by app
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);

    HAL_COMP_Stop(&hcomp1);

    vStreamBufferDelete(comp_ctx.stream_buffer);

    osMessageQueueDelete(event_queue);

    return 0;
}
