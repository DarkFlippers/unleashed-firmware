#include <input/input.h>
#include <stdio.h>
#include <flipper_v2.h>

#ifdef APP_NFC
void nfc_isr(void);
#endif

static volatile bool initialized = false;
static ValueManager input_state_record;
static PubSub input_events_record;
static Event event;
static InputState input_state = {
    false,
};

void input_task(void* p) {
    uint32_t state_bits = 0;
    uint8_t debounce_counters[INPUT_COUNT];

    if(!init_managed(&input_state_record, &input_state, sizeof(input_state))) {
        printf("[input_task] cannot initialize ValueManager for input_state\n");
        furiac_exit(NULL);
    }
    if(!init_pubsub(&input_events_record)) {
        printf("[input_task] cannot initialize PubSub for input_events\n");
        furiac_exit(NULL);
    }
    if(!init_event(&event)) {
        printf("[input_task] cannot initialize Event\n");
        furiac_exit(NULL);
    }

    if(!furi_create("input_state", &input_state_record)) {
        printf("[input_task] cannot create the input_state record\n");
        furiac_exit(NULL);
    }

    if(!furi_create("input_events", &input_events_record)) {
        printf("[input_task] cannot create the input_events record\n");
        furiac_exit(NULL);
    }

    // we ready to work
    furiac_ready();
    initialized = true;

    // Force state update
    for(uint32_t i = 0; i < INPUT_COUNT; i++) {
        debounce_counters[i] = DEBOUNCE_TICKS / 2;
    }

    for(;;) {
        bool changed = false;
        for(uint32_t i = 0; i < INPUT_COUNT; i++) {
            bool input_state = false;

            // dirty hack, f3 has no CHARGING pin
            // TODO rewrite this
            if(i < GPIO_INPUT_PINS_COUNT) {
                input_state = gpio_read(&input_gpio[i]) ^ input_invert[i];
            }

            if(input_state) {
                if(debounce_counters[i] < DEBOUNCE_TICKS) {
                    debounce_counters[i] += 1;
                    changed = true;
                }
            } else {
                if(debounce_counters[i] > 0) {
                    debounce_counters[i] -= 1;
                    changed = true;
                }
            }
        }

        if(!changed) {
            uint32_t new_state_bits = 0;
            for(uint32_t i = 0; i < INPUT_COUNT; i++) {
                if(debounce_counters[i] == DEBOUNCE_TICKS) {
                    new_state_bits |= (1 << i);
                }
            }
            uint32_t changed_bits = new_state_bits ^ state_bits;

            if(changed_bits != 0) {
                // printf("[input] %02x -> %02x\n", state_bits, new_state_bits);
                InputState new_state = _BITS2STATE(new_state_bits);
                write_managed(&input_state_record, &new_state, sizeof(new_state), osWaitForever);

                state_bits = new_state_bits;

                for(uint32_t i = 0; i < INPUT_COUNT; i++) {
                    if((changed_bits & (1 << i)) != 0) {
                        bool state = (new_state_bits & (1 << i)) != 0;
                        InputEvent event = {i, state};
                        notify_pubsub(&input_events_record, &event);
                    }
                }
            }

            // Sleep: wait for event
            wait_event(&event);
        } else {
            osDelay(1);
        }
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t pin) {
#ifdef APP_NFC
    if(pin == NFC_IRQ_Pin) {
        nfc_isr();
        return;
    }
#endif

    if(!initialized) return;

    signal_event(&event);
}
