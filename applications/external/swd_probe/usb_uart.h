#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct UsbUart UsbUart;

typedef struct {
    uint8_t vcp_ch;
    size_t (*rx_data)(void* ctx, uint8_t* data, size_t length);
    void* rx_data_ctx;
} UsbUartConfig;

typedef struct {
    uint32_t rx_cnt;
    uint32_t tx_cnt;
} UsbUartState;

UsbUart* usb_uart_enable(UsbUartConfig* cfg);

void usb_uart_disable(UsbUart* usb_uart);

void usb_uart_set_config(UsbUart* usb_uart, UsbUartConfig* cfg);

void usb_uart_get_config(UsbUart* usb_uart, UsbUartConfig* cfg);

void usb_uart_get_state(UsbUart* usb_uart, UsbUartState* st);

bool usb_uart_tx_data(UsbUart* usb_uart, uint8_t* data, size_t length);
