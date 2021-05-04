#pragma once
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void api_hal_ibutton_start();

void api_hal_ibutton_stop();

void api_hal_ibutton_pin_low();

void api_hal_ibutton_pin_high();

bool api_hal_ibutton_pin_get_level();

#ifdef __cplusplus
}
#endif