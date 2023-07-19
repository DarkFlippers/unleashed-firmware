#include "cc1101.h"
#include <assert.h>
#include <string.h>
#include <furi_hal_cortex.h>

static bool cc1101_spi_trx(FuriHalSpiBusHandle* handle, uint8_t* tx, uint8_t* rx, uint8_t size) {
    FuriHalCortexTimer timer = furi_hal_cortex_timer_get(CC1101_TIMEOUT * 1000);

    while(furi_hal_gpio_read(handle->miso)) {
        if(furi_hal_cortex_timer_is_expired(timer)) {
            //timeout
            return false;
        }
    }
    if(!furi_hal_spi_bus_trx(handle, tx, rx, size, CC1101_TIMEOUT)) return false;
    return true;
}

CC1101Status cc1101_strobe(FuriHalSpiBusHandle* handle, uint8_t strobe) {
    uint8_t tx[1] = {strobe};
    CC1101Status rx[1] = {0};
    rx[0].CHIP_RDYn = 1;

    cc1101_spi_trx(handle, tx, (uint8_t*)rx, 1);

    assert(rx[0].CHIP_RDYn == 0);
    return rx[0];
}

CC1101Status cc1101_write_reg(FuriHalSpiBusHandle* handle, uint8_t reg, uint8_t data) {
    uint8_t tx[2] = {reg, data};
    CC1101Status rx[2] = {0};
    rx[0].CHIP_RDYn = 1;
    rx[1].CHIP_RDYn = 1;

    cc1101_spi_trx(handle, tx, (uint8_t*)rx, 2);

    assert((rx[0].CHIP_RDYn | rx[1].CHIP_RDYn) == 0);
    return rx[1];
}

CC1101Status cc1101_read_reg(FuriHalSpiBusHandle* handle, uint8_t reg, uint8_t* data) {
    assert(sizeof(CC1101Status) == 1);
    uint8_t tx[2] = {reg | CC1101_READ, 0};
    CC1101Status rx[2] = {0};
    rx[0].CHIP_RDYn = 1;

    cc1101_spi_trx(handle, tx, (uint8_t*)rx, 2);

    assert((rx[0].CHIP_RDYn) == 0);
    *data = *(uint8_t*)&rx[1];
    return rx[0];
}

uint8_t cc1101_get_partnumber(FuriHalSpiBusHandle* handle) {
    uint8_t partnumber = 0;
    cc1101_read_reg(handle, CC1101_STATUS_PARTNUM | CC1101_BURST, &partnumber);
    return partnumber;
}

uint8_t cc1101_get_version(FuriHalSpiBusHandle* handle) {
    uint8_t version = 0;
    cc1101_read_reg(handle, CC1101_STATUS_VERSION | CC1101_BURST, &version);
    return version;
}

uint8_t cc1101_get_rssi(FuriHalSpiBusHandle* handle) {
    uint8_t rssi = 0;
    cc1101_read_reg(handle, CC1101_STATUS_RSSI | CC1101_BURST, &rssi);
    return rssi;
}

CC1101Status cc1101_reset(FuriHalSpiBusHandle* handle) {
    return cc1101_strobe(handle, CC1101_STROBE_SRES);
}

CC1101Status cc1101_get_status(FuriHalSpiBusHandle* handle) {
    return cc1101_strobe(handle, CC1101_STROBE_SNOP);
}

CC1101Status cc1101_shutdown(FuriHalSpiBusHandle* handle) {
    return cc1101_strobe(handle, CC1101_STROBE_SPWD);
}

CC1101Status cc1101_calibrate(FuriHalSpiBusHandle* handle) {
    return cc1101_strobe(handle, CC1101_STROBE_SCAL);
}

CC1101Status cc1101_switch_to_idle(FuriHalSpiBusHandle* handle) {
    return cc1101_strobe(handle, CC1101_STROBE_SIDLE);
}

CC1101Status cc1101_switch_to_rx(FuriHalSpiBusHandle* handle) {
    return cc1101_strobe(handle, CC1101_STROBE_SRX);
}

CC1101Status cc1101_switch_to_tx(FuriHalSpiBusHandle* handle) {
    return cc1101_strobe(handle, CC1101_STROBE_STX);
}

CC1101Status cc1101_flush_rx(FuriHalSpiBusHandle* handle) {
    return cc1101_strobe(handle, CC1101_STROBE_SFRX);
}

CC1101Status cc1101_flush_tx(FuriHalSpiBusHandle* handle) {
    return cc1101_strobe(handle, CC1101_STROBE_SFTX);
}

uint32_t cc1101_set_frequency(FuriHalSpiBusHandle* handle, uint32_t value) {
    uint64_t real_value = (uint64_t)value * CC1101_FDIV / CC1101_QUARTZ;

    // Sanity check
    assert((real_value & CC1101_FMASK) == real_value);

    cc1101_write_reg(handle, CC1101_FREQ2, (real_value >> 16) & 0xFF);
    cc1101_write_reg(handle, CC1101_FREQ1, (real_value >> 8) & 0xFF);
    cc1101_write_reg(handle, CC1101_FREQ0, (real_value >> 0) & 0xFF);

    uint64_t real_frequency = real_value * CC1101_QUARTZ / CC1101_FDIV;

    return (uint32_t)real_frequency;
}

uint32_t cc1101_set_intermediate_frequency(FuriHalSpiBusHandle* handle, uint32_t value) {
    uint64_t real_value = value * CC1101_IFDIV / CC1101_QUARTZ;
    assert((real_value & 0xFF) == real_value);

    cc1101_write_reg(handle, CC1101_FSCTRL0, (real_value >> 0) & 0xFF);

    uint64_t real_frequency = real_value * CC1101_QUARTZ / CC1101_IFDIV;

    return (uint32_t)real_frequency;
}

void cc1101_set_pa_table(FuriHalSpiBusHandle* handle, const uint8_t value[8]) {
    uint8_t tx[9] = {CC1101_PATABLE | CC1101_BURST}; //-V1009
    CC1101Status rx[9] = {0};
    rx[0].CHIP_RDYn = 1;
    rx[8].CHIP_RDYn = 1;

    memcpy(&tx[1], &value[0], 8);

    cc1101_spi_trx(handle, tx, (uint8_t*)rx, sizeof(rx));

    assert((rx[0].CHIP_RDYn | rx[8].CHIP_RDYn) == 0);
}

uint8_t cc1101_write_fifo(FuriHalSpiBusHandle* handle, const uint8_t* data, uint8_t size) {
    uint8_t buff_tx[64];
    uint8_t buff_rx[64];
    buff_tx[0] = CC1101_FIFO | CC1101_BURST;
    memcpy(&buff_tx[1], data, size);

    cc1101_spi_trx(handle, buff_tx, (uint8_t*)buff_rx, size + 1);

    return size;
}

uint8_t cc1101_read_fifo(FuriHalSpiBusHandle* handle, uint8_t* data, uint8_t* size) {
    uint8_t buff_trx[2];
    buff_trx[0] = CC1101_FIFO | CC1101_READ | CC1101_BURST;

    cc1101_spi_trx(handle, buff_trx, buff_trx, 2);

    // Check that the packet is placed in the receive buffer
    if(buff_trx[1] > 64) {
        *size = 64;
    } else {
        *size = buff_trx[1];
    }
    furi_hal_spi_bus_trx(handle, NULL, data, *size, CC1101_TIMEOUT);

    return *size;
}