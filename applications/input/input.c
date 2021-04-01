#include "input_i.h"
#include <m-string.h>

#define GPIO_Read(input_pin)                                                    \
    (HAL_GPIO_ReadPin((GPIO_TypeDef*)input_pin.pin->port, input_pin.pin->pin) ^ \
     input_pin.pin->inverted)

static Input* input = NULL;

void input_press_timer_callback(void* arg) {
    InputPinState* input_pin = arg;
    InputEvent event;
    event.key = input_pin->pin->key;
    if(!input_pin->repeat_event) {
        event.type = InputTypeLong;
        notify_pubsub(&input->event_pubsub, &event);
        input_pin->repeat_event = true;
    } else {
        event.type = InputTypeRepeat;
        notify_pubsub(&input->event_pubsub, &event);
    }
    osTimerStart(input_pin->press_timer, INPUT_REPEATE_PRESS_TICKS);
}

void input_isr(void* _pin, void* _ctx) {
    osThreadFlagsSet(input->thread, INPUT_THREAD_FLAG_ISR);
}

void input_cli_send(string_t args, void* context) {
    InputEvent event;

    // Get first word as key name
    string_t key_name;
    string_init(key_name);
    size_t ws = string_search_char(args, ' ');
    if(ws == STRING_FAILURE) {
        printf("Wrong input");
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
        printf("Wrong argument");
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
        printf("Wrong argument");
        return;
    }
    // Publish input event
    notify_pubsub(&input->event_pubsub, &event);
}

int32_t input_task() {
    input = furi_alloc(sizeof(Input));
    input->thread = osThreadGetId();
    init_pubsub(&input->event_pubsub);
    furi_record_create("input_events", &input->event_pubsub);

    input->cli = furi_record_open("cli");
    if(input->cli) {
        cli_add_command(input->cli, "input_send", input_cli_send, input);
    }

    const size_t pin_count = input_pins_count;
    input->pin_states = furi_alloc(pin_count * sizeof(InputPinState));

    api_interrupt_add(input_isr, InterruptTypeExternalInterrupt, NULL);

    for(size_t i = 0; i < pin_count; i++) {
        input->pin_states[i].pin = &input_pins[i];
        input->pin_states[i].state = GPIO_Read(input->pin_states[i]);
        input->pin_states[i].repeat_event = false;
        input->pin_states[i].debounce = INPUT_DEBOUNCE_TICKS_HALF;
        input->pin_states[i].press_timer =
            osTimerNew(input_press_timer_callback, osTimerOnce, &input->pin_states[i], NULL);
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
                event.type = input->pin_states[i].state ? InputTypePress : InputTypeRelease;
                event.key = input->pin_states[i].pin->key;
                // Send Press/Release event
                notify_pubsub(&input->event_pubsub, &event);
                // Short/Long press logic
                if(state) {
                    osTimerStart(input->pin_states[i].press_timer, INPUT_LONG_PRESS_TICKS);
                } else if(input->pin_states[i].repeat_event) {
                    osTimerStop(input->pin_states[i].press_timer);
                    input->pin_states[i].repeat_event = false;
                } else if(osTimerStop(input->pin_states[i].press_timer) == osOK) {
                    event.type = InputTypeShort;
                    notify_pubsub(&input->event_pubsub, &event);
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
