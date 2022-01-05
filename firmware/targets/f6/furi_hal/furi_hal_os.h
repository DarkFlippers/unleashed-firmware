#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize OS helpers
 * Configure and start tick timer
 */
void furi_hal_os_init();

#ifdef __cplusplus
}
#endif