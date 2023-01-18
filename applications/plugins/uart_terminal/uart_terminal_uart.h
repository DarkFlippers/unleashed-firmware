#pragma once

#include "furi_hal.h"

#define RX_BUF_SIZE (320)

typedef struct UART_TerminalUart UART_TerminalUart;

void uart_terminal_uart_set_handle_rx_data_cb(
    UART_TerminalUart* uart,
    void (*handle_rx_data_cb)(uint8_t* buf, size_t len, void* context));
void uart_terminal_uart_tx(uint8_t* data, size_t len);
UART_TerminalUart* uart_terminal_uart_init(UART_TerminalApp* app);
void uart_terminal_uart_free(UART_TerminalUart* uart);
