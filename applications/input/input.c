#include "input_i.h"

#define GPIO_Read(input_pin)                                                    \
    (HAL_GPIO_ReadPin((GPIO_TypeDef*)input_pin.pin->port, input_pin.pin->pin) ^ \
     input_pin.pin->inverted)

static Input* input = NULL;

inline static void input_timer_start(osTimerId_t timer_id, uint32_t ticks) {
    TimerHandle_t hTimer = (TimerHandle_t)timer_id;
    furi_check(xTimerChangePeriod(hTimer, ticks, portMAX_DELAY) == pdPASS);
}

inline static void input_timer_stop(osTimerId_t timer_id) {
    TimerHandle_t hTimer = (TimerHandle_t)timer_id;
    furi_check(xTimerStop(hTimer, portMAX_DELAY) == pdPASS);
    // xTimerStop is not actually stopping timer,
    // Instead it places stop event into timer queue
    // This code ensures that timer is stopped
    while(xTimerIsTimerActive(hTimer) == pdTRUE) osDelay(1);
}

void input_press_timer_callback(void* arg) {
    InputPinState* input_pin = arg;
    InputEvent event;
    event.key = input_pin->pin->key;
    input_pin->press_counter++;
    if(input_pin->press_counter == INPUT_LONG_PRESS_COUNTS) {
        event.type = InputTypeLong;
        notify_pubsub(&input->event_pubsub, &event);
    } else if(input_pin->press_counter > INPUT_LONG_PRESS_COUNTS) {
        input_pin->press_counter--;
        event.type = InputTypeRepeat;
        notify_pubsub(&input->event_pubsub, &event);
    }
}

void input_isr(void* _ctx) {
    osThreadFlagsSet(input->thread, INPUT_THREAD_FLAG_ISR);
}

void input_cli_send(Cli* cli, string_t args, void* context) {
    InputEvent event;

    // Get first word as key name
    string_t key_name;
    string_init(key_name);
    size_t ws = string_search_char(args, ' ');
    if(ws == STRING_FAILURE) {
        printf("Invalid arguments. Use `input_send KEY TYPE`.");
        string_clear(key_name);
        return;
    } else {
        string_set_n(key_name, args, 0, ws);
        string_right(args, ws);
        string_strim(args);
    }
    // Check key name and set event key
    if(!string_cmp(key_name, "up")) {
        event.key = InputKeyUp;
    } else if(!string_cmp(key_name, "down")) {
        event.key = InputKeyDown;
    } else if(!string_cmp(key_name, "left")) {
        event.key = InputKeyLeft;
    } else if(!string_cmp(key_name, "right")) {
        event.key = InputKeyRight;
    } else if(!string_cmp(key_name, "ok")) {
        event.key = InputKeyOk;
    } else if(!string_cmp(key_name, "back")) {
        event.key = InputKeyBack;
    } else {
        printf("Invalid key name. Valid keys: `up`, `down`, `left`, `right`, `back`, `ok`.");
        string_clear(key_name);
        return;
    }
    string_clear(key_name);
    // Check the rest of args string and set event type
    if(!string_cmp(args, "press")) {
        event.type = InputTypePress;
    } else if(!string_cmp(args, "release")) {
        event.type = InputTypeRelease;
    } else if(!string_cmp(args, "short")) {
        event.type = InputTypeShort;
    } else if(!string_cmp(args, "long")) {
        event.type = InputTypeLong;
    } else {
        printf("Ivalid type. Valid types: `press`, `release`, `short`, `long`.");
        return;
    }
    // Publish input event
    notify_pubsub(&input->event_pubsub, &event);
}

int32_t input_srv() {
    input = furi_alloc(sizeof(Input));
    input->thread = osThreadGetId();
    init_pubsub(&input->event_pubsub);
    furi_record_create("input_events", &input->event_pubsub);

    input->cli = furi_record_open("cli");
    if(input->cli) {
        cli_add_command(
            input->cli, "input_send", CliCommandFlagParallelSafe, input_cli_send, input);
    }

    const size_t pin_count = input_pins_count;
    input->pin_states = furi_alloc(pin_count * sizeof(InputPinState));

    for(size_t i = 0; i < pin_count; i++) {
        GpioPin gpio = {(GPIO_TypeDef*)input_pins[i].port, (uint16_t)input_pins[i].pin};
        hal_gpio_add_int_callback(&gpio, input_isr, NULL);
        input->pin_states[i].pin = &input_pins[i];
        input->pin_states[i].state = GPIO_Read(input->pin_states[i]);
        input->pin_states[i].debounce = INPUT_DEBOUNCE_TICKS_HALF;
        input->pin_states[i].press_timer =
            osTimerNew(input_press_timer_callback, osTimerPeriodic, &input->pin_states[i], NULL);
        input->pin_states[i].press_counter = 0;
    }

    while(1) {
        bool is_changing = false;
        for(size_t i = 0; i < pin_count; i++) {
            bool state = GPIO_Read(input->pin_states[i]);
            if(input->pin_states[i].debounce > 0 &&
               input->pin_states[i].debounce < INPUT_DEBOUNCE_TICKS) {
                is_changing = true;
                input->pin_states[i].debounce += (state ? 1 : -1);
            } else if(input->pin_states[i].state != state) {
                input->pin_states[i].state = state;

                // Common state info
                InputEvent event;
                event.key = input->pin_states[i].pin->key;
                event.type = input->pin_states[i].state ? InputTypePress : InputTypeRelease;
                // Send Press/Release event
                notify_pubsub(&input->event_pubsub, &event);

                // Short / Long / Repeat timer routine
                if(state) {
                    input_timer_start(input->pin_states[i].press_timer, INPUT_PRESS_TICKS);
                } else {
                    input_timer_stop(input->pin_states[i].press_timer);
                    if(input->pin_states[i].press_counter < INPUT_LONG_PRESS_COUNTS) {
                        event.type = InputTypeShort;
                        notify_pubsub(&input->event_pubsub, &event);
                    }
                    input->pin_states[i].press_counter = 0;
                }
            }
        }

        if(is_changing) {
            osDelay(1);
        } else {
            osThreadFlagsWait(INPUT_THREAD_FLAG_ISR, osFlagsWaitAny, osWaitForever);
        }
    }

    return 0;
}
