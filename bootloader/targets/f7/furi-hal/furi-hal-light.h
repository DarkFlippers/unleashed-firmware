#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <furi-hal-resources.h>

#ifdef __cplusplus
extern "C" {
#endif

void furi_hal_light_init();

void furi_hal_light_set(Light light, uint8_t value);

#ifdef __cplusplus
}
#endif
