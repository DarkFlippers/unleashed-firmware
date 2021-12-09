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
    event.sequence = input_pin->counter;
    event.key = input_pin->pin->key;
    input_pin->press_counter++;
    if(input_pin->press_counter == INPUT_LONG_PRESS_COUNTS) {
        event.type = InputTypeLong;
        furi_pubsub_publish(input->event_pubsub, &event);
    } else if(input_pin->press_counter > INPUT_LONG_PRESS_COUNTS) {
        input_pin->press_counter--;
        event.type = InputTypeRepeat;
        furi_pubsub_publish(input->event_pubsub, &event);
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
    furi_pubsub_publish(input->event_pubsub, &event);
}

static void input_cli_dump_events_callback(const void* value, void* ctx) {
    furi_assert(value);
    furi_assert(ctx);
    osMessageQueueId_t input_queue = ctx;
    osMessageQueuePut(input_queue, value, 0, osWaitForever);
}

static void input_cli_dump(Cli* cli, string_t args, void* context) {
    osMessageQueueId_t input_queue = osMessageQueueNew(8, sizeof(InputEvent), NULL);
    FuriPubSubSubscription* input_subscription =
        furi_pubsub_subscribe(input->event_pubsub, input_cli_dump_events_callback, input_queue);

    bool stop = false;
    InputEvent input_event;
    while(!stop) {
        if(osMessageQueueGet(input_queue, &input_event, NULL, 100) == osOK) {
            printf(
                "key: %s type: %s\r\n",
                input_get_key_name(input_event.key),
                input_get_type_name(input_event.type));
        }

        if(cli_cmd_interrupt_received(cli)) {
            stop = true;
        }
    }

    furi_pubsub_unsubscribe(input->event_pubsub, input_subscription);
    osMessageQueueDelete(input_queue);
}

const char* input_get_key_name(InputKey key) {
    for(size_t i = 0; i < input_pins_count; i++) {
        if(input_pins[i].key == key) {
            return input_pins[i].name;
        }
    }
    return "Unknown";
}

const char* input_get_type_name(InputType type) {
    switch(type) {
    case InputTypePress:
        return "Press";
    case InputTypeRelease:
        return "Release";
    case InputTypeShort:
        return "Short";
    case InputTypeLong:
        return "Long";
    case InputTypeRepeat:
        return "Repeat";
    }
    return "Unknown";
}

int32_t input_srv() {
    input = furi_alloc(sizeof(Input));
    input->thread = osThreadGetId();
    input->event_pubsub = furi_pubsub_alloc();
    furi_record_create("input_events", input->event_pubsub);

    input->cli = furi_record_open("cli");
    if(input->cli) {
        cli_add_command(
            input->cli, "input_send", CliCommandFlagParallelSafe, input_cli_send, NULL);
        cli_add_command(
            input->cli, "input_dump", CliCommandFlagParallelSafe, input_cli_dump, NULL);
    }

    input->pin_states = furi_alloc(input_pins_count * sizeof(InputPinState));

    for(size_t i = 0; i < input_pins_count; i++) {
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
        for(size_t i = 0; i < input_pins_count; i++) {
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

                // Short / Long / Repeat timer routine
                if(state) {
                    input->counter++;
                    input->pin_states[i].counter = input->counter;
                    event.sequence = input->pin_states[i].counter;
                    input_timer_start(input->pin_states[i].press_timer, INPUT_PRESS_TICKS);
                } else {
                    event.sequence = input->pin_states[i].counter;
                    input_timer_stop(input->pin_states[i].press_timer);
                    if(input->pin_states[i].press_counter < INPUT_LONG_PRESS_COUNTS) {
                        event.type = InputTypeShort;
                        furi_pubsub_publish(input->event_pubsub, &event);
                    }
                    input->pin_states[i].press_counter = 0;
                }

                // Send Press/Release event
                event.type = input->pin_states[i].state ? InputTypePress : InputTypeRelease;
                furi_pubsub_publish(input->event_pubsub, &event);
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
