#include "api-hal-subghz.h"

#include <api-hal-gpio.h>
#include <api-hal-spi.h>
#include <api-hal-interrupt.h>
#include <api-hal-resources.h>

#include <furi.h>
#include <cc1101.h>
#include <stdio.h>

static volatile SubGhzState api_hal_subghz_state = SubGhzStateInit;

static const uint8_t api_hal_subghz_preset_ook_async_regs[][2] = {
    /* Base setting */
    { CC1101_IOCFG0,    0x0D }, // GD0 as async serial data output/input
    { CC1101_MCSM0,     0x18 }, // Autocalibrate on idle to TRX, ~150us OSC guard time

    /* Async OOK Specific things */
    { CC1101_MDMCFG2,   0x30 }, // ASK/OOK, No preamble/sync
    { CC1101_PKTCTRL0,  0x32 }, // Async, no CRC, Infinite
    { CC1101_FREND0,    0x01 }, // OOK/ASK PATABLE

    /* End  */
    { 0, 0 },
};

static const uint8_t api_hal_subghz_preset_ook_async_patable[8] = {
    0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const uint8_t api_hal_subghz_preset_mp_regs[][2] = {
    { CC1101_IOCFG0, 0x0D },
    { CC1101_FIFOTHR, 0x07 },
    { CC1101_PKTCTRL0, 0x32 },
    //{ CC1101_FSCTRL1,  0x0E },
    { CC1101_FSCTRL1, 0x06 },
    { CC1101_FREQ2, 0x10 },
    { CC1101_FREQ1, 0xB0 },
    { CC1101_FREQ0, 0x7F },
    { CC1101_MDMCFG4, 0x17 },
    { CC1101_MDMCFG3, 0x32 },
    { CC1101_MDMCFG2, 0x30 },   //<---OOK/ASK
    { CC1101_MDMCFG1, 0x23 },
    { CC1101_MDMCFG0, 0xF8 },
    { CC1101_MCSM0, 0x18 },
    { CC1101_FOCCFG, 0x18 },
    { CC1101_AGCTRL2, 0x07 },
    { CC1101_AGCTRL1, 0x00 },
    { CC1101_AGCTRL0, 0x91 },
    { CC1101_WORCTRL, 0xFB },
    { CC1101_FREND1, 0xB6 },
    //{ CC1101_FREND0,   0x11 },
    { CC1101_FREND0, 0x01 },
    { CC1101_FSCAL3, 0xE9 },
    { CC1101_FSCAL2, 0x2A },
    { CC1101_FSCAL1, 0x00 },
    { CC1101_FSCAL0, 0x1F },
    { CC1101_TEST2, 0x88 },
    { CC1101_TEST1, 0x31 },
    { CC1101_TEST0, 0x09 },

    /* End  */
    { 0, 0 },
};

static const uint8_t api_hal_subghz_preset_mp_patable[8] = {
    0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const uint8_t api_hal_subghz_preset_2fsk_packet_regs[][2] = {
    /* Base setting */
    { CC1101_IOCFG0,    0x06 }, // GD0 as async serial data output/input
    { CC1101_MCSM0,     0x18 }, // Autocalibrate on idle to TRX, ~150us OSC guard time

    /* Magic */
    { CC1101_TEST2,     0x81},
    { CC1101_TEST1,     0x35},
    { CC1101_TEST0,     0x09},

    /* End */
    { 0, 0 },
};

static const uint8_t api_hal_subghz_preset_2fsk_packet_patable[8] = {
    0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void api_hal_subghz_init() {
    furi_assert(api_hal_subghz_state == SubGhzStateInit);
    api_hal_subghz_state = SubGhzStateIdle;

    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);

    // Reset
    hal_gpio_init(&gpio_cc1101_g0, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
    cc1101_reset(device);
    cc1101_write_reg(device, CC1101_IOCFG0, CC1101IocfgHighImpedance);

    // Prepare GD0 for power on self test
    hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);

    // GD0 low
    cc1101_write_reg(device, CC1101_IOCFG0, CC1101IocfgHW);
    while(hal_gpio_read(&gpio_cc1101_g0) != false);

    // GD0 high
    cc1101_write_reg(device, CC1101_IOCFG0, CC1101IocfgHW | CC1101_IOCFG_INV);
    while(hal_gpio_read(&gpio_cc1101_g0) != true);

    // Reset GD0 to floating state
    cc1101_write_reg(device, CC1101_IOCFG0, CC1101IocfgHighImpedance);
    hal_gpio_init(&gpio_cc1101_g0, GpioModeAnalog, GpioPullNo, GpioSpeedLow);

    // RF switches
    hal_gpio_init(&gpio_rf_sw_0, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    hal_gpio_init(&gpio_rf_sw_1, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);

    // Go to sleep
    cc1101_shutdown(device);

    api_hal_spi_device_return(device);
}

void api_hal_subghz_sleep() {
    furi_assert(api_hal_subghz_state == SubGhzStateIdle);
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);

    cc1101_switch_to_idle(device);

    cc1101_write_reg(device, CC1101_IOCFG0, CC1101IocfgHighImpedance);
    hal_gpio_init(&gpio_cc1101_g0, GpioModeAnalog, GpioPullNo, GpioSpeedLow);

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
    } else if(preset == ApiHalSubGhzPresetMP) {
        api_hal_subghz_load_registers(api_hal_subghz_preset_mp_regs);
        api_hal_subghz_load_patable(api_hal_subghz_preset_mp_patable);
    }
}

uint8_t api_hal_subghz_get_status() {
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);
    CC1101StatusRaw st;
    st.status = cc1101_get_status(device);
    api_hal_spi_device_return(device);
    return st.status_raw;
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

void api_hal_subghz_flush_rx() {
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);
    cc1101_flush_rx(device);
    api_hal_spi_device_return(device);
}

void api_hal_subghz_read_packet(uint8_t* data, uint8_t* size) {
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);
    cc1101_read_fifo(device, data, size);
    api_hal_spi_device_return(device);
}

void api_hal_subghz_shutdown() {
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);
    // Reset and shutdown
    cc1101_shutdown(device);
    api_hal_spi_device_return(device);
}

void api_hal_subghz_reset() {
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);
    hal_gpio_init(&gpio_cc1101_g0, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
    cc1101_switch_to_idle(device);
    cc1101_reset(device);
    cc1101_write_reg(device, CC1101_IOCFG0, CC1101IocfgHighImpedance);
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

uint32_t api_hal_subghz_set_frequency_and_path(uint32_t value) {
    value = api_hal_subghz_set_frequency(value);
    if(value >= 300000000 && value <= 348000335) {
        api_hal_subghz_set_path(ApiHalSubGhzPath315);
    } else if(value >= 387000000 && value <= 464000000) {
        api_hal_subghz_set_path(ApiHalSubGhzPath433);
    } else if(value >= 779000000 && value <= 928000000) {
        api_hal_subghz_set_path(ApiHalSubGhzPath868);
    } else {
        furi_check(0);
    }
    return value;
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
    if (path == ApiHalSubGhzPath433) {
        hal_gpio_write(&gpio_rf_sw_0, 0);
        hal_gpio_write(&gpio_rf_sw_1, 1);
    } else if (path == ApiHalSubGhzPath315) {
        hal_gpio_write(&gpio_rf_sw_0, 1);
        hal_gpio_write(&gpio_rf_sw_1, 0);
    } else if (path == ApiHalSubGhzPath868) {
        hal_gpio_write(&gpio_rf_sw_0, 1);
        hal_gpio_write(&gpio_rf_sw_1, 1);
    } else if (path == ApiHalSubGhzPathIsolate) {
        hal_gpio_write(&gpio_rf_sw_0, 0);
        hal_gpio_write(&gpio_rf_sw_1, 0);
    } else {
        furi_check(0);
    }
}

void api_hal_subghz_set_async_rx_callback(ApiHalSubGhzCaptureCallback callback, void* context) {}

void api_hal_subghz_start_async_rx() {}

void api_hal_subghz_stop_async_rx() {}

void api_hal_subghz_start_async_tx(uint32_t* buffer, size_t buffer_size, size_t repeat) {}

void api_hal_subghz_wait_async_tx() {}

void api_hal_subghz_stop_async_tx() {}

