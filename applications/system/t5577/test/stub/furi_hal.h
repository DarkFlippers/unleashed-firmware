#pragma once

#include "furi.h"

typedef enum { GpioModeOutputPushPull, GpioModeAltFunctionPushPull } GpioMode;
typedef enum { GpioPullNo } GpioPull;
typedef enum { GpioSpeedLow } GpioSpeed;
typedef enum { GpioAltFn2TIM2 } GpioAltFn;
typedef struct {
    int pin;
} GpioPin;

static const GpioPin gpio_nfc_irq_rfid_pull;
static const GpioPin gpio_rfid_carrier_out;
static const GpioPin gpio_rfid_carrier;

typedef enum { FuriHalBusTIM2 } FuriHalBus;
typedef enum { FuriHalInterruptIdTIM2 } FuriHalInterruptId;
typedef void (*FuriHalInterruptISR)(void*);

static inline void furi_hal_ibutton_pin_configure(void) {
}
static inline void furi_hal_ibutton_pin_write(bool v) {
    (void)v;
}
static inline void furi_hal_gpio_init(const GpioPin* p, GpioMode m, GpioPull u, GpioSpeed s) {
    (void)p;
    (void)m;
    (void)u;
    (void)s;
}
static inline void
    furi_hal_gpio_init_ex(const GpioPin* p, GpioMode m, GpioPull u, GpioSpeed s, GpioAltFn a) {
    (void)p;
    (void)m;
    (void)u;
    (void)s;
    (void)a;
}
static inline void furi_hal_gpio_write(const GpioPin* p, bool v) {
    (void)p;
    (void)v;
}
static inline void furi_hal_bus_enable(FuriHalBus b) {
    (void)b;
}
static inline void furi_hal_bus_disable(FuriHalBus b) {
    (void)b;
}
static inline void
    furi_hal_interrupt_set_isr(FuriHalInterruptId i, FuriHalInterruptISR f, void* c) {
    (void)i;
    (void)f;
    (void)c;
}
static inline void furi_hal_rfid_pins_reset(void) {
}
static inline uint32_t furi_hal_cortex_instructions_per_microsecond(void) {
    return 64;
}
