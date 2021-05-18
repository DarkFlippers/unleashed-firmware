#include <api-hal-spi-config.h>
#include <api-hal-resources.h>

extern SPI_HandleTypeDef SPI_R;
extern SPI_HandleTypeDef SPI_D;

const SPI_InitTypeDef api_hal_spi_config_nfc = {
    .Mode = SPI_MODE_MASTER,
    .Direction = SPI_DIRECTION_2LINES,
    .DataSize = SPI_DATASIZE_8BIT,
    .CLKPolarity = SPI_POLARITY_LOW,
    .CLKPhase = SPI_PHASE_2EDGE,
    .NSS = SPI_NSS_SOFT,
    .BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8,
    .FirstBit = SPI_FIRSTBIT_MSB,
    .TIMode = SPI_TIMODE_DISABLE,
    .CRCCalculation = SPI_CRCCALCULATION_DISABLE,
    .CRCPolynomial = 7,
    .CRCLength = SPI_CRC_LENGTH_DATASIZE,
    .NSSPMode = SPI_NSS_PULSE_DISABLE,
};

const SPI_InitTypeDef api_hal_spi_config_subghz = {
    .Mode = SPI_MODE_MASTER,
    .Direction = SPI_DIRECTION_2LINES,
    .DataSize = SPI_DATASIZE_8BIT,
    .CLKPolarity = SPI_POLARITY_LOW,
    .CLKPhase = SPI_PHASE_1EDGE,
    .NSS = SPI_NSS_SOFT,
    .BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8,
    .FirstBit = SPI_FIRSTBIT_MSB,
    .TIMode = SPI_TIMODE_DISABLE,
    .CRCCalculation = SPI_CRCCALCULATION_DISABLE,
    .CRCPolynomial = 7,
    .CRCLength = SPI_CRC_LENGTH_DATASIZE,
    .NSSPMode = SPI_NSS_PULSE_DISABLE,
};

const SPI_InitTypeDef api_hal_spi_config_display = {
    .Mode = SPI_MODE_MASTER,
    .Direction = SPI_DIRECTION_2LINES,
    .DataSize = SPI_DATASIZE_8BIT,
    .CLKPolarity = SPI_POLARITY_LOW,
    .CLKPhase = SPI_PHASE_1EDGE,
    .NSS = SPI_NSS_SOFT,
    .BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16,
    .FirstBit = SPI_FIRSTBIT_MSB,
    .TIMode = SPI_TIMODE_DISABLE,
    .CRCCalculation = SPI_CRCCALCULATION_DISABLE,
    .CRCPolynomial = 7,
    .CRCLength = SPI_CRC_LENGTH_DATASIZE,
    .NSSPMode = SPI_NSS_PULSE_ENABLE,
};

osMutexId_t spi_mutex_d = NULL;
osMutexId_t spi_mutex_r = NULL;

const ApiHalSpiBus spi_r = {
    .spi=&SPI_R,
    .mutex=&spi_mutex_r,
    .miso=&gpio_spi_r_miso,
    .mosi=&gpio_spi_r_mosi,
    .clk=&gpio_spi_r_sck,
};

const ApiHalSpiBus spi_d = {
    .spi=&SPI_D,
    .mutex=&spi_mutex_d,
    .miso=&gpio_spi_d_miso,
    .mosi=&gpio_spi_d_mosi,
    .clk=&gpio_spi_d_sck,
};

const ApiHalSpiDevice api_hal_spi_devices[ApiHalSpiDeviceIdMax] = {
    { .bus=&spi_r, .config=&api_hal_spi_config_subghz, .chip_select=&gpio_subghz_cs, },
    { .bus=&spi_d, .config=&api_hal_spi_config_display, .chip_select=&gpio_display_cs, },
    { .bus=&spi_d, .config=NULL, .chip_select=&gpio_sdcard_cs, },
    { .bus=&spi_r, .config=&api_hal_spi_config_nfc, .chip_select=&gpio_nfc_cs },
};


/**
 * SD Card in fast mode (after init)
 */
const SPIDevice sd_fast_spi = {
    .bus= &spi_d,
    .config = {
        .Mode = SPI_MODE_MASTER,
        .Direction = SPI_DIRECTION_2LINES,
        .DataSize = SPI_DATASIZE_8BIT,
        .CLKPolarity = SPI_POLARITY_LOW,
        .CLKPhase = SPI_PHASE_1EDGE,
        .NSS = SPI_NSS_SOFT,
        .BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2,
        .FirstBit = SPI_FIRSTBIT_MSB,
        .TIMode = SPI_TIMODE_DISABLE,
        .CRCCalculation = SPI_CRCCALCULATION_DISABLE,
        .CRCPolynomial = 7,
        .CRCLength = SPI_CRC_LENGTH_DATASIZE,
        .NSSPMode = SPI_NSS_PULSE_ENABLE,
    }};

/**
 * SD Card in slow mode (before init)
 */
const SPIDevice sd_slow_spi = {
    .bus= &spi_d,
    .config = {
        .Mode = SPI_MODE_MASTER,
        .Direction = SPI_DIRECTION_2LINES,
        .DataSize = SPI_DATASIZE_8BIT,
        .CLKPolarity = SPI_POLARITY_LOW,
        .CLKPhase = SPI_PHASE_1EDGE,
        .NSS = SPI_NSS_SOFT,
        .BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32,
        .FirstBit = SPI_FIRSTBIT_MSB,
        .TIMode = SPI_TIMODE_DISABLE,
        .CRCCalculation = SPI_CRCCALCULATION_DISABLE,
        .CRCPolynomial = 7,
        .CRCLength = SPI_CRC_LENGTH_DATASIZE,
        .NSSPMode = SPI_NSS_PULSE_ENABLE,
    }};
