/**
 * @file input_i.h
 * Input: internal API
 */

#pragma once

#include "input.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <furi.h>
#include <cli/cli.h>
#include <m-string.h>
#include <furi_hal_gpio.h>

#define INPUT_DEBOUNCE_TICKS_HALF (INPUT_DEBOUNCE_TICKS / 2)
#define INPUT_PRESS_TICKS 150
#define INPUT_LONG_PRESS_COUNTS 2
#define INPUT_THREAD_FLAG_ISR 0x00000001

/** Input pin state */
typedef struct {
    const InputPin* pin;
    // State
    volatile bool state;
    volatile uint8_t debounce;
    FuriTimer* press_timer;
    volatile uint8_t press_counter;
    volatile uint32_t counter;
} InputPinState;

/** Input state */
typedef struct {
    FuriThreadId thread_id;
    FuriPubSub* event_pubsub;
    InputPinState* pin_states;
    Cli* cli;
    volatile uint32_t counter;
} Input;

/** Input press timer callback */
void input_press_timer_callback(void* arg);

/** Input interrupt handler */
void input_isr(void* _ctx);

/** Input CLI command handler */
void input_cli(Cli* cli, string_t args, void* context);
