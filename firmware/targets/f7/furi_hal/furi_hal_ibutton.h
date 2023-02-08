/**
 * @file furi_hal_ibutton.h
 * iButton HAL API
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*FuriHalIbuttonEmulateCallback)(void* context);

/** Initialize */
void furi_hal_ibutton_init();

/**
 * Start emulation timer
 * @param period timer period
 * @param callback timer callback
 * @param context callback context
 */
void furi_hal_ibutton_emulate_start(
    uint32_t period,
    FuriHalIbuttonEmulateCallback callback,
    void* context);

/**
 * Update emulation timer period
 * @param period new timer period
 */
void furi_hal_ibutton_emulate_set_next(uint32_t period);

/**
 * Stop emulation timer
 */
void furi_hal_ibutton_emulate_stop();

/**
 * Set the pin to normal mode (open collector), and sets it to float
 */
void furi_hal_ibutton_pin_configure();

/**
 * Sets the pin to analog mode, and sets it to float
 */
void furi_hal_ibutton_pin_reset();

/**
 * iButton write pin
 * @param state true / false
 */
void furi_hal_ibutton_pin_write(const bool state);

#ifdef __cplusplus
}
#endif
