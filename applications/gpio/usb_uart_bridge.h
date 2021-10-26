#pragma once

#include <stdint.h>

typedef struct {
    uint8_t vcp_ch;
    uint8_t uart_ch;
    uint32_t baudrate;
} UsbUartConfig;

void usb_uart_enable(UsbUartConfig* cfg);

void usb_uart_disable();
