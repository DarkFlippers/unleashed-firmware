#include "main.h"
#include <furi-hal.h>
#include <furi.h>

#define SD_DUMMY_BYTE 0xFF

const uint32_t SpiTimeout = 1000;
uint8_t SD_IO_WriteByte(uint8_t Data);

/******************************************************************************
                            BUS OPERATIONS
 *******************************************************************************/

/**
 * @brief  SPI Write byte(s) to device
 * @param  DataIn: Pointer to data buffer to write
 * @param  DataOut: Pointer to data buffer for read data
 * @param  DataLength: number of bytes to write
 * @retval None
 */
static void SPIx_WriteReadData(const uint8_t* DataIn, uint8_t* DataOut, uint16_t DataLength) {
    furi_check(furi_hal_spi_bus_trx(furi_hal_sd_spi_handle, (uint8_t*)DataIn, DataOut, DataLength, SpiTimeout));
}

/**
 * @brief  SPI Write a byte to device
 * @param  Value: value to be written
 * @retval None
 */
__attribute__((unused)) static void SPIx_Write(uint8_t Value) {
    furi_check(furi_hal_spi_bus_tx(furi_hal_sd_spi_handle, (uint8_t*)&Value, 1, SpiTimeout));
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
    hal_gpio_write(furi_hal_sd_spi_handle->cs, true);
    delay_us(10);

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
    /* Some SD Cards are prone to fail if CLK-ed too soon after CS transition. Worst case found: 8us */
    if(val == 1) {
        delay_us(10); // Exit guard time for some SD cards
        hal_gpio_write(furi_hal_sd_spi_handle->cs, true);
    } else {
        hal_gpio_write(furi_hal_sd_spi_handle->cs, false);
        delay_us(10); // Entry guard time for some SD cards
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
