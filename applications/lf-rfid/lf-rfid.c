#include <furi.h>
#include <gui/gui.h>

typedef enum { EventTypeTick, EventTypeKey, EventTypeRx } EventType;

typedef struct {
    bool value;
    uint32_t dwt_value;
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
} State;

static void render_callback(Canvas* canvas, void* ctx) {
    State* state = (State*)acquire_mutex((ValueMutex*)ctx, 25);

    canvas_clear(canvas);

    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 12, "LF RFID");

    canvas_draw_str(canvas, 2, 24, state->on ? "Reading" : "Emulating");

    char buf[14];

    sprintf(buf, "%d kHz", (int)state->freq_khz);
    canvas_draw_str(canvas, 2, 36, buf);

    sprintf(buf, "%02d:%010ld", state->customer_id, state->em_data);
    canvas_draw_str(canvas, 2, 45, buf);

    release_mutex((ValueMutex*)ctx, state);
}

static void input_callback(InputEvent* input_event, void* ctx) {
    osMessageQueueId_t event_queue = ctx;

    AppEvent event;
    event.type = EventTypeKey;
    event.value.input = *input_event;
    osMessageQueuePut(event_queue, &event, 0, 0);
}

extern TIM_HandleTypeDef TIM_C;
void em4100_emulation(uint8_t* data, GpioPin* pin);
void prepare_data(uint32_t ID, uint32_t VENDOR, uint8_t* data);

GpioPin debug_0 = {.pin = GPIO_PIN_2, .port = GPIOB};
GpioPin debug_1 = {.pin = GPIO_PIN_3, .port = GPIOC};

extern COMP_HandleTypeDef hcomp1;

void comparator_trigger_callback(void* hcomp, void* comp_ctx) {
    if((COMP_HandleTypeDef*)hcomp != &hcomp1) return;

    // gpio_write(&debug_0, true);

    osMessageQueueId_t event_queue = comp_ctx;

    AppEvent event;
    event.type = EventTypeRx;
    event.value.rx.value = (HAL_COMP_GetOutputLevel(hcomp) == COMP_OUTPUT_LEVEL_HIGH);
    event.value.rx.dwt_value = DWT->CYCCNT;
    osMessageQueuePut(event_queue, &event, 0, 0);

    // gpio_write(&debug_0, false);
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
        printf("\n");
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
            printf("%d ", bit);

            offset++;
        }
    }
    printf("\n");

    *customer = data;

    data = 0;
    offset = 0;
    printf("data: ");
    for(uint8_t line = 2; line < LINE_SIZE; line++) {
        for(uint8_t col = 0; col < ROW_SIZE; col++) {
            uint32_t bit = buf[line * (ROW_SIZE + 1) + col];

            data |= bit << (31 - offset);
            printf("%d ", bit);

            offset++;
        }
    }
    printf("\n");

    *em_data = data;
}

void lf_rfid_workaround(void* p) {
    osMessageQueueId_t event_queue = osMessageQueueNew(1, sizeof(AppEvent), NULL);

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
    void* comp_ctx = (void*)event_queue;
    api_interrupt_add(comparator_trigger_callback, InterruptTypeComparatorTrigger, comp_ctx);

    // start comp
    HAL_COMP_Start(&hcomp1);

    uint8_t emulation_data[64];

    State _state;
    _state.freq_khz = 125;
    _state.on = false;
    _state.customer_id = 01;
    _state.em_data = 4378151;

    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, &_state, sizeof(State))) {
        printf("cannot create mutex\n");
        furiac_exit(NULL);
    }

    Widget* widget = widget_alloc();

    widget_draw_callback_set(widget, render_callback, &state_mutex);
    widget_input_callback_set(widget, input_callback, event_queue);

    // Open GUI and register widget
    Gui* gui = furi_record_open("gui");
    gui_add_widget(gui, widget, GuiLayerFullscreen);

    AppEvent event;
    uint32_t prev_dwt;
    int8_t symbol = -1; // init state
    bool center = false;
    size_t symbol_cnt = 0;

    GpioPin* led_record = (GpioPin*)&led_gpio[1];
    gpio_init(led_record, GpioModeOutputOpenDrain);

    uint8_t buf[64];
    for(size_t i = 0; i < 64; i++) {
        buf[i] = 0;
    }

    while(1) {
        osStatus_t event_status = osMessageQueueGet(event_queue, &event, NULL, 100);

        if(event.type == EventTypeRx && event_status == osOK) {
            uint32_t dt = (event.value.rx.dwt_value - prev_dwt) / (SystemCoreClock / 1000000.0f);
            prev_dwt = event.value.rx.dwt_value;

            if(dt > 384) {
                // change symbol 0->1 or 1->0
                symbol = event.value.rx.value;
                center = true;
            } else {
                // same symbol as prev or center
                center = !center;
            }

            /*
            gpio_write(&debug_1, true);
            delay_us(center ? 10 : 30);
            gpio_write(&debug_1, false);
            */

            if(center && symbol != -1) {
                /*
                gpio_write(&debug_0, true);
                delay_us(symbol ? 10 : 30);
                gpio_write(&debug_0, false);
                */

                buf[symbol_cnt] = symbol;
                symbol_cnt++;
            }

            // check preamble
            if(symbol_cnt <= 9 && symbol == 0) {
                symbol_cnt = 0;
                symbol = -1;
            }

            // check stop bit
            if(symbol_cnt == 64 && symbol == 1) {
                symbol_cnt = 0;
                symbol = -1;
            }

            if(symbol_cnt == 64) {
                if(even_check(&buf[9])) {
                    State* state = (State*)acquire_mutex_block(&state_mutex);
                    extract_data(&buf[9], &state->customer_id, &state->em_data);
                    printf("customer: %02d, data: %010lu\n", state->customer_id, state->em_data);
                    release_mutex(&state_mutex, state);
                    gpio_write(led_record, false);
                    osDelay(100);
                    gpio_write(led_record, true);
                }

                symbol_cnt = 0;
            }
        } else {
            State* state = (State*)acquire_mutex_block(&state_mutex);

            if(event_status == osOK) {
                if(event.type == EventTypeKey) {
                    // press events
                    if(event.value.input.state && event.value.input.input == InputBack) {
                        hal_pwmn_stop(&TIM_C, TIM_CHANNEL_1); // TODO: move to furiac_onexit
                        gpio_init(pull_pin_record, GpioModeInput);
                        gpio_init((GpioPin*)&ibutton_gpio, GpioModeInput);

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

                        if(!state->on) {
                            prepare_data(state->em_data, state->customer_id, emulation_data);
                        }
                    }
                }
            } else {
                // event timeout
            }

            hal_pwmn_set(
                state->on ? 0.5 : 0.0, (float)(state->freq_khz * 1000), &LFRFID_TIM, LFRFID_CH);

            if(!state->on) {
                em4100_emulation(emulation_data, pull_pin_record);
            } else {
                gpio_write(pull_pin_record, false);
            }

            // common code, for example, force update UI
            widget_update(widget);

            release_mutex(&state_mutex, state);
        }
    }
}
