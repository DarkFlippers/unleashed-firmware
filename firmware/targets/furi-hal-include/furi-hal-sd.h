/**
 * @file furi-hal-sd.h
 * SD Card HAL API
 */

#include <stdint.h>
#include <stdbool.h>
#include <furi-hal-spi-types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Init SD card detect
 */
void hal_sd_detect_init(void);

/** Set SD card detect pin to low
 */
void hal_sd_detect_set_low(void);

/** Get SD card status
 *
 * @return     true if SD card present, false if SD card not present
 */
bool hal_sd_detect(void);

/** Pointer to currently used SPI Handle */
extern FuriHalSpiBusHandle* furi_hal_sd_spi_handle;

#ifdef __cplusplus
}
#endif
