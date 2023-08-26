#pragma once

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SubGhzDevice SubGhzDevice;

void subghz_devices_init();
void subghz_devices_deinit(void);

const SubGhzDevice* subghz_devices_get_by_name(const char* device_name);
const char* subghz_devices_get_name(const SubGhzDevice* device);
bool subghz_devices_begin(const SubGhzDevice* device);
void subghz_devices_end(const SubGhzDevice* device);
bool subghz_devices_is_connect(const SubGhzDevice* device);
void subghz_devices_reset(const SubGhzDevice* device);
void subghz_devices_sleep(const SubGhzDevice* device);
void subghz_devices_idle(const SubGhzDevice* device);
void subghz_devices_load_preset(
    const SubGhzDevice* device,
    FuriHalSubGhzPreset preset,
    uint8_t* preset_data);
uint32_t subghz_devices_set_frequency(const SubGhzDevice* device, uint32_t frequency);
bool subghz_devices_is_frequency_valid(const SubGhzDevice* device, uint32_t frequency);
void subghz_devices_set_async_mirror_pin(const SubGhzDevice* device, const GpioPin* gpio);
const GpioPin* subghz_devices_get_data_gpio(const SubGhzDevice* device);

bool subghz_devices_set_tx(const SubGhzDevice* device);
void subghz_devices_flush_tx(const SubGhzDevice* device);
bool subghz_devices_start_async_tx(const SubGhzDevice* device, void* callback, void* context);
bool subghz_devices_is_async_complete_tx(const SubGhzDevice* device);
void subghz_devices_stop_async_tx(const SubGhzDevice* device);

void subghz_devices_set_rx(const SubGhzDevice* device);
void subghz_devices_flush_rx(const SubGhzDevice* device);
void subghz_devices_start_async_rx(const SubGhzDevice* device, void* callback, void* context);
void subghz_devices_stop_async_rx(const SubGhzDevice* device);

float subghz_devices_get_rssi(const SubGhzDevice* device);
uint8_t subghz_devices_get_lqi(const SubGhzDevice* device);

bool subghz_devices_rx_pipe_not_empty(const SubGhzDevice* device);
bool subghz_devices_is_rx_data_crc_valid(const SubGhzDevice* device);
void subghz_devices_read_packet(const SubGhzDevice* device, uint8_t* data, uint8_t* size);
void subghz_devices_write_packet(const SubGhzDevice* device, const uint8_t* data, uint8_t size);

#ifdef __cplusplus
}
#endif
