#pragma once

#include "input.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <furi.h>

#define INPUT_DEBOUNCE_TICKS_HALF (INPUT_DEBOUNCE_TICKS / 2)
#define INPUT_LONG_PRESS_TICKS 2048
#define INPUT_THREAD_FLAG_ISR 0x00000001

/* Input pin state */
typedef struct {
    const InputPin* pin;
    // State
    volatile bool state;
    volatile uint8_t debounce;
    volatile osTimerId_t press_timer;
} InputPinState;

/* Input state */
typedef struct {
    osThreadId_t thread;
    PubSub event_pubsub;
    InputPinState* pin_states;
} Input;

/* Input press timer callback */
void input_press_timer_callback(void* arg);

/* Input interrupt handler */
void input_isr(void* _pin, void* _ctx);
