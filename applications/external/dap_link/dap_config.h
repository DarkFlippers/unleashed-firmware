// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2022, Alex Taradov <alex@taradov.com>. All rights reserved.

#ifndef _DAP_CONFIG_H_
#define _DAP_CONFIG_H_

/*- Includes ----------------------------------------------------------------*/
#include <furi_hal_gpio.h>

/*- Definitions -------------------------------------------------------------*/
#define DAP_CONFIG_ENABLE_JTAG

#define DAP_CONFIG_DEFAULT_PORT DAP_PORT_SWD
#define DAP_CONFIG_DEFAULT_CLOCK 4200000 // Hz

#define DAP_CONFIG_PACKET_SIZE 64
#define DAP_CONFIG_PACKET_COUNT 1

#define DAP_CONFIG_JTAG_DEV_COUNT 8

// DAP_CONFIG_PRODUCT_STR must contain "CMSIS-DAP" to be compatible with the standard
#define DAP_CONFIG_VENDOR_STR "Flipper Zero"
#define DAP_CONFIG_PRODUCT_STR "Generic CMSIS-DAP Adapter"
#define DAP_CONFIG_SER_NUM_STR usb_serial_number
#define DAP_CONFIG_CMSIS_DAP_VER_STR "2.0.0"

#define DAP_CONFIG_RESET_TARGET_FN dap_app_target_reset
#define DAP_CONFIG_VENDOR_FN dap_app_vendor_cmd

// Attribute to use for performance-critical functions
#define DAP_CONFIG_PERFORMANCE_ATTR

// A value at which dap_clock_test() produces 1 kHz output on the SWCLK pin
// #define DAP_CONFIG_DELAY_CONSTANT 19000
#define DAP_CONFIG_DELAY_CONSTANT 6290

// A threshold for switching to fast clock (no added delays)
// This is the frequency produced by dap_clock_test(1) on the SWCLK pin
#define DAP_CONFIG_FAST_CLOCK 2400000 // Hz

/*- Prototypes --------------------------------------------------------------*/
extern char usb_serial_number[16];

/*- Implementations ---------------------------------------------------------*/
extern GpioPin flipper_dap_swclk_pin;
extern GpioPin flipper_dap_swdio_pin;
extern GpioPin flipper_dap_reset_pin;
extern GpioPin flipper_dap_tdo_pin;
extern GpioPin flipper_dap_tdi_pin;

extern void dap_app_vendor_cmd(uint8_t cmd);
extern void dap_app_target_reset();
extern void dap_app_disconnect();
extern void dap_app_connect_swd();
extern void dap_app_connect_jtag();

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWCLK_TCK_write(int value) {
    furi_hal_gpio_write(&flipper_dap_swclk_pin, value);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWDIO_TMS_write(int value) {
    furi_hal_gpio_write(&flipper_dap_swdio_pin, value);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_TDI_write(int value) {
#ifdef DAP_CONFIG_ENABLE_JTAG
    furi_hal_gpio_write(&flipper_dap_tdi_pin, value);
#else
    (void)value;
#endif
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_TDO_write(int value) {
#ifdef DAP_CONFIG_ENABLE_JTAG
    furi_hal_gpio_write(&flipper_dap_tdo_pin, value);
#else
    (void)value;
#endif
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_nTRST_write(int value) {
    (void)value;
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_nRESET_write(int value) {
    furi_hal_gpio_write(&flipper_dap_reset_pin, value);
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_SWCLK_TCK_read(void) {
    return furi_hal_gpio_read(&flipper_dap_swclk_pin);
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_SWDIO_TMS_read(void) {
    return furi_hal_gpio_read(&flipper_dap_swdio_pin);
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_TDO_read(void) {
#ifdef DAP_CONFIG_ENABLE_JTAG
    return furi_hal_gpio_read(&flipper_dap_tdo_pin);
#else
    return 0;
#endif
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_TDI_read(void) {
#ifdef DAP_CONFIG_ENABLE_JTAG
    return furi_hal_gpio_read(&flipper_dap_tdi_pin);
#else
    return 0;
#endif
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_nTRST_read(void) {
    return 0;
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_nRESET_read(void) {
    return furi_hal_gpio_read(&flipper_dap_reset_pin);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWCLK_TCK_set(void) {
    LL_GPIO_SetOutputPin(flipper_dap_swclk_pin.port, flipper_dap_swclk_pin.pin);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWCLK_TCK_clr(void) {
    LL_GPIO_ResetOutputPin(flipper_dap_swclk_pin.port, flipper_dap_swclk_pin.pin);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWDIO_TMS_in(void) {
    LL_GPIO_SetPinMode(flipper_dap_swdio_pin.port, flipper_dap_swdio_pin.pin, LL_GPIO_MODE_INPUT);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWDIO_TMS_out(void) {
    LL_GPIO_SetPinMode(flipper_dap_swdio_pin.port, flipper_dap_swdio_pin.pin, LL_GPIO_MODE_OUTPUT);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SETUP(void) {
    furi_hal_gpio_init(&flipper_dap_swdio_pin, GpioModeInput, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_init(&flipper_dap_swclk_pin, GpioModeInput, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_init(&flipper_dap_reset_pin, GpioModeInput, GpioPullNo, GpioSpeedVeryHigh);
#ifdef DAP_CONFIG_ENABLE_JTAG
    furi_hal_gpio_init(&flipper_dap_tdo_pin, GpioModeInput, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_init(&flipper_dap_tdi_pin, GpioModeInput, GpioPullNo, GpioSpeedVeryHigh);
#endif
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_DISCONNECT(void) {
    furi_hal_gpio_init(&flipper_dap_swdio_pin, GpioModeInput, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_init(&flipper_dap_swclk_pin, GpioModeInput, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_init(&flipper_dap_reset_pin, GpioModeInput, GpioPullNo, GpioSpeedVeryHigh);
#ifdef DAP_CONFIG_ENABLE_JTAG
    furi_hal_gpio_init(&flipper_dap_tdo_pin, GpioModeInput, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_init(&flipper_dap_tdi_pin, GpioModeInput, GpioPullNo, GpioSpeedVeryHigh);
#endif
    dap_app_disconnect();
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_CONNECT_SWD(void) {
    furi_hal_gpio_init(
        &flipper_dap_swdio_pin, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_write(&flipper_dap_swdio_pin, true);

    furi_hal_gpio_init(
        &flipper_dap_swclk_pin, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_write(&flipper_dap_swclk_pin, true);

    furi_hal_gpio_init(
        &flipper_dap_reset_pin, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_write(&flipper_dap_reset_pin, true);

#ifdef DAP_CONFIG_ENABLE_JTAG
    furi_hal_gpio_init(&flipper_dap_tdo_pin, GpioModeInput, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_init(&flipper_dap_tdi_pin, GpioModeInput, GpioPullNo, GpioSpeedVeryHigh);
#endif
    dap_app_connect_swd();
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_CONNECT_JTAG(void) {
    furi_hal_gpio_init(
        &flipper_dap_swdio_pin, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_write(&flipper_dap_swdio_pin, true);

    furi_hal_gpio_init(
        &flipper_dap_swclk_pin, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_write(&flipper_dap_swclk_pin, true);

    furi_hal_gpio_init(
        &flipper_dap_reset_pin, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_write(&flipper_dap_reset_pin, true);

#ifdef DAP_CONFIG_ENABLE_JTAG
    furi_hal_gpio_init(&flipper_dap_tdo_pin, GpioModeInput, GpioPullNo, GpioSpeedVeryHigh);

    furi_hal_gpio_init(
        &flipper_dap_tdi_pin, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_write(&flipper_dap_tdi_pin, true);
#endif
    dap_app_connect_jtag();
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_LED(int index, int state) {
    (void)index;
    (void)state;
}

//-----------------------------------------------------------------------------
__attribute__((always_inline)) static inline void DAP_CONFIG_DELAY(uint32_t cycles) {
    asm volatile("1: subs %[cycles], %[cycles], #1 \n"
                 "   bne 1b \n"
                 : [cycles] "+l"(cycles));
}

#endif // _DAP_CONFIG_H_
