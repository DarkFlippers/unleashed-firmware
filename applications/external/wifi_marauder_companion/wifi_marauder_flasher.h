#pragma once

#include "wifi_marauder_app_i.h"
#include "wifi_marauder_uart.h"
#define SERIAL_FLASHER_INTERFACE_UART /* TODO why is application.fam not passing this via cdefines */
#include "esp_loader_io.h"

void wifi_marauder_flash_start_thread(WifiMarauderApp* app);
void wifi_marauder_flash_stop_thread(WifiMarauderApp* app);
void wifi_marauder_flash_handle_rx_data_cb(uint8_t* buf, size_t len, void* context);