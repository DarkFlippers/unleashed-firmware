#pragma once

#include <furi_hal_spi_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Preset for ST25R916 */
extern const LL_SPI_InitTypeDef furi_hal_spi_preset_2edge_low_8m;

/** Preset for CC1101 */
extern const LL_SPI_InitTypeDef furi_hal_spi_preset_1edge_low_8m;

/** Preset for ST7567 (Display) */
extern const LL_SPI_InitTypeDef furi_hal_spi_preset_1edge_low_4m;

/** Preset for SdCard in fast mode */
extern const LL_SPI_InitTypeDef furi_hal_spi_preset_1edge_low_16m;

/** Preset for SdCard in slow mode */
extern const LL_SPI_InitTypeDef furi_hal_spi_preset_1edge_low_2m;

/** Furi Hal Spi Bus R (Radio: CC1101, Nfc, External)*/
extern FuriHalSpiBus furi_hal_spi_bus_r;

/** Furi Hal Spi Bus D (Display, SdCard) */
extern FuriHalSpiBus furi_hal_spi_bus_d;

/** CC1101 on `furi_hal_spi_bus_r` */
extern FuriHalSpiBusHandle furi_hal_spi_bus_handle_subghz;

/** ST25R3916 on `furi_hal_spi_bus_r` */
extern FuriHalSpiBusHandle furi_hal_spi_bus_handle_nfc;

/** External on `furi_hal_spi_bus_r`
 * Preset: `furi_hal_spi_preset_1edge_low_2m`
 * 
 * miso: pa6
 * mosi: pa7
 * sck: pb3
 * cs:  pa4 (software controlled)
 * 
 * @warning not initialized by default, call `furi_hal_spi_bus_handle_init` to initialize
 * Bus pins are floating on inactive state, CS high after initialization
 * 
 */
extern FuriHalSpiBusHandle furi_hal_spi_bus_handle_external;

/** ST7567(Display) on `furi_hal_spi_bus_d` */
extern FuriHalSpiBusHandle furi_hal_spi_bus_handle_display;

/** SdCard in fast mode on `furi_hal_spi_bus_d` */
extern FuriHalSpiBusHandle furi_hal_spi_bus_handle_sd_fast;

/** SdCard in slow mode on `furi_hal_spi_bus_d` */
extern FuriHalSpiBusHandle furi_hal_spi_bus_handle_sd_slow;

#ifdef __cplusplus
}
#endif
