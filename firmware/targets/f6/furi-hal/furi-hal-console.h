#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void furi_hal_console_init();

void furi_hal_console_tx(const uint8_t* buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif
