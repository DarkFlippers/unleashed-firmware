#include <furi_hal_ibutton.h>
#include <furi_hal_interrupt.h>
#include <furi_hal_resources.h>

#include <stm32wbxx_ll_tim.h>
#include <stm32wbxx_ll_exti.h>

#include <furi.h>

#define FURI_HAL_IBUTTON_TIMER TIM1
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

static void furi_hal_ibutton_emulate_isr() {
    if(LL_TIM_IsActiveFlag_UPDATE(FURI_HAL_IBUTTON_TIMER)) {
        LL_TIM_ClearFlag_UPDATE(FURI_HAL_IBUTTON_TIMER);
        furi_hal_ibutton->callback(furi_hal_ibutton->context);
    }
}

void furi_hal_ibutton_init() {
    furi_hal_ibutton = malloc(sizeof(FuriHalIbutton));
    furi_hal_ibutton->state = FuriHalIbuttonStateIdle;
}

void furi_hal_ibutton_emulate_start(
    uint32_t period,
    FuriHalIbuttonEmulateCallback callback,
    void* context) {
    furi_assert(furi_hal_ibutton);
    furi_assert(furi_hal_ibutton->state == FuriHalIbuttonStateIdle);

    furi_hal_ibutton->state = FuriHalIbuttonStateRunning;
    furi_hal_ibutton->callback = callback;
    furi_hal_ibutton->context = context;

    FURI_CRITICAL_ENTER();
    LL_TIM_DeInit(FURI_HAL_IBUTTON_TIMER);
    FURI_CRITICAL_EXIT();

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

void furi_hal_ibutton_emulate_stop() {
    furi_assert(furi_hal_ibutton);

    if(furi_hal_ibutton->state == FuriHalIbuttonStateRunning) {
        furi_hal_ibutton->state = FuriHalIbuttonStateIdle;
        LL_TIM_DisableCounter(FURI_HAL_IBUTTON_TIMER);

        FURI_CRITICAL_ENTER();
        LL_TIM_DeInit(FURI_HAL_IBUTTON_TIMER);
        FURI_CRITICAL_EXIT();

        furi_hal_interrupt_set_isr(FURI_HAL_IBUTTON_TIMER_IRQ, NULL, NULL);

        furi_hal_ibutton->callback = NULL;
        furi_hal_ibutton->context = NULL;
    }
}

void furi_hal_ibutton_pin_configure() {
    furi_hal_gpio_write(&ibutton_gpio, true);
    furi_hal_gpio_init(&ibutton_gpio, GpioModeOutputOpenDrain, GpioPullNo, GpioSpeedLow);
}

void furi_hal_ibutton_pin_reset() {
    furi_hal_gpio_write(&ibutton_gpio, true);
    furi_hal_gpio_init(&ibutton_gpio, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
}

void furi_hal_ibutton_pin_write(const bool state) {
    furi_hal_gpio_write(&ibutton_gpio, state);
}
