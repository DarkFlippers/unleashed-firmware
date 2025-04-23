#include "input.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <furi.h>
#include <furi_hal_gpio.h>
#include <furi_hal_vibro.h>
#include <toolbox/cli/cli_command.h>
#include <cli/cli_main_commands.h>
#include <toolbox/pipe.h>

#define INPUT_DEBOUNCE_TICKS_HALF (INPUT_DEBOUNCE_TICKS / 2)
#define INPUT_PRESS_TICKS         150
#define INPUT_LONG_PRESS_COUNTS   2
#define INPUT_THREAD_FLAG_ISR     0x00000001

#define TAG "Input"

/** Input pin state */
typedef struct {
    const InputPin* pin;
    // State
    volatile bool state;
    volatile uint8_t debounce;
    FuriTimer* press_timer;
    FuriPubSub* event_pubsub;
    volatile uint8_t press_counter;
    volatile uint32_t counter;
} InputPinState;

/** Input CLI command handler */
void input_cli(PipeSide* pipe, FuriString* args, void* context);

// #define INPUT_DEBUG

#define GPIO_Read(input_pin) (furi_hal_gpio_read(input_pin.pin->gpio) ^ (input_pin.pin->inverted))

void input_press_timer_callback(void* arg) {
    InputPinState* input_pin = arg;
    InputEvent event;
    event.sequence_source = INPUT_SEQUENCE_SOURCE_HARDWARE;
    event.sequence_counter = input_pin->counter;
    event.key = input_pin->pin->key;
    input_pin->press_counter++;
    if(input_pin->press_counter == INPUT_LONG_PRESS_COUNTS) {
        event.type = InputTypeLong;
        furi_pubsub_publish(input_pin->event_pubsub, &event);
    } else if(input_pin->press_counter > INPUT_LONG_PRESS_COUNTS) {
        input_pin->press_counter--;
        event.type = InputTypeRepeat;
        furi_pubsub_publish(input_pin->event_pubsub, &event);
    }
}

void input_isr(void* _ctx) {
    FuriThreadId thread_id = (FuriThreadId)_ctx;
    furi_thread_flags_set(thread_id, INPUT_THREAD_FLAG_ISR);
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
    default:
        return "Unknown";
    }
}

int32_t input_srv(void* p) {
    UNUSED(p);

    const FuriThreadId thread_id = furi_thread_get_current_id();
    FuriPubSub* event_pubsub = furi_pubsub_alloc();
    uint32_t counter = 1;
    furi_record_create(RECORD_INPUT_EVENTS, event_pubsub);

    //define object input_settings, take memory load (or init) settings and create record for access to settings structure from outside
    InputSettings* settings = malloc(sizeof(InputSettings));
    furi_record_create(RECORD_INPUT_SETTINGS, settings);
    input_settings_load(settings);

#ifdef INPUT_DEBUG
    furi_hal_gpio_init_simple(&gpio_ext_pa4, GpioModeOutputPushPull);
#endif

#ifdef SRV_CLI
    CliRegistry* registry = furi_record_open(RECORD_CLI);
    cli_registry_add_command(
        registry, "input", CliCommandFlagParallelSafe, input_cli, event_pubsub);
    furi_record_close(RECORD_CLI);
#endif

    InputPinState pin_states[input_pins_count];

    for(size_t i = 0; i < input_pins_count; i++) {
        furi_hal_gpio_add_int_callback(input_pins[i].gpio, input_isr, thread_id);
        pin_states[i].pin = &input_pins[i];
        pin_states[i].state = GPIO_Read(pin_states[i]);
        pin_states[i].debounce = INPUT_DEBOUNCE_TICKS_HALF;
        pin_states[i].press_timer =
            furi_timer_alloc(input_press_timer_callback, FuriTimerTypePeriodic, &pin_states[i]);
        pin_states[i].event_pubsub = event_pubsub;
        pin_states[i].press_counter = 0;
    }

    while(1) {
        bool is_changing = false;
        for(size_t i = 0; i < input_pins_count; i++) {
            bool state = GPIO_Read(pin_states[i]);
            if(state) {
                if(pin_states[i].debounce < INPUT_DEBOUNCE_TICKS) pin_states[i].debounce += 1;
            } else {
                if(pin_states[i].debounce > 0) pin_states[i].debounce -= 1;
            }

            if(pin_states[i].debounce > 0 && pin_states[i].debounce < INPUT_DEBOUNCE_TICKS) {
                is_changing = true;
            } else if(pin_states[i].state != state) {
                pin_states[i].state = state;

                // Common state info
                InputEvent event;
                event.sequence_source = INPUT_SEQUENCE_SOURCE_HARDWARE;
                event.key = pin_states[i].pin->key;

                // Short / Long / Repeat timer routine
                if(state) {
                    pin_states[i].counter = counter++;
                    event.sequence_counter = pin_states[i].counter;
                    furi_timer_start(pin_states[i].press_timer, INPUT_PRESS_TICKS);
                } else {
                    event.sequence_counter = pin_states[i].counter;
                    furi_timer_stop(pin_states[i].press_timer);
                    while(furi_timer_is_running(pin_states[i].press_timer))
                        furi_delay_tick(1);
                    if(pin_states[i].press_counter < INPUT_LONG_PRESS_COUNTS) {
                        event.type = InputTypeShort;
                        furi_pubsub_publish(event_pubsub, &event);
                    }
                    pin_states[i].press_counter = 0;
                }

                // Send Press/Release event
                event.type = pin_states[i].state ? InputTypePress : InputTypeRelease;
                furi_pubsub_publish(event_pubsub, &event);
                // vibro signal if user setup vibro touch level in Settings-Input.
                if(settings->vibro_touch_level) {
                    //delay 1 ticks for compatibility with rgb_backlight_mod
                    furi_delay_tick(1);
                    furi_hal_vibro_on(true);
                    furi_delay_tick(settings->vibro_touch_level);
                    furi_hal_vibro_on(false);
                };
            }
        }

        if(is_changing) {
#ifdef INPUT_DEBUG
            furi_hal_gpio_write(&gpio_ext_pa4, 1);
#endif
            furi_delay_tick(1);
        } else {
#ifdef INPUT_DEBUG
            furi_hal_gpio_write(&gpio_ext_pa4, 0);
#endif
            furi_thread_flags_wait(INPUT_THREAD_FLAG_ISR, FuriFlagWaitAny, FuriWaitForever);
        }
    }

    return 0;
}
