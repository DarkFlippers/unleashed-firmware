#include "devices.h"

#include "registry.h"

void subghz_devices_init(void) {
    furi_check(!subghz_device_registry_is_valid());
    subghz_device_registry_init();
}

void subghz_devices_deinit(void) {
    furi_check(subghz_device_registry_is_valid());
    subghz_device_registry_deinit();
}

const SubGhzDevice* subghz_devices_get_by_name(const char* device_name) {
    furi_check(subghz_device_registry_is_valid());
    const SubGhzDevice* device = subghz_device_registry_get_by_name(device_name);
    return device;
}

const char* subghz_devices_get_name(const SubGhzDevice* device) {
    const char* ret = NULL;
    if(device) {
        ret = device->name;
    }
    return ret;
}

bool subghz_devices_begin(const SubGhzDevice* device) {
    furi_check(device);
    bool ret = false;
    if(device->interconnect->begin) {
        ret = device->interconnect->begin();
    }
    return ret;
}

void subghz_devices_end(const SubGhzDevice* device) {
    furi_check(device);
    if(device->interconnect->end) {
        device->interconnect->end();
    }
}

bool subghz_devices_is_connect(const SubGhzDevice* device) {
    furi_check(device);
    bool ret = false;
    if(device->interconnect->is_connect) {
        ret = device->interconnect->is_connect();
    }
    return ret;
}

void subghz_devices_reset(const SubGhzDevice* device) {
    furi_check(device);
    if(device->interconnect->reset) {
        device->interconnect->reset();
    }
}

void subghz_devices_sleep(const SubGhzDevice* device) {
    furi_check(device);
    if(device->interconnect->sleep) {
        device->interconnect->sleep();
    }
}

void subghz_devices_idle(const SubGhzDevice* device) {
    furi_check(device);
    if(device->interconnect->idle) {
        device->interconnect->idle();
    }
}

void subghz_devices_load_preset(
    const SubGhzDevice* device,
    FuriHalSubGhzPreset preset,
    uint8_t* preset_data) {
    furi_check(device);
    if(device->interconnect->load_preset) {
        device->interconnect->load_preset(preset, preset_data);
    }
}

uint32_t subghz_devices_set_frequency(const SubGhzDevice* device, uint32_t frequency) {
    furi_check(device);
    uint32_t ret = 0;
    if(device->interconnect->set_frequency) {
        ret = device->interconnect->set_frequency(frequency);
    }
    return ret;
}

bool subghz_devices_is_frequency_valid(const SubGhzDevice* device, uint32_t frequency) {
    bool ret = false;
    furi_check(device);
    if(device->interconnect->is_frequency_valid) {
        ret = device->interconnect->is_frequency_valid(frequency);
    }
    return ret;
}

void subghz_devices_set_async_mirror_pin(const SubGhzDevice* device, const GpioPin* gpio) {
    furi_check(device);
    if(device->interconnect->set_async_mirror_pin) {
        device->interconnect->set_async_mirror_pin(gpio);
    }
}

const GpioPin* subghz_devices_get_data_gpio(const SubGhzDevice* device) {
    furi_check(device);
    const GpioPin* ret = NULL;
    if(device->interconnect->get_data_gpio) {
        ret = device->interconnect->get_data_gpio();
    }
    return ret;
}

bool subghz_devices_set_tx(const SubGhzDevice* device) {
    bool ret = 0;
    furi_check(device);
    if(device->interconnect->set_tx) {
        ret = device->interconnect->set_tx();
    }
    return ret;
}

void subghz_devices_flush_tx(const SubGhzDevice* device) {
    furi_check(device);
    if(device->interconnect->flush_tx) {
        device->interconnect->flush_tx();
    }
}

bool subghz_devices_start_async_tx(const SubGhzDevice* device, void* callback, void* context) {
    bool ret = false;
    furi_check(device);
    if(device->interconnect->start_async_tx) {
        ret = device->interconnect->start_async_tx(callback, context);
    }
    return ret;
}

bool subghz_devices_is_async_complete_tx(const SubGhzDevice* device) {
    bool ret = false;
    furi_check(device);
    if(device->interconnect->is_async_complete_tx) {
        ret = device->interconnect->is_async_complete_tx();
    }
    return ret;
}

void subghz_devices_stop_async_tx(const SubGhzDevice* device) {
    furi_check(device);
    if(device->interconnect->stop_async_tx) {
        device->interconnect->stop_async_tx();
    }
}

void subghz_devices_set_rx(const SubGhzDevice* device) {
    furi_check(device);
    if(device->interconnect->set_rx) {
        device->interconnect->set_rx();
    }
}

void subghz_devices_flush_rx(const SubGhzDevice* device) {
    furi_check(device);
    if(device->interconnect->flush_rx) {
        device->interconnect->flush_rx();
    }
}

void subghz_devices_start_async_rx(const SubGhzDevice* device, void* callback, void* context) {
    furi_check(device);
    if(device->interconnect->start_async_rx) {
        device->interconnect->start_async_rx(callback, context);
    }
}

void subghz_devices_stop_async_rx(const SubGhzDevice* device) {
    furi_check(device);
    if(device->interconnect->stop_async_rx) {
        device->interconnect->stop_async_rx();
    }
}

float subghz_devices_get_rssi(const SubGhzDevice* device) {
    float ret = 0;
    furi_check(device);
    if(device->interconnect->get_rssi) {
        ret = device->interconnect->get_rssi();
    }
    return ret;
}

uint8_t subghz_devices_get_lqi(const SubGhzDevice* device) {
    furi_check(device);
    uint8_t ret = 0;
    if(device->interconnect->get_lqi) {
        ret = device->interconnect->get_lqi();
    }
    return ret;
}

bool subghz_devices_rx_pipe_not_empty(const SubGhzDevice* device) {
    furi_check(device);
    bool ret = false;
    if(device->interconnect->rx_pipe_not_empty) {
        ret = device->interconnect->rx_pipe_not_empty();
    }
    return ret;
}

bool subghz_devices_is_rx_data_crc_valid(const SubGhzDevice* device) {
    bool ret = false;
    furi_check(device);
    if(device->interconnect->is_rx_data_crc_valid) {
        ret = device->interconnect->is_rx_data_crc_valid();
    }
    return ret;
}

void subghz_devices_read_packet(const SubGhzDevice* device, uint8_t* data, uint8_t* size) {
    furi_check(device);
    if(device->interconnect->read_packet) {
        device->interconnect->read_packet(data, size);
    }
}

void subghz_devices_write_packet(const SubGhzDevice* device, const uint8_t* data, uint8_t size) {
    furi_check(device);
    if(device->interconnect->write_packet) {
        device->interconnect->write_packet(data, size);
    }
}
