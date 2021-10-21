#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UartIrqEventRXNE,
    UartIrqEventIDLE,
    //TODO: more events
} UartIrqEvent;

void furi_hal_console_init();

void furi_hal_console_tx(const uint8_t* buffer, size_t buffer_size);

void furi_hal_console_tx_with_new_line(const uint8_t* buffer, size_t buffer_size);

/**
 * Printf-like plain uart interface
 * @warning Will not work in ISR context
 * @param format 
 * @param ... 
 */
void furi_hal_console_printf(const char format[], ...);

void furi_hal_console_puts(const char* data);


void furi_hal_usart_init();

void furi_hal_usart_deinit();

void furi_hal_usart_set_br(uint32_t baud);

void furi_hal_usart_tx(const uint8_t* buffer, size_t buffer_size);

void furi_hal_usart_set_irq_cb(void (*cb)(UartIrqEvent ev, uint8_t data));


#ifdef __cplusplus
}
#endif
