#include <furi_hal_ibutton.h>
#include <furi_hal_interrupt.h>
#include <furi_hal_resources.h>
#include <furi_hal_bus.h>

#include <stm32wbxx_ll_tim.h>

#include <furi.h>

#define TAG "FuriHalIbutton"

#define FURI_HAL_IBUTTON_TIMER     TIM1
#define FURI_HAL_IBUTTON_TIMER_BUS FuriHalBusTIM1
#define FURI_HAL_IBUTTON_TIMER_IRQ FuriHalInterruptIdTim1UpTim16

typedef enum {
    FuriHalIbuttonStateIdle,
    FuriHalIbuttonStateRunning,
} FuriHalIbuttonState;

typedef struct {
    FuriHalIbuttonState state;
    FuriHalIbuttonEmulateCallback callback;
    void* context;
} FuriHalIbutton;

FuriHalIbutton* furi_hal_ibutton = NULL;

static void furi_hal_ibutton_emulate_isr(void* context) {
    UNUSED(context);
    if(LL_TIM_IsActiveFlag_UPDATE(FURI_HAL_IBUTTON_TIMER)) {
        LL_TIM_ClearFlag_UPDATE(FURI_HAL_IBUTTON_TIMER);
        furi_hal_ibutton->callback(furi_hal_ibutton->context);
    }
}

void furi_hal_ibutton_init(void) {
    furi_hal_ibutton = malloc(sizeof(FuriHalIbutton));
    furi_hal_ibutton->state = FuriHalIbuttonStateIdle;

    FURI_LOG_I(TAG, "Init OK");
}

void furi_hal_ibutton_emulate_start(
    uint32_t period,
    FuriHalIbuttonEmulateCallback callback,
    void* context) {
    furi_check(furi_hal_ibutton);
    furi_check(furi_hal_ibutton->state == FuriHalIbuttonStateIdle);

    furi_hal_ibutton->state = FuriHalIbuttonStateRunning;
    furi_hal_ibutton->callback = callback;
    furi_hal_ibutton->context = context;

    furi_hal_bus_enable(FURI_HAL_IBUTTON_TIMER_BUS);

    furi_hal_interrupt_set_isr(FURI_HAL_IBUTTON_TIMER_IRQ, furi_hal_ibutton_emulate_isr, NULL);

    LL_TIM_SetPrescaler(FURI_HAL_IBUTTON_TIMER, 0);
    LL_TIM_SetCounterMode(FURI_HAL_IBUTTON_TIMER, LL_TIM_COUNTERMODE_UP);
    LL_TIM_SetAutoReload(FURI_HAL_IBUTTON_TIMER, period);
    LL_TIM_DisableARRPreload(FURI_HAL_IBUTTON_TIMER);
    LL_TIM_SetRepetitionCounter(FURI_HAL_IBUTTON_TIMER, 0);

    LL_TIM_SetClockDivision(FURI_HAL_IBUTTON_TIMER, LL_TIM_CLOCKDIVISION_DIV1);
    LL_TIM_SetClockSource(FURI_HAL_IBUTTON_TIMER, LL_TIM_CLOCKSOURCE_INTERNAL);
    LL_TIM_GenerateEvent_UPDATE(FURI_HAL_IBUTTON_TIMER);

    LL_TIM_EnableIT_UPDATE(FURI_HAL_IBUTTON_TIMER);

    LL_TIM_EnableCounter(FURI_HAL_IBUTTON_TIMER);
}

void furi_hal_ibutton_emulate_set_next(uint32_t period) {
    LL_TIM_SetAutoReload(FURI_HAL_IBUTTON_TIMER, period);
}

void furi_hal_ibutton_emulate_stop(void) {
    furi_check(furi_hal_ibutton);

    if(furi_hal_ibutton->state == FuriHalIbuttonStateRunning) {
        furi_hal_ibutton->state = FuriHalIbuttonStateIdle;
        LL_TIM_DisableCounter(FURI_HAL_IBUTTON_TIMER);

        furi_hal_bus_disable(FURI_HAL_IBUTTON_TIMER_BUS);
        furi_hal_interrupt_set_isr(FURI_HAL_IBUTTON_TIMER_IRQ, NULL, NULL);

        furi_hal_ibutton->callback = NULL;
        furi_hal_ibutton->context = NULL;
    }
}

void furi_hal_ibutton_pin_configure(void) {
    furi_hal_gpio_write(&gpio_ibutton, true);
    furi_hal_gpio_init(&gpio_ibutton, GpioModeOutputOpenDrain, GpioPullNo, GpioSpeedLow);
}

void furi_hal_ibutton_pin_reset(void) {
    furi_hal_gpio_write(&gpio_ibutton, true);
    furi_hal_gpio_init(&gpio_ibutton, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
}

void furi_hal_ibutton_pin_write(const bool state) {
    furi_hal_gpio_write(&gpio_ibutton, state);
}
