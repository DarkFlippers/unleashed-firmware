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

void furi_hal_ibutton_emulate_start(
    uint32_t period,
    FuriHalIbuttonEmulateCallback callback,
    void* context);

void furi_hal_ibutton_emulate_set_next(uint32_t period);

void furi_hal_ibutton_emulate_stop();

void furi_hal_ibutton_start();

void furi_hal_ibutton_stop();

void furi_hal_ibutton_pin_low();

void furi_hal_ibutton_pin_high();

bool furi_hal_ibutton_pin_get_level();

#ifdef __cplusplus
}
#endif
