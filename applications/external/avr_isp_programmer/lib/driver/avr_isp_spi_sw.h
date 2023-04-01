#pragma once

#include <furi_hal.h>

typedef enum {
    AvrIspSpiSwSpeed1Mhz = 0,
    AvrIspSpiSwSpeed400Khz = 1,
    AvrIspSpiSwSpeed250Khz = 2,
    AvrIspSpiSwSpeed125Khz = 4,
    AvrIspSpiSwSpeed60Khz = 8,
    AvrIspSpiSwSpeed40Khz = 12,
    AvrIspSpiSwSpeed20Khz = 24,
    AvrIspSpiSwSpeed10Khz = 48,
    AvrIspSpiSwSpeed5Khz = 96,
    AvrIspSpiSwSpeed1Khz = 480,
} AvrIspSpiSwSpeed;

typedef struct AvrIspSpiSw AvrIspSpiSw;

AvrIspSpiSw* avr_isp_spi_sw_init(AvrIspSpiSwSpeed speed);
void avr_isp_spi_sw_free(AvrIspSpiSw* instance);
uint8_t avr_isp_spi_sw_txrx(AvrIspSpiSw* instance, uint8_t data);
void avr_isp_spi_sw_res_set(AvrIspSpiSw* instance, bool state);
void avr_isp_spi_sw_sck_set(AvrIspSpiSw* instance, bool state);