/**
 * @file furi_hal_ibutton.h
 * iButton HAL API
 */

#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void furi_hal_ibutton_start();

void furi_hal_ibutton_stop();

void furi_hal_ibutton_pin_low();

void furi_hal_ibutton_pin_high();

bool furi_hal_ibutton_pin_get_level();

#ifdef __cplusplus
}
#endif