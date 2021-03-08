#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <api-hal-resources.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize vibro */
void api_hal_vibro_init();

/* Turn on/off vibro */
void api_hal_vibro_on(bool value);

#ifdef __cplusplus
}
#endif
