#pragma once

#include <furi-hal-gpio.h>
#include <stm32wbxx_ll_spi.h>
#include <cmsis_os2.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const LL_SPI_InitTypeDef furi_hal_spi_config_nfc;
extern const LL_SPI_InitTypeDef furi_hal_spi_config_subghz;
extern const LL_SPI_InitTypeDef furi_hal_spi_config_display;
extern const LL_SPI_InitTypeDef furi_hal_spi_config_sd_fast;
extern const LL_SPI_InitTypeDef furi_hal_spi_config_sd_slow;

/** FURI HAL SPI BUS handler
 * Structure content may change at some point
 */
typedef struct {
    const SPI_TypeDef* spi;
    const osMutexId_t* mutex;
    const GpioPin* miso;
    const GpioPin* mosi;
    const GpioPin* clk;
} FuriHalSpiBus;

/** FURI HAL SPI Device handler
 * Structure content may change at some point
 */
typedef struct {
    const FuriHalSpiBus* bus;
    const LL_SPI_InitTypeDef* config;
    const GpioPin* chip_select;
} FuriHalSpiDevice;

/** FURI HAL SPI Standard Device IDs */
typedef enum {
    FuriHalSpiDeviceIdSubGhz,        /** SubGhz: CC1101, non-standard SPI usage */
    FuriHalSpiDeviceIdDisplay,       /** Display: ERC12864, only have MOSI */
    FuriHalSpiDeviceIdSdCardFast,    /** SDCARD: fast mode, after initialization */
    FuriHalSpiDeviceIdSdCardSlow,    /** SDCARD: slow mode, before initialization */
    FuriHalSpiDeviceIdNfc,           /** NFC: ST25R3916, pretty standard, but RFAL makes it complex */

    FuriHalSpiDeviceIdMax,           /** Service Value, do not use */
} FuriHalSpiDeviceId;

/** Furi Hal Spi Bus R
 * CC1101, Nfc
 */
extern const FuriHalSpiBus spi_r;

/** Furi Hal Spi Bus D
 * Display, SdCard
 */
extern const FuriHalSpiBus spi_d;

/** Furi Hal Spi devices */
extern const FuriHalSpiDevice furi_hal_spi_devices[FuriHalSpiDeviceIdMax];

#ifdef __cplusplus
}
#endif