#include <furi_hal_infrared.h>
#include <core/check.h>
#include "stm32wbxx_ll_dma.h"
#include "sys/_stdint.h"
#include <furi_hal_interrupt.h>
#include <furi_hal_resources.h>

#include <stdint.h>
#include <stm32wbxx_ll_tim.h>
#include <stm32wbxx_ll_gpio.h>

#include <stdio.h>
#include <furi.h>
#include <math.h>

#define INFRARED_TX_DEBUG 0

#if INFRARED_TX_DEBUG == 1
#define gpio_infrared_tx gpio_infrared_tx_debug
const GpioPin gpio_infrared_tx_debug = {.port = GPIOA, .pin = GPIO_PIN_7};
#endif

#define INFRARED_TIM_TX_DMA_BUFFER_SIZE 200
#define INFRARED_POLARITY_SHIFT 1

#define INFRARED_TX_CCMR_HIGH \
    (TIM_CCMR2_OC3PE | LL_TIM_OCMODE_PWM2) /* Mark time - enable PWM2 mode */
#define INFRARED_TX_CCMR_LOW \
    (TIM_CCMR2_OC3PE | LL_TIM_OCMODE_FORCED_INACTIVE) /* Space time - force low */

typedef struct {
    FuriHalInfraredRxCaptureCallback capture_callback;
    void* capture_context;
    FuriHalInfraredRxTimeoutCallback timeout_callback;
    void* timeout_context;
} InfraredTimRx;

typedef struct {
    uint8_t* polarity;
    uint16_t* data;
    size_t size;
    bool packet_end;
    bool last_packet_end;
} InfraredTxBuf;

typedef struct {
    float cycle_duration;
    FuriHalInfraredTxGetDataISRCallback data_callback;
    FuriHalInfraredTxSignalSentISRCallback signal_sent_callback;
    void* data_context;
    void* signal_sent_context;
    InfraredTxBuf buffer[2];
    FuriSemaphore* stop_semaphore;
    uint32_t
        tx_timing_rest_duration; /** if timing is too long (> 0xFFFF), send it in few iterations */
    bool tx_timing_rest_level;
    FuriHalInfraredTxGetDataState tx_timing_rest_status;
} InfraredTimTx;

typedef enum {
    InfraredStateIdle, /** Furi Hal Infrared is ready to start RX or TX */
    InfraredStateAsyncRx, /** Async RX started */
    InfraredStateAsyncTx, /** Async TX started, DMA and timer is on */
    InfraredStateAsyncTxStopReq, /** Async TX started, async stop request received */
    InfraredStateAsyncTxStopInProgress, /** Async TX started, stop request is processed and we wait for last data to be sent */
    InfraredStateAsyncTxStopped, /** Async TX complete, cleanup needed */
    InfraredStateMAX,
} InfraredState;

static volatile InfraredState furi_hal_infrared_state = InfraredStateIdle;
static InfraredTimTx infrared_tim_tx;
static InfraredTimRx infrared_tim_rx;

static void furi_hal_infrared_tx_fill_buffer(uint8_t buf_num, uint8_t polarity_shift);
static void furi_hal_infrared_async_tx_free_resources(void);
static void furi_hal_infrared_tx_dma_set_polarity(uint8_t buf_num, uint8_t polarity_shift);
static void furi_hal_infrared_tx_dma_set_buffer(uint8_t buf_num);
static void furi_hal_infrared_tx_fill_buffer_last(uint8_t buf_num);
static uint8_t furi_hal_infrared_get_current_dma_tx_buffer(void);
static void furi_hal_infrared_tx_dma_polarity_isr();
static void furi_hal_infrared_tx_dma_isr();

static void furi_hal_infrared_tim_rx_isr() {
    static uint32_t previous_captured_ch2 = 0;

    /* Timeout */
    if(LL_TIM_IsActiveFlag_CC3(TIM2)) {
        LL_TIM_ClearFlag_CC3(TIM2);
        furi_assert(furi_hal_infrared_state == InfraredStateAsyncRx);

        /* Timers CNT register starts to counting from 0 to ARR, but it is
         * reseted when Channel 1 catches interrupt. It is not reseted by
         * channel 2, though, so we have to distract it's values (see TimerIRQSourceCCI1 ISR).
         * This can cause false timeout: when time is over, but we started
         * receiving new signal few microseconds ago, because CNT register
         * is reseted once per period, not per sample. */
        if(LL_GPIO_IsInputPinSet(gpio_infrared_rx.port, gpio_infrared_rx.pin) != 0) {
            if(infrared_tim_rx.timeout_callback)
                infrared_tim_rx.timeout_callback(infrared_tim_rx.timeout_context);
        }
    }

    /* Rising Edge */
    if(LL_TIM_IsActiveFlag_CC1(TIM2)) {
        LL_TIM_ClearFlag_CC1(TIM2);
        furi_assert(furi_hal_infrared_state == InfraredStateAsyncRx);

        if(READ_BIT(TIM2->CCMR1, TIM_CCMR1_CC1S)) {
            /* Low pin level is a Mark state of INFRARED signal. Invert level for further processing. */
            uint32_t duration = LL_TIM_IC_GetCaptureCH1(TIM2) - previous_captured_ch2;
            if(infrared_tim_rx.capture_callback)
                infrared_tim_rx.capture_callback(infrared_tim_rx.capture_context, 1, duration);
        } else {
            furi_assert(0);
        }
    }

    /* Falling Edge */
    if(LL_TIM_IsActiveFlag_CC2(TIM2)) {
        LL_TIM_ClearFlag_CC2(TIM2);
        furi_assert(furi_hal_infrared_state == InfraredStateAsyncRx);

        if(READ_BIT(TIM2->CCMR1, TIM_CCMR1_CC2S)) {
            /* High pin level is a Space state of INFRARED signal. Invert level for further processing. */
            uint32_t duration = LL_TIM_IC_GetCaptureCH2(TIM2);
            previous_captured_ch2 = duration;
            if(infrared_tim_rx.capture_callback)
                infrared_tim_rx.capture_callback(infrared_tim_rx.capture_context, 0, duration);
        } else {
            furi_assert(0);
        }
    }
}

void furi_hal_infrared_async_rx_start(void) {
    furi_assert(furi_hal_infrared_state == InfraredStateIdle);

    furi_hal_gpio_init_ex(
        &gpio_infrared_rx, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedLow, GpioAltFn1TIM2);

    LL_TIM_InitTypeDef TIM_InitStruct = {0};
    TIM_InitStruct.Prescaler = 64 - 1;
    TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
    TIM_InitStruct.Autoreload = 0x7FFFFFFE;
    TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
    LL_TIM_Init(TIM2, &TIM_InitStruct);

    LL_TIM_SetClockSource(TIM2, LL_TIM_CLOCKSOURCE_INTERNAL);
    LL_TIM_DisableARRPreload(TIM2);
    LL_TIM_SetTriggerInput(TIM2, LL_TIM_TS_TI1FP1);
    LL_TIM_SetSlaveMode(TIM2, LL_TIM_SLAVEMODE_RESET);
    LL_TIM_CC_DisableChannel(TIM2, LL_TIM_CHANNEL_CH2);
    LL_TIM_IC_SetFilter(TIM2, LL_TIM_CHANNEL_CH2, LL_TIM_IC_FILTER_FDIV1);
    LL_TIM_IC_SetPolarity(TIM2, LL_TIM_CHANNEL_CH2, LL_TIM_IC_POLARITY_FALLING);
    LL_TIM_DisableIT_TRIG(TIM2);
    LL_TIM_DisableDMAReq_TRIG(TIM2);
    LL_TIM_SetTriggerOutput(TIM2, LL_TIM_TRGO_RESET);
    LL_TIM_EnableMasterSlaveMode(TIM2);
    LL_TIM_IC_SetActiveInput(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_ACTIVEINPUT_DIRECTTI);
    LL_TIM_IC_SetPrescaler(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_ICPSC_DIV1);
    LL_TIM_IC_SetFilter(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_IC_FILTER_FDIV1);
    LL_TIM_IC_SetPolarity(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_IC_POLARITY_RISING);
    LL_TIM_IC_SetActiveInput(TIM2, LL_TIM_CHANNEL_CH2, LL_TIM_ACTIVEINPUT_INDIRECTTI);
    LL_TIM_IC_SetPrescaler(TIM2, LL_TIM_CHANNEL_CH2, LL_TIM_ICPSC_DIV1);

    furi_hal_interrupt_set_isr(FuriHalInterruptIdTIM2, furi_hal_infrared_tim_rx_isr, NULL);
    furi_hal_infrared_state = InfraredStateAsyncRx;

    LL_TIM_EnableIT_CC1(TIM2);
    LL_TIM_EnableIT_CC2(TIM2);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH2);

    LL_TIM_SetCounter(TIM2, 0);
    LL_TIM_EnableCounter(TIM2);
}

void furi_hal_infrared_async_rx_stop(void) {
    furi_assert(furi_hal_infrared_state == InfraredStateAsyncRx);

    FURI_CRITICAL_ENTER();

    LL_TIM_DeInit(TIM2);
    furi_hal_interrupt_set_isr(FuriHalInterruptIdTIM2, NULL, NULL);
    furi_hal_infrared_state = InfraredStateIdle;

    FURI_CRITICAL_EXIT();
}

void furi_hal_infrared_async_rx_set_timeout(uint32_t timeout_us) {
    LL_TIM_OC_SetCompareCH3(TIM2, timeout_us);
    LL_TIM_OC_SetMode(TIM2, LL_TIM_CHANNEL_CH3, LL_TIM_OCMODE_ACTIVE);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH3);
    LL_TIM_EnableIT_CC3(TIM2);
}

bool furi_hal_infrared_is_busy(void) {
    return furi_hal_infrared_state != InfraredStateIdle;
}

void furi_hal_infrared_async_rx_set_capture_isr_callback(
    FuriHalInfraredRxCaptureCallback callback,
    void* ctx) {
    infrared_tim_rx.capture_callback = callback;
    infrared_tim_rx.capture_context = ctx;
}

void furi_hal_infrared_async_rx_set_timeout_isr_callback(
    FuriHalInfraredRxTimeoutCallback callback,
    void* ctx) {
    infrared_tim_rx.timeout_callback = callback;
    infrared_tim_rx.timeout_context = ctx;
}

static void furi_hal_infrared_tx_dma_terminate(void) {
    LL_DMA_DisableIT_TC(DMA1, LL_DMA_CHANNEL_1);
    LL_DMA_DisableIT_HT(DMA1, LL_DMA_CHANNEL_2);
    LL_DMA_DisableIT_TC(DMA1, LL_DMA_CHANNEL_2);

    furi_assert(furi_hal_infrared_state == InfraredStateAsyncTxStopInProgress);

    LL_DMA_DisableIT_TC(DMA1, LL_DMA_CHANNEL_1);
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_1);
    LL_TIM_DisableCounter(TIM1);
    FuriStatus status = furi_semaphore_release(infrared_tim_tx.stop_semaphore);
    furi_check(status == FuriStatusOk);
    furi_hal_infrared_state = InfraredStateAsyncTxStopped;
}

static uint8_t furi_hal_infrared_get_current_dma_tx_buffer(void) {
    uint8_t buf_num = 0;
    uint32_t buffer_adr = LL_DMA_GetMemoryAddress(DMA1, LL_DMA_CHANNEL_2);
    if(buffer_adr == (uint32_t)infrared_tim_tx.buffer[0].data) {
        buf_num = 0;
    } else if(buffer_adr == (uint32_t)infrared_tim_tx.buffer[1].data) {
        buf_num = 1;
    } else {
        furi_assert(0);
    }
    return buf_num;
}

static void furi_hal_infrared_tx_dma_polarity_isr() {
    if(LL_DMA_IsActiveFlag_TE1(DMA1)) {
        LL_DMA_ClearFlag_TE1(DMA1);
        furi_crash(NULL);
    }
    if(LL_DMA_IsActiveFlag_TC1(DMA1) && LL_DMA_IsEnabledIT_TC(DMA1, LL_DMA_CHANNEL_1)) {
        LL_DMA_ClearFlag_TC1(DMA1);

        furi_check(
            (furi_hal_infrared_state == InfraredStateAsyncTx) ||
            (furi_hal_infrared_state == InfraredStateAsyncTxStopReq) ||
            (furi_hal_infrared_state == InfraredStateAsyncTxStopInProgress));
        /* actually TC2 is processed and buffer is next buffer */
        uint8_t next_buf_num = furi_hal_infrared_get_current_dma_tx_buffer();
        furi_hal_infrared_tx_dma_set_polarity(next_buf_num, 0);
    }
}

static void furi_hal_infrared_tx_dma_isr() {
    if(LL_DMA_IsActiveFlag_TE2(DMA1)) {
        LL_DMA_ClearFlag_TE2(DMA1);
        furi_crash(NULL);
    }
    if(LL_DMA_IsActiveFlag_HT2(DMA1) && LL_DMA_IsEnabledIT_HT(DMA1, LL_DMA_CHANNEL_2)) {
        LL_DMA_ClearFlag_HT2(DMA1);
        uint8_t buf_num = furi_hal_infrared_get_current_dma_tx_buffer();
        uint8_t next_buf_num = !buf_num;
        if(infrared_tim_tx.buffer[buf_num].last_packet_end) {
            LL_DMA_DisableIT_HT(DMA1, LL_DMA_CHANNEL_2);
        } else if(
            !infrared_tim_tx.buffer[buf_num].packet_end ||
            (furi_hal_infrared_state == InfraredStateAsyncTx)) {
            furi_hal_infrared_tx_fill_buffer(next_buf_num, 0);
            if(infrared_tim_tx.buffer[next_buf_num].last_packet_end) {
                LL_DMA_DisableIT_HT(DMA1, LL_DMA_CHANNEL_2);
            }
        } else if(furi_hal_infrared_state == InfraredStateAsyncTxStopReq) {
            /* fallthrough */
        } else {
            furi_crash(NULL);
        }
    }
    if(LL_DMA_IsActiveFlag_TC2(DMA1) && LL_DMA_IsEnabledIT_TC(DMA1, LL_DMA_CHANNEL_2)) {
        LL_DMA_ClearFlag_TC2(DMA1);
        furi_check(
            (furi_hal_infrared_state == InfraredStateAsyncTxStopInProgress) ||
            (furi_hal_infrared_state == InfraredStateAsyncTxStopReq) ||
            (furi_hal_infrared_state == InfraredStateAsyncTx));

        uint8_t buf_num = furi_hal_infrared_get_current_dma_tx_buffer();
        uint8_t next_buf_num = !buf_num;
        if(furi_hal_infrared_state == InfraredStateAsyncTxStopInProgress) {
            furi_hal_infrared_tx_dma_terminate();
        } else if(
            infrared_tim_tx.buffer[buf_num].last_packet_end ||
            (infrared_tim_tx.buffer[buf_num].packet_end &&
             (furi_hal_infrared_state == InfraredStateAsyncTxStopReq))) {
            furi_hal_infrared_state = InfraredStateAsyncTxStopInProgress;
            furi_hal_infrared_tx_fill_buffer_last(next_buf_num);
            furi_hal_infrared_tx_dma_set_buffer(next_buf_num);
        } else {
            /* if it's not end of the packet - continue receiving */
            furi_hal_infrared_tx_dma_set_buffer(next_buf_num);
        }
        if(infrared_tim_tx.signal_sent_callback && infrared_tim_tx.buffer[buf_num].packet_end &&
           (furi_hal_infrared_state != InfraredStateAsyncTxStopped)) {
            infrared_tim_tx.signal_sent_callback(infrared_tim_tx.signal_sent_context);
        }
    }
}

static void furi_hal_infrared_configure_tim_pwm_tx(uint32_t freq, float duty_cycle) {
    /*    LL_DBGMCU_APB2_GRP1_FreezePeriph(LL_DBGMCU_APB2_GRP1_TIM1_STOP); */

    LL_TIM_DisableCounter(TIM1);
    LL_TIM_SetRepetitionCounter(TIM1, 0);
    LL_TIM_SetCounter(TIM1, 0);
    LL_TIM_SetPrescaler(TIM1, 0);
    LL_TIM_SetCounterMode(TIM1, LL_TIM_COUNTERMODE_UP);
    LL_TIM_EnableARRPreload(TIM1);
    LL_TIM_SetAutoReload(
        TIM1, __LL_TIM_CALC_ARR(SystemCoreClock, LL_TIM_GetPrescaler(TIM1), freq));
#if INFRARED_TX_DEBUG == 1
    LL_TIM_OC_SetCompareCH1(TIM1, ((LL_TIM_GetAutoReload(TIM1) + 1) * (1 - duty_cycle)));
    LL_TIM_OC_EnablePreload(TIM1, LL_TIM_CHANNEL_CH1);
    /* LL_TIM_OCMODE_PWM2 set by DMA */
    LL_TIM_OC_SetMode(TIM1, LL_TIM_CHANNEL_CH1, LL_TIM_OCMODE_FORCED_INACTIVE);
    LL_TIM_OC_SetPolarity(TIM1, LL_TIM_CHANNEL_CH1N, LL_TIM_OCPOLARITY_HIGH);
    LL_TIM_OC_DisableFast(TIM1, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH1N);
    LL_TIM_DisableIT_CC1(TIM1);
#else
    LL_TIM_OC_SetCompareCH3(TIM1, ((LL_TIM_GetAutoReload(TIM1) + 1) * (1 - duty_cycle)));
    LL_TIM_OC_EnablePreload(TIM1, LL_TIM_CHANNEL_CH3);
    /* LL_TIM_OCMODE_PWM2 set by DMA */
    LL_TIM_OC_SetMode(TIM1, LL_TIM_CHANNEL_CH3, LL_TIM_OCMODE_FORCED_INACTIVE);
    LL_TIM_OC_SetPolarity(TIM1, LL_TIM_CHANNEL_CH3N, LL_TIM_OCPOLARITY_HIGH);
    LL_TIM_OC_DisableFast(TIM1, LL_TIM_CHANNEL_CH3);
    LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH3N);
    LL_TIM_DisableIT_CC3(TIM1);
#endif
    LL_TIM_DisableMasterSlaveMode(TIM1);
    LL_TIM_EnableAllOutputs(TIM1);
    LL_TIM_DisableIT_UPDATE(TIM1);
    LL_TIM_EnableDMAReq_UPDATE(TIM1);

    NVIC_SetPriority(TIM1_UP_TIM16_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);
}

static void furi_hal_infrared_configure_tim_cmgr2_dma_tx(void) {
    LL_DMA_InitTypeDef dma_config = {0};
#if INFRARED_TX_DEBUG == 1
    dma_config.PeriphOrM2MSrcAddress = (uint32_t) & (TIM1->CCMR1);
#else
    dma_config.PeriphOrM2MSrcAddress = (uint32_t) & (TIM1->CCMR2);
#endif
    dma_config.MemoryOrM2MDstAddress = (uint32_t)NULL;
    dma_config.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
    dma_config.Mode = LL_DMA_MODE_NORMAL;
    dma_config.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
    dma_config.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
    /* fill word to have other bits set to 0 */
    dma_config.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_WORD;
    dma_config.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE;
    dma_config.NbData = 0;
    dma_config.PeriphRequest = LL_DMAMUX_REQ_TIM1_UP;
    dma_config.Priority = LL_DMA_PRIORITY_VERYHIGH;
    LL_DMA_Init(DMA1, LL_DMA_CHANNEL_1, &dma_config);

    LL_DMA_ClearFlag_TE1(DMA1);
    LL_DMA_ClearFlag_TC1(DMA1);

    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_1);
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_1);

    furi_hal_interrupt_set_isr_ex(
        FuriHalInterruptIdDma1Ch1, 4, furi_hal_infrared_tx_dma_polarity_isr, NULL);
}

static void furi_hal_infrared_configure_tim_rcr_dma_tx(void) {
    LL_DMA_InitTypeDef dma_config = {0};
    dma_config.PeriphOrM2MSrcAddress = (uint32_t) & (TIM1->RCR);
    dma_config.MemoryOrM2MDstAddress = (uint32_t)NULL;
    dma_config.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
    dma_config.Mode = LL_DMA_MODE_NORMAL;
    dma_config.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
    dma_config.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
    dma_config.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_HALFWORD;
    dma_config.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_HALFWORD;
    dma_config.NbData = 0;
    dma_config.PeriphRequest = LL_DMAMUX_REQ_TIM1_UP;
    dma_config.Priority = LL_DMA_PRIORITY_MEDIUM;
    LL_DMA_Init(DMA1, LL_DMA_CHANNEL_2, &dma_config);

    LL_DMA_ClearFlag_TC2(DMA1);
    LL_DMA_ClearFlag_HT2(DMA1);
    LL_DMA_ClearFlag_TE2(DMA1);

    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_2);
    LL_DMA_EnableIT_HT(DMA1, LL_DMA_CHANNEL_2);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_2);

    furi_hal_interrupt_set_isr_ex(
        FuriHalInterruptIdDma1Ch2, 5, furi_hal_infrared_tx_dma_isr, NULL);
}

static void furi_hal_infrared_tx_fill_buffer_last(uint8_t buf_num) {
    furi_assert(buf_num < 2);
    furi_assert(furi_hal_infrared_state != InfraredStateAsyncRx);
    furi_assert(furi_hal_infrared_state < InfraredStateMAX);
    furi_assert(infrared_tim_tx.data_callback);
    InfraredTxBuf* buffer = &infrared_tim_tx.buffer[buf_num];
    furi_assert(buffer->data != NULL);
    (void)buffer->data;
    furi_assert(buffer->polarity != NULL);
    (void)buffer->polarity;

    infrared_tim_tx.buffer[buf_num].data[0] = 0; // 1 pulse
    infrared_tim_tx.buffer[buf_num].polarity[0] = INFRARED_TX_CCMR_LOW;
    infrared_tim_tx.buffer[buf_num].data[1] = 0; // 1 pulse
    infrared_tim_tx.buffer[buf_num].polarity[1] = INFRARED_TX_CCMR_LOW;
    infrared_tim_tx.buffer[buf_num].size = 2;
    infrared_tim_tx.buffer[buf_num].last_packet_end = true;
    infrared_tim_tx.buffer[buf_num].packet_end = true;
}

static void furi_hal_infrared_tx_fill_buffer(uint8_t buf_num, uint8_t polarity_shift) {
    furi_assert(buf_num < 2);
    furi_assert(furi_hal_infrared_state != InfraredStateAsyncRx);
    furi_assert(furi_hal_infrared_state < InfraredStateMAX);
    furi_assert(infrared_tim_tx.data_callback);
    InfraredTxBuf* buffer = &infrared_tim_tx.buffer[buf_num];
    furi_assert(buffer->data != NULL);
    furi_assert(buffer->polarity != NULL);

    FuriHalInfraredTxGetDataState status = FuriHalInfraredTxGetDataStateOk;
    uint32_t duration = 0;
    bool level = 0;
    size_t* size = &buffer->size;
    size_t polarity_counter = 0;
    while(polarity_shift--) {
        buffer->polarity[polarity_counter++] = INFRARED_TX_CCMR_LOW;
    }

    for(*size = 0; (*size < INFRARED_TIM_TX_DMA_BUFFER_SIZE) &&
                   (status == FuriHalInfraredTxGetDataStateOk);) {
        if(infrared_tim_tx.tx_timing_rest_duration > 0) {
            if(infrared_tim_tx.tx_timing_rest_duration > 0xFFFF) {
                buffer->data[*size] = 0xFFFF;
                status = FuriHalInfraredTxGetDataStateOk;
            } else {
                buffer->data[*size] = infrared_tim_tx.tx_timing_rest_duration;
                status = infrared_tim_tx.tx_timing_rest_status;
            }
            infrared_tim_tx.tx_timing_rest_duration -= buffer->data[*size];
            buffer->polarity[polarity_counter] = infrared_tim_tx.tx_timing_rest_level ?
                                                     INFRARED_TX_CCMR_HIGH :
                                                     INFRARED_TX_CCMR_LOW;
            ++(*size);
            ++polarity_counter;
            continue;
        }

        status = infrared_tim_tx.data_callback(infrared_tim_tx.data_context, &duration, &level);

        uint32_t num_of_impulses = roundf(duration / infrared_tim_tx.cycle_duration);

        if(num_of_impulses == 0) {
            if((*size == 0) && (status == FuriHalInfraredTxGetDataStateDone)) {
                /* if this is one sample in current buffer, but we
                 * have more to send - continue
                 */
                status = FuriHalInfraredTxGetDataStateOk;
            }
        } else if((num_of_impulses - 1) > 0xFFFF) {
            infrared_tim_tx.tx_timing_rest_duration = num_of_impulses - 1;
            infrared_tim_tx.tx_timing_rest_status = status;
            infrared_tim_tx.tx_timing_rest_level = level;
            status = FuriHalInfraredTxGetDataStateOk;
        } else {
            buffer->polarity[polarity_counter] = level ? INFRARED_TX_CCMR_HIGH :
                                                         INFRARED_TX_CCMR_LOW;
            buffer->data[*size] = num_of_impulses - 1;
            ++(*size);
            ++polarity_counter;
        }
    }

    buffer->last_packet_end = (status == FuriHalInfraredTxGetDataStateLastDone);
    buffer->packet_end = buffer->last_packet_end || (status == FuriHalInfraredTxGetDataStateDone);

    if(*size == 0) {
        buffer->data[0] = 0; // 1 pulse
        buffer->polarity[0] = INFRARED_TX_CCMR_LOW;
        buffer->size = 1;
    }
}

static void furi_hal_infrared_tx_dma_set_polarity(uint8_t buf_num, uint8_t polarity_shift) {
    furi_assert(buf_num < 2);
    furi_assert(furi_hal_infrared_state < InfraredStateMAX);
    InfraredTxBuf* buffer = &infrared_tim_tx.buffer[buf_num];
    furi_assert(buffer->polarity != NULL);

    FURI_CRITICAL_ENTER();
    bool channel_enabled = LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNEL_1);
    if(channel_enabled) {
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_1);
    }
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_1, (uint32_t)buffer->polarity);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_1, buffer->size + polarity_shift);
    if(channel_enabled) {
        LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);
    }
    FURI_CRITICAL_EXIT();
}

static void furi_hal_infrared_tx_dma_set_buffer(uint8_t buf_num) {
    furi_assert(buf_num < 2);
    furi_assert(furi_hal_infrared_state < InfraredStateMAX);
    InfraredTxBuf* buffer = &infrared_tim_tx.buffer[buf_num];
    furi_assert(buffer->data != NULL);

    /* non-circular mode requires disabled channel before setup */
    FURI_CRITICAL_ENTER();
    bool channel_enabled = LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNEL_2);
    if(channel_enabled) {
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
    }
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_2, (uint32_t)buffer->data);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, buffer->size);
    if(channel_enabled) {
        LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);
    }
    FURI_CRITICAL_EXIT();
}

static void furi_hal_infrared_async_tx_free_resources(void) {
    furi_assert(
        (furi_hal_infrared_state == InfraredStateIdle) ||
        (furi_hal_infrared_state == InfraredStateAsyncTxStopped));

    furi_hal_gpio_init(&gpio_infrared_tx, GpioModeOutputOpenDrain, GpioPullDown, GpioSpeedLow);
    furi_hal_interrupt_set_isr(FuriHalInterruptIdDma1Ch1, NULL, NULL);
    furi_hal_interrupt_set_isr(FuriHalInterruptIdDma1Ch2, NULL, NULL);
    LL_TIM_DeInit(TIM1);

    furi_semaphore_free(infrared_tim_tx.stop_semaphore);
    free(infrared_tim_tx.buffer[0].data);
    free(infrared_tim_tx.buffer[1].data);
    free(infrared_tim_tx.buffer[0].polarity);
    free(infrared_tim_tx.buffer[1].polarity);

    infrared_tim_tx.buffer[0].data = NULL;
    infrared_tim_tx.buffer[1].data = NULL;
    infrared_tim_tx.buffer[0].polarity = NULL;
    infrared_tim_tx.buffer[1].polarity = NULL;
}

void furi_hal_infrared_async_tx_start(uint32_t freq, float duty_cycle) {
    if((duty_cycle > 1) || (duty_cycle <= 0) || (freq > INFRARED_MAX_FREQUENCY) ||
       (freq < INFRARED_MIN_FREQUENCY) || (infrared_tim_tx.data_callback == NULL)) {
        furi_crash(NULL);
    }

    furi_assert(furi_hal_infrared_state == InfraredStateIdle);
    furi_assert(infrared_tim_tx.buffer[0].data == NULL);
    furi_assert(infrared_tim_tx.buffer[1].data == NULL);
    furi_assert(infrared_tim_tx.buffer[0].polarity == NULL);
    furi_assert(infrared_tim_tx.buffer[1].polarity == NULL);

    size_t alloc_size_data = INFRARED_TIM_TX_DMA_BUFFER_SIZE * sizeof(uint16_t);
    infrared_tim_tx.buffer[0].data = malloc(alloc_size_data);
    infrared_tim_tx.buffer[1].data = malloc(alloc_size_data);

    size_t alloc_size_polarity =
        (INFRARED_TIM_TX_DMA_BUFFER_SIZE + INFRARED_POLARITY_SHIFT) * sizeof(uint8_t);
    infrared_tim_tx.buffer[0].polarity = malloc(alloc_size_polarity);
    infrared_tim_tx.buffer[1].polarity = malloc(alloc_size_polarity);

    infrared_tim_tx.stop_semaphore = furi_semaphore_alloc(1, 0);
    infrared_tim_tx.cycle_duration = 1000000.0 / freq;
    infrared_tim_tx.tx_timing_rest_duration = 0;

    furi_hal_infrared_tx_fill_buffer(0, INFRARED_POLARITY_SHIFT);

    furi_hal_infrared_configure_tim_pwm_tx(freq, duty_cycle);
    furi_hal_infrared_configure_tim_cmgr2_dma_tx();
    furi_hal_infrared_configure_tim_rcr_dma_tx();
    furi_hal_infrared_tx_dma_set_polarity(0, INFRARED_POLARITY_SHIFT);
    furi_hal_infrared_tx_dma_set_buffer(0);

    furi_hal_infrared_state = InfraredStateAsyncTx;

    LL_TIM_ClearFlag_UPDATE(TIM1);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);
    furi_delay_us(5);
    LL_TIM_GenerateEvent_UPDATE(TIM1); /* DMA -> TIMx_RCR */
    furi_delay_us(5);
    LL_GPIO_ResetOutputPin(
        gpio_infrared_tx.port, gpio_infrared_tx.pin); /* when disable it prevents false pulse */
    furi_hal_gpio_init_ex(
        &gpio_infrared_tx, GpioModeAltFunctionPushPull, GpioPullUp, GpioSpeedHigh, GpioAltFn1TIM1);

    FURI_CRITICAL_ENTER();
    LL_TIM_GenerateEvent_UPDATE(TIM1); /* TIMx_RCR -> Repetition counter */
    LL_TIM_EnableCounter(TIM1);
    FURI_CRITICAL_EXIT();
}

void furi_hal_infrared_async_tx_wait_termination(void) {
    furi_assert(furi_hal_infrared_state >= InfraredStateAsyncTx);
    furi_assert(furi_hal_infrared_state < InfraredStateMAX);

    FuriStatus status;
    status = furi_semaphore_acquire(infrared_tim_tx.stop_semaphore, FuriWaitForever);
    furi_check(status == FuriStatusOk);
    furi_hal_infrared_async_tx_free_resources();
    furi_hal_infrared_state = InfraredStateIdle;
}

void furi_hal_infrared_async_tx_stop(void) {
    furi_assert(furi_hal_infrared_state >= InfraredStateAsyncTx);
    furi_assert(furi_hal_infrared_state < InfraredStateMAX);

    FURI_CRITICAL_ENTER();
    if(furi_hal_infrared_state == InfraredStateAsyncTx)
        furi_hal_infrared_state = InfraredStateAsyncTxStopReq;
    FURI_CRITICAL_EXIT();

    furi_hal_infrared_async_tx_wait_termination();
}

void furi_hal_infrared_async_tx_set_data_isr_callback(
    FuriHalInfraredTxGetDataISRCallback callback,
    void* context) {
    furi_assert(furi_hal_infrared_state == InfraredStateIdle);
    infrared_tim_tx.data_callback = callback;
    infrared_tim_tx.data_context = context;
}

void furi_hal_infrared_async_tx_set_signal_sent_isr_callback(
    FuriHalInfraredTxSignalSentISRCallback callback,
    void* context) {
    infrared_tim_tx.signal_sent_callback = callback;
    infrared_tim_tx.signal_sent_context = context;
}
