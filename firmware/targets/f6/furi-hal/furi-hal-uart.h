#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FuriHalUartIdUSART1,
    FuriHalUartIdLPUART1,
} FuriHalUartId;

typedef enum {
    UartIrqEventRXNE,
    UartIrqEventIDLE,
    //TODO: more events
} UartIrqEvent;

void furi_hal_uart_init(FuriHalUartId ch, uint32_t baud);

void furi_hal_uart_deinit(FuriHalUartId ch);

void furi_hal_uart_set_br(FuriHalUartId ch, uint32_t baud);

void furi_hal_uart_tx(FuriHalUartId ch, uint8_t* buffer, size_t buffer_size);

void furi_hal_uart_set_irq_cb(FuriHalUartId ch, void (*cb)(UartIrqEvent ev, uint8_t data));

#ifdef __cplusplus
}
#endif
