#include <furi_hal_rfid.h>
#include <furi_hal_ibutton.h>
#include <furi_hal_interrupt.h>
#include <furi_hal_resources.h>
#include <furi.h>

#include <stm32wbxx_ll_tim.h>
#include <stm32wbxx_ll_comp.h>

#define FURI_HAL_RFID_READ_TIMER TIM1
#define FURI_HAL_RFID_READ_TIMER_CHANNEL LL_TIM_CHANNEL_CH1N
// We can't use N channel for LL_TIM_OC_Init, so...
#define FURI_HAL_RFID_READ_TIMER_CHANNEL_CONFIG LL_TIM_CHANNEL_CH1

#define FURI_HAL_RFID_EMULATE_TIMER TIM2
#define FURI_HAL_RFID_EMULATE_TIMER_IRQ FuriHalInterruptIdTIM2
#define FURI_HAL_RFID_EMULATE_TIMER_CHANNEL LL_TIM_CHANNEL_CH3

typedef struct {
    FuriHalRfidEmulateCallback callback;
    void* context;
} FuriHalRfid;

FuriHalRfid* furi_hal_rfid = NULL;

#define LFRFID_LL_READ_TIM TIM1
#define LFRFID_LL_READ_CONFIG_CHANNEL LL_TIM_CHANNEL_CH1
#define LFRFID_LL_READ_CHANNEL LL_TIM_CHANNEL_CH1N

#define LFRFID_LL_EMULATE_TIM TIM2
#define LFRFID_LL_EMULATE_CHANNEL LL_TIM_CHANNEL_CH3

void furi_hal_rfid_init() {
    furi_assert(furi_hal_rfid == NULL);
    furi_hal_rfid = malloc(sizeof(FuriHalRfid));

    furi_hal_rfid_pins_reset();

    LL_COMP_InitTypeDef COMP_InitStruct = {0};
    COMP_InitStruct.PowerMode = LL_COMP_POWERMODE_MEDIUMSPEED;
    COMP_InitStruct.InputPlus = LL_COMP_INPUT_PLUS_IO1;
    COMP_InitStruct.InputMinus = LL_COMP_INPUT_MINUS_1_2VREFINT;
    COMP_InitStruct.InputHysteresis = LL_COMP_HYSTERESIS_HIGH;
#ifdef INVERT_RFID_IN
    COMP_InitStruct.OutputPolarity = LL_COMP_OUTPUTPOL_INVERTED;
#else
    COMP_InitStruct.OutputPolarity = LL_COMP_OUTPUTPOL_NONINVERTED;
#endif
    COMP_InitStruct.OutputBlankingSource = LL_COMP_BLANKINGSRC_NONE;
    LL_COMP_Init(COMP1, &COMP_InitStruct);
    LL_COMP_SetCommonWindowMode(__LL_COMP_COMMON_INSTANCE(COMP1), LL_COMP_WINDOWMODE_DISABLE);

    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_20);
    LL_EXTI_EnableFallingTrig_0_31(LL_EXTI_LINE_20);
    LL_EXTI_EnableRisingTrig_0_31(LL_EXTI_LINE_20);
    LL_EXTI_DisableEvent_0_31(LL_EXTI_LINE_20);
    LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_20);

    NVIC_SetPriority(COMP_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(COMP_IRQn);
}

void furi_hal_rfid_pins_reset() {
    // ibutton bus disable
    furi_hal_ibutton_stop();

    // pulldown rfid antenna
    furi_hal_gpio_init(&gpio_rfid_carrier_out, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_write(&gpio_rfid_carrier_out, false);

    // from both sides
    furi_hal_gpio_init(&gpio_nfc_irq_rfid_pull, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_write(&gpio_nfc_irq_rfid_pull, true);

    furi_hal_gpio_init_simple(&gpio_rfid_carrier, GpioModeAnalog);

    furi_hal_gpio_init(&gpio_rfid_data_in, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
}

void furi_hal_rfid_pins_emulate() {
    // ibutton low
    furi_hal_ibutton_start_drive();
    furi_hal_ibutton_pin_low();

    // pull pin to timer out
    furi_hal_gpio_init_ex(
        &gpio_nfc_irq_rfid_pull,
        GpioModeAltFunctionPushPull,
        GpioPullNo,
        GpioSpeedLow,
        GpioAltFn1TIM2);

    // pull rfid antenna from carrier side
    furi_hal_gpio_init(&gpio_rfid_carrier_out, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_write(&gpio_rfid_carrier_out, false);

    furi_hal_gpio_init_ex(
        &gpio_rfid_carrier, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedLow, GpioAltFn2TIM2);
}

void furi_hal_rfid_pins_read() {
    // ibutton low
    furi_hal_ibutton_start_drive();
    furi_hal_ibutton_pin_low();

    // dont pull rfid antenna
    furi_hal_gpio_init(&gpio_nfc_irq_rfid_pull, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_write(&gpio_nfc_irq_rfid_pull, false);

    // carrier pin to timer out
    furi_hal_gpio_init_ex(
        &gpio_rfid_carrier_out,
        GpioModeAltFunctionPushPull,
        GpioPullNo,
        GpioSpeedLow,
        GpioAltFn1TIM1);

    // comparator in
    furi_hal_gpio_init(&gpio_rfid_data_in, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
}

void furi_hal_rfid_pin_pull_release() {
    furi_hal_gpio_write(&gpio_nfc_irq_rfid_pull, true);
}

void furi_hal_rfid_pin_pull_pulldown() {
    furi_hal_gpio_write(&gpio_nfc_irq_rfid_pull, false);
}

void furi_hal_rfid_tim_read(float freq, float duty_cycle) {
    FURI_CRITICAL_ENTER();
    LL_TIM_DeInit(FURI_HAL_RFID_READ_TIMER);
    FURI_CRITICAL_EXIT();

    LL_TIM_InitTypeDef TIM_InitStruct = {0};
    TIM_InitStruct.Autoreload = (SystemCoreClock / freq) - 1;
    LL_TIM_Init(FURI_HAL_RFID_READ_TIMER, &TIM_InitStruct);
    LL_TIM_DisableARRPreload(FURI_HAL_RFID_READ_TIMER);

    LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};
    TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
    TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_ENABLE;
    TIM_OC_InitStruct.CompareValue = TIM_InitStruct.Autoreload * duty_cycle;
    LL_TIM_OC_Init(
        FURI_HAL_RFID_READ_TIMER, FURI_HAL_RFID_READ_TIMER_CHANNEL_CONFIG, &TIM_OC_InitStruct);

    LL_TIM_EnableCounter(FURI_HAL_RFID_READ_TIMER);
}

void furi_hal_rfid_tim_read_start() {
    LL_TIM_EnableAllOutputs(FURI_HAL_RFID_READ_TIMER);
}

void furi_hal_rfid_tim_read_stop() {
    LL_TIM_DisableAllOutputs(FURI_HAL_RFID_READ_TIMER);
}

void furi_hal_rfid_tim_emulate(float freq) {
    UNUSED(freq); // FIXME
    // basic PWM setup with needed freq and internal clock
    FURI_CRITICAL_ENTER();
    LL_TIM_DeInit(FURI_HAL_RFID_EMULATE_TIMER);
    FURI_CRITICAL_EXIT();

    LL_TIM_SetPrescaler(FURI_HAL_RFID_EMULATE_TIMER, 0);
    LL_TIM_SetCounterMode(FURI_HAL_RFID_EMULATE_TIMER, LL_TIM_COUNTERMODE_UP);
    LL_TIM_SetAutoReload(FURI_HAL_RFID_EMULATE_TIMER, 1);
    LL_TIM_DisableARRPreload(FURI_HAL_RFID_EMULATE_TIMER);
    LL_TIM_SetRepetitionCounter(FURI_HAL_RFID_EMULATE_TIMER, 0);

    LL_TIM_SetClockDivision(FURI_HAL_RFID_EMULATE_TIMER, LL_TIM_CLOCKDIVISION_DIV1);
    LL_TIM_SetClockSource(FURI_HAL_RFID_EMULATE_TIMER, LL_TIM_CLOCKSOURCE_EXT_MODE2);
    LL_TIM_ConfigETR(
        FURI_HAL_RFID_EMULATE_TIMER,
        LL_TIM_ETR_POLARITY_INVERTED,
        LL_TIM_ETR_PRESCALER_DIV1,
        LL_TIM_ETR_FILTER_FDIV1);

    LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};
    TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
    TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_ENABLE;
    TIM_OC_InitStruct.CompareValue = 1;
    LL_TIM_OC_Init(
        FURI_HAL_RFID_EMULATE_TIMER, FURI_HAL_RFID_EMULATE_TIMER_CHANNEL, &TIM_OC_InitStruct);

    LL_TIM_GenerateEvent_UPDATE(FURI_HAL_RFID_EMULATE_TIMER);
}

static void furi_hal_rfid_emulate_isr() {
    if(LL_TIM_IsActiveFlag_UPDATE(FURI_HAL_RFID_EMULATE_TIMER)) {
        LL_TIM_ClearFlag_UPDATE(FURI_HAL_RFID_EMULATE_TIMER);
        furi_hal_rfid->callback(furi_hal_rfid->context);
    }
}

void furi_hal_rfid_tim_emulate_start(FuriHalRfidEmulateCallback callback, void* context) {
    furi_assert(furi_hal_rfid);

    furi_hal_rfid->callback = callback;
    furi_hal_rfid->context = context;

    furi_hal_interrupt_set_isr(FURI_HAL_RFID_EMULATE_TIMER_IRQ, furi_hal_rfid_emulate_isr, NULL);

    LL_TIM_EnableIT_UPDATE(FURI_HAL_RFID_EMULATE_TIMER);
    LL_TIM_EnableAllOutputs(FURI_HAL_RFID_EMULATE_TIMER);
    LL_TIM_EnableCounter(FURI_HAL_RFID_EMULATE_TIMER);
}

void furi_hal_rfid_tim_emulate_stop() {
    LL_TIM_DisableCounter(FURI_HAL_RFID_EMULATE_TIMER);
    LL_TIM_DisableAllOutputs(FURI_HAL_RFID_EMULATE_TIMER);
    furi_hal_interrupt_set_isr(FURI_HAL_RFID_EMULATE_TIMER_IRQ, NULL, NULL);
}

void furi_hal_rfid_tim_reset() {
    FURI_CRITICAL_ENTER();

    LL_TIM_DeInit(FURI_HAL_RFID_READ_TIMER);
    LL_TIM_DeInit(FURI_HAL_RFID_EMULATE_TIMER);

    FURI_CRITICAL_EXIT();
}

void furi_hal_rfid_set_emulate_period(uint32_t period) {
    LL_TIM_SetAutoReload(FURI_HAL_RFID_EMULATE_TIMER, period);
}

void furi_hal_rfid_set_emulate_pulse(uint32_t pulse) {
#if FURI_HAL_RFID_EMULATE_TIMER_CHANNEL == LL_TIM_CHANNEL_CH3
    LL_TIM_OC_SetCompareCH3(FURI_HAL_RFID_EMULATE_TIMER, pulse);
#else
#error Update this code. Would you kindly?
#endif
}

void furi_hal_rfid_set_read_period(uint32_t period) {
    LL_TIM_SetAutoReload(FURI_HAL_RFID_READ_TIMER, period);
}

void furi_hal_rfid_set_read_pulse(uint32_t pulse) {
#if FURI_HAL_RFID_READ_TIMER_CHANNEL == LL_TIM_CHANNEL_CH1N
    LL_TIM_OC_SetCompareCH1(FURI_HAL_RFID_READ_TIMER, pulse);
#else
#error Update this code. Would you kindly?
#endif
}

void furi_hal_rfid_change_read_config(float freq, float duty_cycle) {
    uint32_t period = (uint32_t)((SystemCoreClock) / freq) - 1;
    furi_hal_rfid_set_read_period(period);
    furi_hal_rfid_set_read_pulse(period * duty_cycle);
}

void furi_hal_rfid_comp_start() {
    LL_COMP_Enable(COMP1);
    // Magic
    uint32_t wait_loop_index = ((80 / 10UL) * ((SystemCoreClock / (100000UL * 2UL)) + 1UL));
    while(wait_loop_index) {
        wait_loop_index--;
    }
}

void furi_hal_rfid_comp_stop() {
    LL_COMP_Disable(COMP1);
}

FuriHalRfidCompCallback furi_hal_rfid_comp_callback = NULL;
void* furi_hal_rfid_comp_callback_context = NULL;

void furi_hal_rfid_comp_set_callback(FuriHalRfidCompCallback callback, void* context) {
    FURI_CRITICAL_ENTER();
    furi_hal_rfid_comp_callback = callback;
    furi_hal_rfid_comp_callback_context = context;
    __DMB();
    FURI_CRITICAL_EXIT();
}

/* Comparator trigger event */
void COMP_IRQHandler() {
    if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_20)) {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_20);
    }
    if(furi_hal_rfid_comp_callback) {
        furi_hal_rfid_comp_callback(
            (LL_COMP_ReadOutputLevel(COMP1) == LL_COMP_OUTPUT_LEVEL_LOW),
            furi_hal_rfid_comp_callback_context);
    }
}
