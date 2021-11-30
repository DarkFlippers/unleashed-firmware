/**
 * @file furi-hal.h
 * Furi HAL API
 */

#pragma once

#ifdef __cplusplus
template <unsigned int N> struct STOP_EXTERNING_ME {};
#endif

#include "furi-hal-bootloader.h"
#include "furi-hal-clock.h"
#include "furi-hal-crypto.h"
#include "furi-hal-console.h"
#include "furi-hal-os.h"
#include "furi-hal-sd.h"
#include "furi-hal-i2c.h"
#include "furi-hal-resources.h"
#include "furi-hal-gpio.h"
#include "furi-hal-light.h"
#include "furi-hal-delay.h"
#include "furi-hal-pwm.h"
#include "furi-hal-task.h"
#include "furi-hal-power.h"
#include "furi-hal-vcp.h"
#include "furi-hal-interrupt.h"
#include "furi-hal-version.h"
#include "furi-hal-bt.h"
#include "furi-hal-spi.h"
#include "furi-hal-flash.h"
#include "furi-hal-subghz.h"
#include "furi-hal-vibro.h"
#include "furi-hal-ibutton.h"
#include "furi-hal-rfid.h"
#include "furi-hal-nfc.h"
#include "furi-hal-usb.h"
#include "furi-hal-usb-hid.h"
#include "furi-hal-compress.h"
#include "furi-hal-uart.h"

/** Init furi-hal */
void furi_hal_init();
