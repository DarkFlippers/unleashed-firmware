#include <furi-hal-spi-config.h>
#include <furi-hal-resources.h>

#define SPI_R SPI1
#define SPI_D SPI2

const LL_SPI_InitTypeDef furi_hal_spi_config_nfc = {
    .Mode = LL_SPI_MODE_MASTER,
    .TransferDirection = LL_SPI_FULL_DUPLEX,
    .DataWidth = LL_SPI_DATAWIDTH_8BIT,
    .ClockPolarity = LL_SPI_POLARITY_LOW,
    .ClockPhase = LL_SPI_PHASE_2EDGE,
    .NSS = LL_SPI_NSS_SOFT,
    .BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV8,
    .BitOrder = LL_SPI_MSB_FIRST,
    .CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE,
    .CRCPoly = 7,
};

const LL_SPI_InitTypeDef furi_hal_spi_config_subghz = {
    .Mode = LL_SPI_MODE_MASTER,
    .TransferDirection = LL_SPI_FULL_DUPLEX,
    .DataWidth = LL_SPI_DATAWIDTH_8BIT,
    .ClockPolarity = LL_SPI_POLARITY_LOW,
    .ClockPhase = LL_SPI_PHASE_1EDGE,
    .NSS = LL_SPI_NSS_SOFT,
    .BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV8,
    .BitOrder = LL_SPI_MSB_FIRST,
    .CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE,
    .CRCPoly = 7,
};

const LL_SPI_InitTypeDef furi_hal_spi_config_display = {
    .Mode = LL_SPI_MODE_MASTER,
    .TransferDirection = LL_SPI_FULL_DUPLEX,
    .DataWidth = LL_SPI_DATAWIDTH_8BIT,
    .ClockPolarity = LL_SPI_POLARITY_LOW,
    .ClockPhase = LL_SPI_PHASE_1EDGE,
    .NSS = LL_SPI_NSS_SOFT,
    .BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV16,
    .BitOrder = LL_SPI_MSB_FIRST,
    .CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE,
    .CRCPoly = 7,
};

/**
 * SD Card in fast mode (after init)
 */
const LL_SPI_InitTypeDef furi_hal_spi_config_sd_fast = {
    .Mode = LL_SPI_MODE_MASTER,
    .TransferDirection = LL_SPI_FULL_DUPLEX,
    .DataWidth = LL_SPI_DATAWIDTH_8BIT,
    .ClockPolarity = LL_SPI_POLARITY_LOW,
    .ClockPhase = LL_SPI_PHASE_1EDGE,
    .NSS = LL_SPI_NSS_SOFT,
    .BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV2,
    .BitOrder = LL_SPI_MSB_FIRST,
    .CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE,
    .CRCPoly = 7,
};

/**
 * SD Card in slow mode (before init)
 */
const LL_SPI_InitTypeDef furi_hal_spi_config_sd_slow = {
    .Mode = LL_SPI_MODE_MASTER,
    .TransferDirection = LL_SPI_FULL_DUPLEX,
    .DataWidth = LL_SPI_DATAWIDTH_8BIT,
    .ClockPolarity = LL_SPI_POLARITY_LOW,
    .ClockPhase = LL_SPI_PHASE_1EDGE,
    .NSS = LL_SPI_NSS_SOFT,
    .BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV32,
    .BitOrder = LL_SPI_MSB_FIRST,
    .CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE,
    .CRCPoly = 7,
};

osMutexId_t spi_mutex_d = NULL;
osMutexId_t spi_mutex_r = NULL;

const FuriHalSpiBus spi_r = {
    .spi=SPI_R,
    .mutex=&spi_mutex_r,
    .miso=&gpio_spi_r_miso,
    .mosi=&gpio_spi_r_mosi,
    .clk=&gpio_spi_r_sck,
};

const FuriHalSpiBus spi_d = {
    .spi=SPI_D,
    .mutex=&spi_mutex_d,
    .miso=&gpio_spi_d_miso,
    .mosi=&gpio_spi_d_mosi,
    .clk=&gpio_spi_d_sck,
};

const FuriHalSpiDevice furi_hal_spi_devices[FuriHalSpiDeviceIdMax] = {
    { .bus=&spi_r, .config=&furi_hal_spi_config_subghz, .chip_select=&gpio_subghz_cs, },
    { .bus=&spi_d, .config=&furi_hal_spi_config_display, .chip_select=&gpio_display_cs, },
    { .bus=&spi_d, .config=&furi_hal_spi_config_sd_fast, .chip_select=&gpio_sdcard_cs, },
    { .bus=&spi_d, .config=&furi_hal_spi_config_sd_slow, .chip_select=&gpio_sdcard_cs, },
    { .bus=&spi_r, .config=&furi_hal_spi_config_nfc, .chip_select=&gpio_nfc_cs },
};
