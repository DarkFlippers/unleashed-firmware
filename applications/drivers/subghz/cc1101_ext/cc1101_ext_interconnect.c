#include "cc1101_ext_interconnect.h"
#include "cc1101_ext.h"
#include <lib/subghz/devices/cc1101_configs.h>

#define TAG "SubGhzDeviceCC1101Ext"

static bool subghz_device_cc1101_ext_interconnect_is_frequency_valid(uint32_t frequency) {
    bool ret = subghz_device_cc1101_ext_is_frequency_valid(frequency);
    if(!ret) {
        furi_crash("SubGhz: Incorrect frequency.");
    }
    return ret;
}

static uint32_t subghz_device_cc1101_ext_interconnect_set_frequency(uint32_t frequency) {
    subghz_device_cc1101_ext_interconnect_is_frequency_valid(frequency);
    return subghz_device_cc1101_ext_set_frequency(frequency);
}

static bool subghz_device_cc1101_ext_interconnect_start_async_tx(void* callback, void* context) {
    return subghz_device_cc1101_ext_start_async_tx(
        (SubGhzDeviceCC1101ExtCallback)callback, context);
}

static void subghz_device_cc1101_ext_interconnect_start_async_rx(void* callback, void* context) {
    subghz_device_cc1101_ext_start_async_rx(
        (SubGhzDeviceCC1101ExtCaptureCallback)callback, context);
}

static void subghz_device_cc1101_ext_interconnect_load_preset(
    FuriHalSubGhzPreset preset,
    uint8_t* preset_data) {
    switch(preset) {
    case FuriHalSubGhzPresetOok650Async:
        subghz_device_cc1101_ext_load_custom_preset(
            subghz_device_cc1101_preset_ook_650khz_async_regs);
        break;
    case FuriHalSubGhzPresetOok270Async:
        subghz_device_cc1101_ext_load_custom_preset(
            subghz_device_cc1101_preset_ook_270khz_async_regs);
        break;
    case FuriHalSubGhzPreset2FSKDev238Async:
        subghz_device_cc1101_ext_load_custom_preset(
            subghz_device_cc1101_preset_2fsk_dev2_38khz_async_regs);
        break;
    case FuriHalSubGhzPreset2FSKDev476Async:
        subghz_device_cc1101_ext_load_custom_preset(
            subghz_device_cc1101_preset_2fsk_dev47_6khz_async_regs);
        break;
    case FuriHalSubGhzPresetMSK99_97KbAsync:
        subghz_device_cc1101_ext_load_custom_preset(
            subghz_device_cc1101_preset_msk_99_97kb_async_regs);
        break;
    case FuriHalSubGhzPresetGFSK9_99KbAsync:
        subghz_device_cc1101_ext_load_custom_preset(
            subghz_device_cc1101_preset_gfsk_9_99kb_async_regs);
        break;

    default:
        subghz_device_cc1101_ext_load_custom_preset(preset_data);
    }
}

const SubGhzDeviceInterconnect subghz_device_cc1101_ext_interconnect = {
    .begin = subghz_device_cc1101_ext_alloc,
    .end = subghz_device_cc1101_ext_free,
    .is_connect = subghz_device_cc1101_ext_is_connect,
    .reset = subghz_device_cc1101_ext_reset,
    .sleep = subghz_device_cc1101_ext_sleep,
    .idle = subghz_device_cc1101_ext_idle,
    .load_preset = subghz_device_cc1101_ext_interconnect_load_preset,
    .set_frequency = subghz_device_cc1101_ext_interconnect_set_frequency,
    .is_frequency_valid = subghz_device_cc1101_ext_is_frequency_valid,
    .set_async_mirror_pin = subghz_device_cc1101_ext_set_async_mirror_pin,
    .get_data_gpio = subghz_device_cc1101_ext_get_data_gpio,

    .set_tx = subghz_device_cc1101_ext_tx,
    .flush_tx = subghz_device_cc1101_ext_flush_tx,
    .start_async_tx = subghz_device_cc1101_ext_interconnect_start_async_tx,
    .is_async_complete_tx = subghz_device_cc1101_ext_is_async_tx_complete,
    .stop_async_tx = subghz_device_cc1101_ext_stop_async_tx,

    .set_rx = subghz_device_cc1101_ext_rx,
    .flush_rx = subghz_device_cc1101_ext_flush_rx,
    .start_async_rx = subghz_device_cc1101_ext_interconnect_start_async_rx,
    .stop_async_rx = subghz_device_cc1101_ext_stop_async_rx,

    .get_rssi = subghz_device_cc1101_ext_get_rssi,
    .get_lqi = subghz_device_cc1101_ext_get_lqi,

    .rx_pipe_not_empty = subghz_device_cc1101_ext_rx_pipe_not_empty,
    .is_rx_data_crc_valid = subghz_device_cc1101_ext_is_rx_data_crc_valid,
    .read_packet = subghz_device_cc1101_ext_read_packet,
    .write_packet = subghz_device_cc1101_ext_write_packet,
};

const SubGhzDevice subghz_device_cc1101_ext = {
    .name = SUBGHZ_DEVICE_CC1101_EXT_NAME,
    .interconnect = &subghz_device_cc1101_ext_interconnect,
};

static const FlipperAppPluginDescriptor subghz_device_cc1101_ext_descriptor = {
    .appid = SUBGHZ_RADIO_DEVICE_PLUGIN_APP_ID,
    .ep_api_version = SUBGHZ_RADIO_DEVICE_PLUGIN_API_VERSION,
    .entry_point = &subghz_device_cc1101_ext,
};

const FlipperAppPluginDescriptor* subghz_device_cc1101_ext_ep() {
    return &subghz_device_cc1101_ext_descriptor;
}