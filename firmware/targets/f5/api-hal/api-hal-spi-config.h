#pragma once

#include <api-hal-gpio.h>
#include <cmsis_os2.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const SPI_InitTypeDef api_hal_spi_config_nfc;
extern const SPI_InitTypeDef api_hal_spi_config_subghz;
extern const SPI_InitTypeDef api_hal_spi_config_display;
extern const SPI_InitTypeDef api_hal_spi_config_sd_fast;
extern const SPI_InitTypeDef api_hal_spi_config_sd_slow;

/** API HAL SPI BUS handler
 * Structure content may change at some point
 */
typedef struct {
    const SPI_HandleTypeDef* spi;
    const osMutexId_t* mutex;
    const GpioPin* miso;
    const GpioPin* mosi;
    const GpioPin* clk;
} ApiHalSpiBus;

/** API HAL SPI Device handler
 * Structure content may change at some point
 */
typedef struct {
    const ApiHalSpiBus* bus;
    const SPI_InitTypeDef* config;
    const GpioPin* chip_select;
} ApiHalSpiDevice;

/** API HAL SPI Standard Device IDs */
typedef enum {
    ApiHalSpiDeviceIdSubGhz,        /** SubGhz: CC1101, non-standard SPI usage */
    ApiHalSpiDeviceIdDisplay,       /** Display: ERC12864, only have MOSI */
    ApiHalSpiDeviceIdSdCardFast,    /** SDCARD: fast mode, after initialization */
    ApiHalSpiDeviceIdSdCardSlow,    /** SDCARD: slow mode, before initialization */
    ApiHalSpiDeviceIdNfc,           /** NFC: ST25R3916, pretty standard, but RFAL makes it complex */

    ApiHalSpiDeviceIdMax,           /** Service Value, do not use */
} ApiHalSpiDeviceId;

/** Api Hal Spi Bus R
 * CC1101, Nfc
 */
extern const ApiHalSpiBus spi_r;

/** Api Hal Spi Bus D
 * Display, SdCard
 */
extern const ApiHalSpiBus spi_d;

/** Api Hal Spi devices */
extern const ApiHalSpiDevice api_hal_spi_devices[ApiHalSpiDeviceIdMax];

typedef struct {
    const ApiHalSpiBus* bus;
    const SPI_InitTypeDef config;
} SPIDevice;

#ifdef __cplusplus
}
#endif