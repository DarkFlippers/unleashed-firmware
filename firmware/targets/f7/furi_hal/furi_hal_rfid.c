#include <furi_hal_rfid.h>
#include <furi_hal_ibutton.h>
#include <furi_hal_interrupt.h>
#include <furi_hal_resources.h>
#include <furi.h>

#include <stm32wbxx_ll_tim.h>
#include <stm32wbxx_ll_comp.h>
#include <stm32wbxx_ll_dma.h>

#define FURI_HAL_RFID_READ_TIMER TIM1
#define FURI_HAL_RFID_READ_TIMER_CHANNEL LL_TIM_CHANNEL_CH1N
// We can't use N channel for LL_TIM_OC_Init, so...
#define FURI_HAL_RFID_READ_TIMER_CHANNEL_CONFIG LL_TIM_CHANNEL_CH1

#define FURI_HAL_RFID_EMULATE_TIMER TIM2
#define FURI_HAL_RFID_EMULATE_TIMER_IRQ FuriHalInterruptIdTIM2
#define FURI_HAL_RFID_EMULATE_TIMER_CHANNEL LL_TIM_CHANNEL_CH3

#define RFID_CAPTURE_TIM TIM2
#define RFID_CAPTURE_IND_CH LL_TIM_CHANNEL_CH3
#define RFID_CAPTURE_DIR_CH LL_TIM_CHANNEL_CH4

/* DMA Channels definition */
#define RFID_DMA DMA2
#define RFID_DMA_CH1_CHANNEL LL_DMA_CHANNEL_1
#define RFID_DMA_CH2_CHANNEL LL_DMA_CHANNEL_2
#define RFID_DMA_CH1_IRQ FuriHalInterruptIdDma2Ch1
#define RFID_DMA_CH1_DEF RFID_DMA, RFID_DMA_CH1_CHANNEL
#define RFID_DMA_CH2_DEF RFID_DMA, RFID_DMA_CH2_CHANNEL

typedef struct {
    FuriHalRfidEmulateCallback callback;
    FuriHalRfidDMACallback dma_callback;
    FuriHalRfidReadCaptureCallback read_capture_callback;
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
    furi_hal_ibutton_pin_reset();

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
    furi_hal_ibutton_pin_configure();
    furi_hal_ibutton_pin_write(false);

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
    furi_hal_ibutton_pin_configure();
    furi_hal_ibutton_pin_write(false);

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

static void furi_hal_capture_dma_isr(void* context) {
    UNUSED(context);

    // Channel 3, positive level
    if(LL_TIM_IsActiveFlag_CC3(RFID_CAPTURE_TIM)) {
        LL_TIM_ClearFlag_CC3(RFID_CAPTURE_TIM);
        furi_hal_rfid->read_capture_callback(
            true, LL_TIM_IC_GetCaptureCH3(RFID_CAPTURE_TIM), furi_hal_rfid->context);
    }

    // Channel 4, overall level
    if(LL_TIM_IsActiveFlag_CC4(RFID_CAPTURE_TIM)) {
        LL_TIM_ClearFlag_CC4(RFID_CAPTURE_TIM);
        LL_TIM_SetCounter(RFID_CAPTURE_TIM, 0);
        furi_hal_rfid->read_capture_callback(
            false, LL_TIM_IC_GetCaptureCH4(RFID_CAPTURE_TIM), furi_hal_rfid->context);
    }
}

void furi_hal_rfid_tim_read_capture_start(FuriHalRfidReadCaptureCallback callback, void* context) {
    FURI_CRITICAL_ENTER();
    LL_TIM_DeInit(RFID_CAPTURE_TIM);
    FURI_CRITICAL_EXIT();

    furi_assert(furi_hal_rfid);

    furi_hal_rfid->read_capture_callback = callback;
    furi_hal_rfid->context = context;

    // Timer: base
    LL_TIM_InitTypeDef TIM_InitStruct = {0};
    TIM_InitStruct.Prescaler = 64 - 1;
    TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
    TIM_InitStruct.Autoreload = UINT32_MAX;
    TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
    LL_TIM_Init(RFID_CAPTURE_TIM, &TIM_InitStruct);

    // Timer: advanced
    LL_TIM_SetClockSource(RFID_CAPTURE_TIM, LL_TIM_CLOCKSOURCE_INTERNAL);
    LL_TIM_DisableARRPreload(RFID_CAPTURE_TIM);
    LL_TIM_SetTriggerInput(RFID_CAPTURE_TIM, LL_TIM_TS_TI2FP2);
    LL_TIM_SetSlaveMode(RFID_CAPTURE_TIM, LL_TIM_SLAVEMODE_DISABLED);
    LL_TIM_SetTriggerOutput(RFID_CAPTURE_TIM, LL_TIM_TRGO_RESET);
    LL_TIM_EnableMasterSlaveMode(RFID_CAPTURE_TIM);
    LL_TIM_DisableDMAReq_TRIG(RFID_CAPTURE_TIM);
    LL_TIM_DisableIT_TRIG(RFID_CAPTURE_TIM);
    LL_TIM_SetRemap(RFID_CAPTURE_TIM, LL_TIM_TIM2_TI4_RMP_COMP1);

    // Timer: channel 3 indirect
    LL_TIM_IC_SetActiveInput(RFID_CAPTURE_TIM, RFID_CAPTURE_IND_CH, LL_TIM_ACTIVEINPUT_INDIRECTTI);
    LL_TIM_IC_SetPrescaler(RFID_CAPTURE_TIM, RFID_CAPTURE_IND_CH, LL_TIM_ICPSC_DIV1);
    LL_TIM_IC_SetPolarity(RFID_CAPTURE_TIM, RFID_CAPTURE_IND_CH, LL_TIM_IC_POLARITY_FALLING);
    LL_TIM_IC_SetFilter(RFID_CAPTURE_TIM, RFID_CAPTURE_IND_CH, LL_TIM_IC_FILTER_FDIV1);

    // Timer: channel 4 direct
    LL_TIM_IC_SetActiveInput(RFID_CAPTURE_TIM, RFID_CAPTURE_DIR_CH, LL_TIM_ACTIVEINPUT_DIRECTTI);
    LL_TIM_IC_SetPrescaler(RFID_CAPTURE_TIM, RFID_CAPTURE_DIR_CH, LL_TIM_ICPSC_DIV1);
    LL_TIM_IC_SetPolarity(RFID_CAPTURE_TIM, RFID_CAPTURE_DIR_CH, LL_TIM_IC_POLARITY_RISING);
    LL_TIM_IC_SetFilter(RFID_CAPTURE_TIM, RFID_CAPTURE_DIR_CH, LL_TIM_IC_FILTER_FDIV1);

    furi_hal_interrupt_set_isr(FURI_HAL_RFID_EMULATE_TIMER_IRQ, furi_hal_capture_dma_isr, NULL);

    LL_TIM_EnableIT_CC3(RFID_CAPTURE_TIM);
    LL_TIM_EnableIT_CC4(RFID_CAPTURE_TIM);
    LL_TIM_CC_EnableChannel(RFID_CAPTURE_TIM, RFID_CAPTURE_IND_CH);
    LL_TIM_CC_EnableChannel(RFID_CAPTURE_TIM, RFID_CAPTURE_DIR_CH);
    LL_TIM_SetCounter(RFID_CAPTURE_TIM, 0);
    LL_TIM_EnableCounter(RFID_CAPTURE_TIM);

    furi_hal_rfid_comp_start();
}

void furi_hal_rfid_tim_read_capture_stop() {
    furi_hal_rfid_comp_stop();

    furi_hal_interrupt_set_isr(FURI_HAL_RFID_EMULATE_TIMER_IRQ, NULL, NULL);

    FURI_CRITICAL_ENTER();
    LL_TIM_DeInit(RFID_CAPTURE_TIM);
    FURI_CRITICAL_EXIT();
}

static void furi_hal_rfid_dma_isr() {
#if RFID_DMA_CH1_CHANNEL == LL_DMA_CHANNEL_1
    if(LL_DMA_IsActiveFlag_HT1(RFID_DMA)) {
        LL_DMA_ClearFlag_HT1(RFID_DMA);
        furi_hal_rfid->dma_callback(true, furi_hal_rfid->context);
    }

    if(LL_DMA_IsActiveFlag_TC1(RFID_DMA)) {
        LL_DMA_ClearFlag_TC1(RFID_DMA);
        furi_hal_rfid->dma_callback(false, furi_hal_rfid->context);
    }
#else
#error Update this code. Would you kindly?
#endif
}

void furi_hal_rfid_tim_emulate_dma_start(
    uint32_t* duration,
    uint32_t* pulse,
    size_t length,
    FuriHalRfidDMACallback callback,
    void* context) {
    furi_assert(furi_hal_rfid);

    // setup interrupts
    furi_hal_rfid->dma_callback = callback;
    furi_hal_rfid->context = context;

    // setup pins
    furi_hal_rfid_pins_emulate();

    // configure timer
    furi_hal_rfid_tim_emulate(125000);
    LL_TIM_OC_SetPolarity(
        FURI_HAL_RFID_EMULATE_TIMER, FURI_HAL_RFID_EMULATE_TIMER_CHANNEL, LL_TIM_OCPOLARITY_HIGH);
    LL_TIM_EnableDMAReq_UPDATE(FURI_HAL_RFID_EMULATE_TIMER);

    // configure DMA "mem -> ARR" channel
    LL_DMA_InitTypeDef dma_config = {0};
    dma_config.PeriphOrM2MSrcAddress = (uint32_t) & (FURI_HAL_RFID_EMULATE_TIMER->ARR);
    dma_config.MemoryOrM2MDstAddress = (uint32_t)duration;
    dma_config.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
    dma_config.Mode = LL_DMA_MODE_CIRCULAR;
    dma_config.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
    dma_config.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
    dma_config.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_WORD;
    dma_config.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_WORD;
    dma_config.NbData = length;
    dma_config.PeriphRequest = LL_DMAMUX_REQ_TIM2_UP;
    dma_config.Priority = LL_DMA_MODE_NORMAL;
    LL_DMA_Init(RFID_DMA_CH1_DEF, &dma_config);
    LL_DMA_EnableChannel(RFID_DMA_CH1_DEF);

    // configure DMA "mem -> CCR3" channel
#if FURI_HAL_RFID_EMULATE_TIMER_CHANNEL == LL_TIM_CHANNEL_CH3
    dma_config.PeriphOrM2MSrcAddress = (uint32_t) & (FURI_HAL_RFID_EMULATE_TIMER->CCR3);
#else
#error Update this code. Would you kindly?
#endif
    dma_config.MemoryOrM2MDstAddress = (uint32_t)pulse;
    dma_config.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
    dma_config.Mode = LL_DMA_MODE_CIRCULAR;
    dma_config.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
    dma_config.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
    dma_config.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_WORD;
    dma_config.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_WORD;
    dma_config.NbData = length;
    dma_config.PeriphRequest = LL_DMAMUX_REQ_TIM2_UP;
    dma_config.Priority = LL_DMA_MODE_NORMAL;
    LL_DMA_Init(RFID_DMA_CH2_DEF, &dma_config);
    LL_DMA_EnableChannel(RFID_DMA_CH2_DEF);

    // attach interrupt to one of DMA channels
    furi_hal_interrupt_set_isr(RFID_DMA_CH1_IRQ, furi_hal_rfid_dma_isr, NULL);
    LL_DMA_EnableIT_TC(RFID_DMA_CH1_DEF);
    LL_DMA_EnableIT_HT(RFID_DMA_CH1_DEF);

    // start
    LL_TIM_EnableAllOutputs(FURI_HAL_RFID_EMULATE_TIMER);

    LL_TIM_SetCounter(FURI_HAL_RFID_EMULATE_TIMER, 0);
    LL_TIM_EnableCounter(FURI_HAL_RFID_EMULATE_TIMER);
}

void furi_hal_rfid_tim_emulate_dma_stop() {
    LL_TIM_DisableCounter(FURI_HAL_RFID_EMULATE_TIMER);
    LL_TIM_DisableAllOutputs(FURI_HAL_RFID_EMULATE_TIMER);

    furi_hal_interrupt_set_isr(RFID_DMA_CH1_IRQ, NULL, NULL);
    LL_DMA_DisableIT_TC(RFID_DMA_CH1_DEF);
    LL_DMA_DisableIT_HT(RFID_DMA_CH1_DEF);

    FURI_CRITICAL_ENTER();

    LL_DMA_DeInit(RFID_DMA_CH1_DEF);
    LL_DMA_DeInit(RFID_DMA_CH2_DEF);
    LL_TIM_DeInit(FURI_HAL_RFID_EMULATE_TIMER);

    FURI_CRITICAL_EXIT();
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