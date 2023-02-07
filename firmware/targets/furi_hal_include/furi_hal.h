/**
 * @file furi_hal.h
 * Furi HAL API
 */

#pragma once

#ifdef __cplusplus
template <unsigned int N>
struct STOP_EXTERNING_ME {};
#endif

#include <furi_hal_cortex.h>
#include <furi_hal_clock.h>
#include <furi_hal_crypto.h>
#include <furi_hal_console.h>
#include <furi_hal_debug.h>
#include <furi_hal_os.h>
#include <furi_hal_sd.h>
#include <furi_hal_i2c.h>
#include <furi_hal_region.h>
#include <furi_hal_resources.h>
#include <furi_hal_rtc.h>
#include <furi_hal_speaker.h>
#include <furi_hal_gpio.h>
#include <furi_hal_light.h>
#include <furi_hal_power.h>
#include <furi_hal_interrupt.h>
#include <furi_hal_version.h>
#include <furi_hal_bt.h>
#include <furi_hal_spi.h>
#include <furi_hal_flash.h>
#include <furi_hal_vibro.h>
#include <furi_hal_usb.h>
#include <furi_hal_usb_hid.h>
#include <furi_hal_compress.h>
#include <furi_hal_uart.h>
#include <furi_hal_info.h>
#include <furi_hal_random.h>
#include <furi_hal_target_hw.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Early FuriHal init, only essential subsystems */
void furi_hal_init_early();

/** Early FuriHal deinit */
void furi_hal_deinit_early();

/** Init FuriHal */
void furi_hal_init();

/** Transfer execution to address
 *
 * @param[in]  address  pointer to new executable
 */
void furi_hal_switch(void* address);

#ifdef __cplusplus
}
#endif
