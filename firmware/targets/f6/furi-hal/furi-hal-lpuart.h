#pragma once

#include <stddef.h>
#include <stdint.h>
#include "furi-hal-console.h"

#ifdef __cplusplus
extern "C" {
#endif


void furi_hal_lpuart_init();

void furi_hal_lpuart_deinit();

void furi_hal_lpuart_set_br(uint32_t baud);

void furi_hal_lpuart_tx(const uint8_t* buffer, size_t buffer_size);

void furi_hal_lpuart_set_irq_cb(void (*cb)(UartIrqEvent ev, uint8_t data));

#ifdef __cplusplus
}
#endif
