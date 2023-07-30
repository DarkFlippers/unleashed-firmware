#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <furi.h>
#include <furi_hal.h>

#include "preset.h"

#include <flipper_application/flipper_application.h>

#define SUBGHZ_RADIO_DEVICE_PLUGIN_APP_ID "subghz_radio_device"
#define SUBGHZ_RADIO_DEVICE_PLUGIN_API_VERSION 2

typedef struct SubGhzDeviceRegistry SubGhzDeviceRegistry;
typedef struct SubGhzDevice SubGhzDevice;
typedef struct SubGhzDeviceConf SubGhzDeviceConf;

typedef bool (*SubGhzBegin)(SubGhzDeviceConf* conf);
typedef void (*SubGhzEnd)(void);
typedef bool (*SubGhzIsConnect)(void);
typedef void (*SubGhzReset)(void);
typedef void (*SubGhzSleep)(void);
typedef void (*SubGhzIdle)(void);
typedef void (*SubGhzLoadPreset)(FuriHalSubGhzPreset preset, uint8_t* preset_data);
typedef uint32_t (*SubGhzSetFrequency)(uint32_t frequency);
typedef bool (*SubGhzIsFrequencyValid)(uint32_t frequency);

typedef void (*SubGhzSetAsyncMirrorPin)(const GpioPin* gpio);
typedef const GpioPin* (*SubGhzGetDataGpio)(void);

typedef bool (*SubGhzSetTx)(void);
typedef void (*SubGhzFlushTx)(void);
typedef bool (*SubGhzStartAsyncTx)(void* callback, void* context);
typedef bool (*SubGhzIsAsyncCompleteTx)(void);
typedef void (*SubGhzStopAsyncTx)(void);

typedef void (*SubGhzSetRx)(void);
typedef void (*SubGhzFlushRx)(void);
typedef void (*SubGhzStartAsyncRx)(void* callback, void* context);
typedef void (*SubGhzStopAsyncRx)(void);

typedef float (*SubGhzGetRSSI)(void);
typedef uint8_t (*SubGhzGetLQI)(void);

typedef bool (*SubGhzRxPipeNotEmpty)(void);
typedef bool (*SubGhzRxIsDataCrcValid)(void);
typedef void (*SubGhzReadPacket)(uint8_t* data, uint8_t* size);
typedef void (*SubGhzWritePacket)(const uint8_t* data, uint8_t size);

typedef struct {
    SubGhzBegin begin;
    SubGhzEnd end;

    SubGhzIsConnect is_connect;
    SubGhzReset reset;
    SubGhzSleep sleep;
    SubGhzIdle idle;

    SubGhzLoadPreset load_preset;
    SubGhzSetFrequency set_frequency;
    SubGhzIsFrequencyValid is_frequency_valid;
    SubGhzSetAsyncMirrorPin set_async_mirror_pin;
    SubGhzGetDataGpio get_data_gpio;

    SubGhzSetTx set_tx;
    SubGhzFlushTx flush_tx;
    SubGhzStartAsyncTx start_async_tx;
    SubGhzIsAsyncCompleteTx is_async_complete_tx;
    SubGhzStopAsyncTx stop_async_tx;

    SubGhzSetRx set_rx;
    SubGhzFlushRx flush_rx;
    SubGhzStartAsyncRx start_async_rx;
    SubGhzStopAsyncRx stop_async_rx;

    SubGhzGetRSSI get_rssi;
    SubGhzGetLQI get_lqi;

    SubGhzRxPipeNotEmpty rx_pipe_not_empty;
    SubGhzRxIsDataCrcValid is_rx_data_crc_valid;
    SubGhzReadPacket read_packet;
    SubGhzWritePacket write_packet;

} SubGhzDeviceInterconnect;

struct SubGhzDevice {
    const char* name;
    const SubGhzDeviceInterconnect* interconnect;
};

struct SubGhzDeviceConf {
    uint8_t ver;
    bool extended_range;
    bool power_amp;
};