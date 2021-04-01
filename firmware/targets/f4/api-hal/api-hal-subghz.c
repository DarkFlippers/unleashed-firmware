#include "api-hal-subghz.h"
#include <api-hal-gpio.h>
#include <api-hal-spi.h>
#include <cc1101.h>
#include <stdio.h>
#include "main.h"

static const uint8_t api_hal_subghz_preset_ook_async_regs[][2] = {
    /* Base setting */
    { CC1101_IOCFG0,    0x0D }, // GD0 as async serial data output/input
    { CC1101_FSCTRL1,   0x06 }, // Set IF 26m/2^10*2=2.2MHz
    { CC1101_MCSM0,     0x18 }, // Autocalibrate on idle to TRX, ~150us OSC guard time
    /* Async OOK Specific things  */
    { CC1101_MDMCFG2,   0x30 }, // ASK/OOK, No preamble/sync
    { CC1101_PKTCTRL0,  0x32 }, // Async, no CRC, Infinite
    { CC1101_FREND0,    0x01 }, // OOK/ASK PATABLE
    /* End  */
    { 0, 0 },
};

static const uint8_t api_hal_subghz_preset_ook_async_patable[8] = {
    0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const uint8_t api_hal_subghz_preset_2fsk_packet_regs[][2] = {
    /* Base setting */
    { CC1101_IOCFG0,    0x06 }, // GD0 as async serial data output/input
    { CC1101_FSCTRL1,   0x06 }, // Set IF 26m/2^10*2=2.2MHz
    { CC1101_MCSM0,     0x18 }, // Autocalibrate on idle to TRX, ~150us OSC guard time
    /* End */
    { 0, 0 },
};

static const uint8_t api_hal_subghz_preset_2fsk_packet_patable[8] = {
    0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void api_hal_subghz_init() {
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);
    // Reset and shutdown
    cc1101_reset(device);
    cc1101_write_reg(device, CC1101_IOCFG0, 0x2E); // High impedance 3-state
    cc1101_shutdown(device);
    api_hal_spi_device_return(device);
}

void api_hal_subghz_dump_state() {
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);
    printf(
        "[api_hal_subghz] cc1101 chip %d, version %d\r\n",
        cc1101_get_partnumber(device),
        cc1101_get_version(device)
    );
    api_hal_spi_device_return(device);
}

void api_hal_subghz_load_preset(ApiHalSubGhzPreset preset) {
    if(preset == ApiHalSubGhzPresetOokAsync) {
        api_hal_subghz_load_registers(api_hal_subghz_preset_ook_async_regs);
        api_hal_subghz_load_patable(api_hal_subghz_preset_ook_async_patable);
    } else if(preset == ApiHalSubGhzPreset2FskPacket) {
        api_hal_subghz_load_registers(api_hal_subghz_preset_2fsk_packet_regs);
        api_hal_subghz_load_patable(api_hal_subghz_preset_2fsk_packet_patable);
    }
}

void api_hal_subghz_load_registers(const uint8_t data[][2]) {
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);
    cc1101_reset(device);
    uint32_t i = 0;
    while (data[i][0]) {
        cc1101_write_reg(device, data[i][0], data[i][1]);
        i++;
    }
    api_hal_spi_device_return(device);
}

void api_hal_subghz_load_patable(const uint8_t data[8]) {
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);
    cc1101_set_pa_table(device, data);
    api_hal_spi_device_return(device);
}

void api_hal_subghz_write_packet(const uint8_t* data, uint8_t size) {
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);
    cc1101_flush_tx(device);
    cc1101_write_fifo(device, data, size);
    api_hal_spi_device_return(device);
}

void api_hal_subghz_read_packet(uint8_t* data, uint8_t size) {

}

void api_hal_subghz_shutdown() {
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);
    // Reset and shutdown
    cc1101_shutdown(device);
    api_hal_spi_device_return(device);
}

void api_hal_subghz_reset() {
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);
    cc1101_reset(device);
    api_hal_spi_device_return(device);
}

void api_hal_subghz_idle() {
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);
    cc1101_switch_to_idle(device);
    api_hal_spi_device_return(device);
}

void api_hal_subghz_rx() {
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);
    cc1101_switch_to_rx(device);
    api_hal_spi_device_return(device);
}

void api_hal_subghz_tx() {
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);
    cc1101_switch_to_tx(device);
    api_hal_spi_device_return(device);
}

float api_hal_subghz_get_rssi() {
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);
    int32_t rssi_dec = cc1101_get_rssi(device);
    api_hal_spi_device_return(device);

    float rssi = rssi_dec;
    if(rssi_dec >= 128) {
        rssi = ((rssi - 256.0f) / 2.0f) - 74.0f;
    } else {
        rssi = (rssi / 2.0f) - 74.0f;
    }

    return rssi;
}

uint32_t api_hal_subghz_set_frequency(uint32_t value) {
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);

    // Compensate rounding
    if (value % cc1101_get_frequency_step(device) > (cc1101_get_frequency_step(device) / 2)) {
        value += cc1101_get_frequency_step(device);
    }

    uint32_t real_frequency = cc1101_set_frequency(device, value);
    cc1101_calibrate(device);

    api_hal_spi_device_return(device);

    return real_frequency;
}

void api_hal_subghz_set_path(ApiHalSubGhzPath path) {
}
