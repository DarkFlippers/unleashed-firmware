/**
 * @file furi_hal.h
 * Furi HAL API
 */

#pragma once

#ifdef __cplusplus
template <unsigned int N> struct STOP_EXTERNING_ME {};
#endif

#include "furi_hal_bootloader.h"
#include "furi_hal_clock.h"
#include "furi_hal_crypto.h"
#include "furi_hal_console.h"
#include "furi_hal_os.h"
#include "furi_hal_sd.h"
#include "furi_hal_i2c.h"
#include "furi_hal_resources.h"
#include "furi_hal_rtc.h"
#include "furi_hal_speaker.h"
#include "furi_hal_gpio.h"
#include "furi_hal_light.h"
#include "furi_hal_delay.h"
#include "furi_hal_task.h"
#include "furi_hal_power.h"
#include "furi_hal_vcp.h"
#include "furi_hal_interrupt.h"
#include "furi_hal_version.h"
#include "furi_hal_bt.h"
#include "furi_hal_spi.h"
#include "furi_hal_flash.h"
#include "furi_hal_subghz.h"
#include "furi_hal_vibro.h"
#include "furi_hal_ibutton.h"
#include "furi_hal_rfid.h"
#include "furi_hal_nfc.h"
#include "furi_hal_usb.h"
#include "furi_hal_usb_hid.h"
#include "furi_hal_compress.h"
#include "furi_hal_uart.h"
#include "furi_hal_info.h"
#include "furi_hal_random.h"

/** Init furi_hal */
void furi_hal_init();

/**
 * Init critical parts of furi_hal
 * That code should not use memory allocations
 */
void furi_hal_init_critical();
