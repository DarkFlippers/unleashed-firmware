#include "cc1101.h"
#include <cmsis_os2.h>
#include <furi-hal-delay.h>
#include <assert.h>
#include <string.h>

CC1101Status cc1101_strobe(const FuriHalSpiDevice* device, uint8_t strobe) {
    uint8_t tx[1] = { strobe };
    CC1101Status rx[1] = { 0 };

    hal_gpio_write(device->chip_select, false);
    while(hal_gpio_read(device->bus->miso));
    furi_hal_spi_bus_trx(device->bus, tx, (uint8_t*)rx, 1, CC1101_TIMEOUT);
    hal_gpio_write(device->chip_select, true);

    assert(rx[0].CHIP_RDYn == 0);
    return rx[0];
}

CC1101Status cc1101_write_reg(const FuriHalSpiDevice* device, uint8_t reg, uint8_t data) {
    uint8_t tx[2] = { reg, data };
    CC1101Status rx[2] = { 0 };

    hal_gpio_write(device->chip_select, false);
    while(hal_gpio_read(device->bus->miso));
    furi_hal_spi_bus_trx(device->bus, tx, (uint8_t*)rx, 2, CC1101_TIMEOUT);
    hal_gpio_write(device->chip_select, true);

    assert((rx[0].CHIP_RDYn|rx[1].CHIP_RDYn) == 0);
    return rx[1];
}

CC1101Status cc1101_read_reg(const FuriHalSpiDevice* device, uint8_t reg, uint8_t* data) {
    assert(sizeof(CC1101Status) == 1);
    uint8_t tx[2] = { reg|CC1101_READ, 0};
    CC1101Status rx[2] = { 0 };

    hal_gpio_write(device->chip_select, false);
    while(hal_gpio_read(device->bus->miso));
    furi_hal_spi_bus_trx(device->bus, tx, (uint8_t*)rx, 2, CC1101_TIMEOUT);
    hal_gpio_write(device->chip_select, true);

    assert((rx[0].CHIP_RDYn) == 0);
    *data = *(uint8_t*)&rx[1];
    return rx[0];
}

uint8_t cc1101_get_partnumber(const FuriHalSpiDevice* device) {
    uint8_t partnumber=0;
    cc1101_read_reg(device, CC1101_STATUS_PARTNUM|CC1101_BURST, &partnumber);
    return partnumber;
}

uint8_t cc1101_get_version(const FuriHalSpiDevice* device) {
    uint8_t version=0;
    cc1101_read_reg(device, CC1101_STATUS_VERSION|CC1101_BURST, &version);
    return version;
}

uint8_t cc1101_get_rssi(const FuriHalSpiDevice* device) {
    uint8_t rssi=0;
    cc1101_read_reg(device, CC1101_STATUS_RSSI|CC1101_BURST, &rssi);
    return rssi;
}

void cc1101_reset(const FuriHalSpiDevice* device) {
    hal_gpio_write(device->chip_select, false);
    delay_us(1000);
    hal_gpio_write(device->chip_select, true);
    delay_us(1000);
    cc1101_strobe(device, CC1101_STROBE_SRES);
}

CC1101Status cc1101_get_status(const FuriHalSpiDevice* device) {
    return cc1101_strobe(device, CC1101_STROBE_SNOP);
}

void cc1101_shutdown(const FuriHalSpiDevice* device) {
    cc1101_strobe(device, CC1101_STROBE_SPWD);
}

void cc1101_calibrate(const FuriHalSpiDevice* device) {
    cc1101_strobe(device, CC1101_STROBE_SCAL);
}

void cc1101_switch_to_idle(const FuriHalSpiDevice* device) {
    cc1101_strobe(device, CC1101_STROBE_SIDLE);
}

void cc1101_switch_to_rx(const FuriHalSpiDevice* device) {
    cc1101_strobe(device, CC1101_STROBE_SRX);
}

void cc1101_switch_to_tx(const FuriHalSpiDevice* device) {
    cc1101_strobe(device, CC1101_STROBE_STX);
}

void cc1101_flush_rx(const FuriHalSpiDevice* device) {
    cc1101_strobe(device, CC1101_STROBE_SFRX);
}

void cc1101_flush_tx(const FuriHalSpiDevice* device) {
    cc1101_strobe(device, CC1101_STROBE_SFTX);
}

uint32_t cc1101_set_frequency(const FuriHalSpiDevice* device, uint32_t value) {
    uint64_t real_value = (uint64_t)value * CC1101_FDIV / CC1101_QUARTZ;

    // Sanity check
    assert((real_value & CC1101_FMASK) == real_value);

    cc1101_write_reg(device, CC1101_FREQ2, (real_value >> 16) & 0xFF);
    cc1101_write_reg(device, CC1101_FREQ1, (real_value >> 8 ) & 0xFF);
    cc1101_write_reg(device, CC1101_FREQ0, (real_value >> 0 ) & 0xFF);

    uint64_t real_frequency = real_value * CC1101_QUARTZ / CC1101_FDIV;

    return (uint32_t)real_frequency;
}

uint32_t cc1101_set_intermediate_frequency(const FuriHalSpiDevice* device, uint32_t value) {
    uint64_t real_value = value * CC1101_IFDIV / CC1101_QUARTZ;
    assert((real_value & 0xFF) == real_value);

    cc1101_write_reg(device, CC1101_FSCTRL0, (real_value >> 0 ) & 0xFF);

    uint64_t real_frequency = real_value * CC1101_QUARTZ / CC1101_IFDIV;

    return (uint32_t)real_frequency;
}

void cc1101_set_pa_table(const FuriHalSpiDevice* device, const uint8_t value[8]) {
    uint8_t tx[9] = { CC1101_PATABLE | CC1101_BURST };
    CC1101Status rx[9] = { 0 };

    memcpy(&tx[1], &value[0], 8);

    hal_gpio_write(device->chip_select, false);
    while(hal_gpio_read(device->bus->miso));
    furi_hal_spi_bus_trx(device->bus, tx, (uint8_t*)rx, sizeof(rx), CC1101_TIMEOUT);
    hal_gpio_write(device->chip_select, true);

    assert((rx[0].CHIP_RDYn|rx[8].CHIP_RDYn) == 0);
}

uint8_t cc1101_write_fifo(const FuriHalSpiDevice* device, const uint8_t* data, uint8_t size) {
    uint8_t buff_tx[64];
    uint8_t buff_rx[64];
    buff_tx[0] = CC1101_FIFO | CC1101_BURST;
    memcpy(&buff_tx[1], data, size);

    // Start transaction
    hal_gpio_write(device->chip_select, false);
    // Wait IC to become ready
    while(hal_gpio_read(device->bus->miso));
    // Tell IC what we want
    furi_hal_spi_bus_trx(device->bus, buff_tx, (uint8_t*) buff_rx, size + 1, CC1101_TIMEOUT);

    // Finish transaction
    hal_gpio_write(device->chip_select, true);

    return size;
}

uint8_t cc1101_read_fifo(const FuriHalSpiDevice* device, uint8_t* data, uint8_t* size) {
    uint8_t buff_tx[64];
    buff_tx[0] = CC1101_FIFO | CC1101_READ | CC1101_BURST;
    uint8_t buff_rx[2];

    // Start transaction
    hal_gpio_write(device->chip_select, false);
    // Wait IC to become ready
    while(hal_gpio_read(device->bus->miso));

    // First byte - packet length
    furi_hal_spi_bus_trx(device->bus, buff_tx, buff_rx, 2, CC1101_TIMEOUT);
    *size = buff_rx[2];
    furi_hal_spi_bus_trx(device->bus, &buff_tx[1], data, *size, CC1101_TIMEOUT);
    cc1101_flush_rx(device);

    hal_gpio_write(device->chip_select, true);
    return *size;
}
