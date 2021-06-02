#pragma once

#include "input.h"
#include <FreeRTOS.h>
#include <timers.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <furi.h>
#include <cli/cli.h>
#include <m-string.h>
#include <api-hal-gpio.h>

#define INPUT_DEBOUNCE_TICKS_HALF (INPUT_DEBOUNCE_TICKS / 2)
#define INPUT_PRESS_TICKS 150
#define INPUT_LONG_PRESS_COUNTS 2
#define INPUT_THREAD_FLAG_ISR 0x00000001

/* Input pin state */
typedef struct {
    const InputPin* pin;
    // State
    volatile bool state;
    volatile uint8_t debounce;
    volatile osTimerId_t press_timer;
    volatile uint8_t press_counter;
} InputPinState;

/* Input state */
typedef struct {
    osThreadId_t thread;
    PubSub event_pubsub;
    InputPinState* pin_states;
    Cli* cli;
} Input;

/* Input press timer callback */
void input_press_timer_callback(void* arg);

/* Input interrupt handler */
void input_isr(void* _ctx);
