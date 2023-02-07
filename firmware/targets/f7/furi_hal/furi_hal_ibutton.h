/**
 * @file furi_hal_ibutton.h
 * iButton HAL API
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <furi_hal_gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*FuriHalIbuttonEmulateCallback)(void* context);

/** Initialize */
void furi_hal_ibutton_init();

void furi_hal_ibutton_emulate_start(
    uint32_t period,
    FuriHalIbuttonEmulateCallback callback,
    void* context);

void furi_hal_ibutton_emulate_set_next(uint32_t period);

void furi_hal_ibutton_emulate_stop();

/**
 * Sets the pin to normal mode (open collector), and sets it to float
 */
void furi_hal_ibutton_start_drive();

/**
 * Sets the pin to normal mode (open collector), and clears pin EXTI interrupt.
 * Used in EXTI interrupt context.
 */
void furi_hal_ibutton_start_drive_in_isr();

/**
 * Sets the pin to interrupt mode (EXTI interrupt on rise or fall), and sets it to float
 */
void furi_hal_ibutton_start_interrupt();

/**
 * Sets the pin to interrupt mode (EXTI interrupt on rise or fall), and clears pin EXTI interrupt.
 * Used in EXTI interrupt context.
 */
void furi_hal_ibutton_start_interrupt_in_isr();

/**
 * Sets the pin to analog mode, and sets it to float
 */
void furi_hal_ibutton_stop();

/**
 * Attach interrupt callback to iButton pin
 * @param cb callback
 * @param context context
 */
void furi_hal_ibutton_add_interrupt(GpioExtiCallback cb, void* context);

/**
 * Remove interrupt callback from iButton pin
 */
void furi_hal_ibutton_remove_interrupt();

/**
 * Sets the pin to low
 */
void furi_hal_ibutton_pin_low();

/**
 * Sets the pin to high (float in iButton pin modes)
 */
void furi_hal_ibutton_pin_high();

/**
 * Get pin level
 * @return true if level is high
 * @return false if level is low
 */
bool furi_hal_ibutton_pin_get_level();

#ifdef __cplusplus
}
#endif
