#include <input/input.h>
#include <input_priv.h>
#include <stdio.h>
#include <flipper.h>

static volatile bool initialized = false;
static SemaphoreHandle_t event;
static InputState input_state = {
    false,
};

void input_task(void* p) {
    uint32_t state_bits = 0;
    StaticSemaphore_t event_semaphore;
    uint8_t debounce_counters[INPUT_COUNT];

    event = xSemaphoreCreateCountingStatic(1, 0, &event_semaphore);

    if(!furi_create_deprecated("input_state", (void*)&input_state, sizeof(input_state))) {
        printf("[input_task] cannot create the input_state record\n");
        furiac_exit(NULL);
    }

    FuriRecordSubscriber* input_state_record =
        furi_open_deprecated("input_state", false, false, NULL, NULL, NULL);
    if(input_state_record == NULL) {
        printf("[input_task] cannot open the input_state record\n");
        furiac_exit(NULL);
    }

    if(!furi_create_deprecated("input_events", NULL, 0)) {
        printf("[input_task] cannot create the input_events record\n");
        furiac_exit(NULL);
    }

    FuriRecordSubscriber* input_events_record =
        furi_open_deprecated("input_events", false, false, NULL, NULL, NULL);
    if(input_events_record == NULL) {
        printf("[input_task] cannot open the input_events record\n");
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
            bool input_state = app_gpio_read(input_gpio[i]) ^ input_invert[i];
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
                furi_write(input_state_record, &new_state, sizeof(new_state));

                state_bits = new_state_bits;

                for(uint32_t i = 0; i < INPUT_COUNT; i++) {
                    if((changed_bits & (1 << i)) != 0) {
                        bool state = (new_state_bits & (1 << i)) != 0;
                        InputEvent event = {i, state};
                        furi_write(input_events_record, &event, sizeof(event));
                    }
                }
            }

            // Sleep: wait for event
            xSemaphoreTake(event, portMAX_DELAY);
        } else {
            osDelay(1);
        }
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t pin) {
    if(!initialized) return;

    BaseType_t task_woken = pdFALSE;

    // Ignore the result, as we do not care about repeated event during event processing.
    xSemaphoreGiveFromISR(event, &task_woken);

    if(task_woken) {
        portYIELD_FROM_ISR(task_woken);
    }
}
