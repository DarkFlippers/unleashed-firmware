#pragma once

#include "furi_hal.h"

#define RX_BUF_SIZE (2048)

typedef struct WifiMarauderUart WifiMarauderUart;

void wifi_marauder_uart_set_handle_rx_data_cb(
    WifiMarauderUart* uart,
    void (*handle_rx_data_cb)(uint8_t* buf, size_t len, void* context));
void wifi_marauder_uart_tx(uint8_t* data, size_t len);
void wifi_marauder_lp_uart_tx(uint8_t* data, size_t len);
WifiMarauderUart* wifi_marauder_usart_init(WifiMarauderApp* app);
WifiMarauderUart* wifi_marauder_lp_uart_init(WifiMarauderApp* app);
void wifi_marauder_uart_free(WifiMarauderUart* uart);
