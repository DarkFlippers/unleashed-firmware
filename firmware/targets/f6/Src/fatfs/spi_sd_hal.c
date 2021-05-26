#include "main.h"
#include "api-hal-spi.h"

#define SD_DUMMY_BYTE 0xFF

const uint32_t SpiTimeout = 1000;
uint8_t SD_IO_WriteByte(uint8_t Data);
static const ApiHalSpiDevice* sd_spi_dev = &api_hal_spi_devices[ApiHalSpiDeviceIdSdCardFast];


/******************************************************************************
                            BUS OPERATIONS
 *******************************************************************************/

/**
 * @brief  SPI error treatment function
 * @retval None
 */
static void SPIx_Error(void) {
    /* Re-Initiaize the SPI communication BUS */
    api_hal_spi_bus_reset(sd_spi_dev->bus);
}

/**
 * @brief  SPI Write byte(s) to device
 * @param  DataIn: Pointer to data buffer to write
 * @param  DataOut: Pointer to data buffer for read data
 * @param  DataLength: number of bytes to write
 * @retval None
 */
static void SPIx_WriteReadData(const uint8_t* DataIn, uint8_t* DataOut, uint16_t DataLength) {
    bool status = api_hal_spi_bus_trx(sd_spi_dev->bus, (uint8_t*)DataIn, DataOut, DataLength, SpiTimeout);

    /* Check the communication status */
    if(!status) {
        /* Execute user timeout callback */
        SPIx_Error();
    }
}

/**
 * @brief  SPI Write a byte to device
 * @param  Value: value to be written
 * @retval None
 */
__attribute__((unused)) static void SPIx_Write(uint8_t Value) {
    uint8_t data;

    bool status = api_hal_spi_bus_trx(sd_spi_dev->bus, (uint8_t*)&Value, &data, 1, SpiTimeout);

    /* Check the communication status */
    if(!status) {
        /* Execute user timeout callback */
        SPIx_Error();
    }
}

/******************************************************************************
                            LINK OPERATIONS
 *******************************************************************************/

/********************************* LINK SD ************************************/
/**
 * @brief  Initialize the SD Card and put it into StandBy State (Ready for
 *         data transfer).
 * @retval None
 */
void SD_IO_Init(void) {
    uint8_t counter = 0;

    /* SD chip select high */
    hal_gpio_write(sd_spi_dev->chip_select, true);

    /* Send dummy byte 0xFF, 10 times with CS high */
    /* Rise CS and MOSI for 80 clocks cycles */
    for(counter = 0; counter <= 200; counter++) {
        /* Send dummy byte 0xFF */
        SD_IO_WriteByte(SD_DUMMY_BYTE);
    }
}

/**
 * @brief  Set SD interface Chip Select state
 * @param  val: 0 (low) or 1 (high) state
 * @retval None
 */
void SD_IO_CSState(uint8_t val) {
    if(val == 1) {
        hal_gpio_write(sd_spi_dev->chip_select, true);
    } else {
        hal_gpio_write(sd_spi_dev->chip_select, false);
    }
}

/**
 * @brief  Write byte(s) on the SD
 * @param  DataIn: Pointer to data buffer to write
 * @param  DataOut: Pointer to data buffer for read data
 * @param  DataLength: number of bytes to write
 * @retval None
 */
void SD_IO_WriteReadData(const uint8_t* DataIn, uint8_t* DataOut, uint16_t DataLength) {
    /* Send the byte */
    SPIx_WriteReadData(DataIn, DataOut, DataLength);
}

/**
 * @brief  Write a byte on the SD.
 * @param  Data: byte to send.
 * @retval Data written
 */
uint8_t SD_IO_WriteByte(uint8_t Data) {
    uint8_t tmp;

    /* Send the byte */
    SPIx_WriteReadData(&Data, &tmp, 1);
    return tmp;
}
