#pragma once

#include "wifi_marauder_app_i.h"
#include "wifi_marauder_uart.h"
#define SERIAL_FLASHER_INTERFACE_UART /* TODO why is application.fam not passing this via cdefines */
#include "esp_loader_io.h"

#define ESP_ADDR_BOOT_S3 0x0
#define ESP_ADDR_BOOT 0x1000
#define ESP_ADDR_PART 0x8000
#define ESP_ADDR_NVS 0x9000
#define ESP_ADDR_BOOT_APP0 0xE000
#define ESP_ADDR_APP 0x10000

void wifi_marauder_flash_start_thread(WifiMarauderApp* app);
void wifi_marauder_flash_stop_thread(WifiMarauderApp* app);
void wifi_marauder_flash_handle_rx_data_cb(uint8_t* buf, size_t len, void* context);