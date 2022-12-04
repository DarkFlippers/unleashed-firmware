#pragma once

#include "furi_hal.h"

#define RX_BUF_SIZE (320)

typedef struct WifideautherUart WifideautherUart;

void wifi_deauther_uart_set_handle_rx_data_cb(
    WifideautherUart* uart,
    void (*handle_rx_data_cb)(uint8_t* buf, size_t len, void* context));
void wifi_deauther_uart_tx(uint8_t* data, size_t len);
WifideautherUart* wifi_deauther_uart_init(WifideautherApp* app);
void wifi_deauther_uart_free(WifideautherUart* uart);
