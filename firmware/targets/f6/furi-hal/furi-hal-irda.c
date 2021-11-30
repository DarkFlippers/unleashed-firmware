#include "furi-hal-irda.h"
#include "furi-hal-delay.h"
#include "furi/check.h"
#include "stm32wbxx_ll_dma.h"
#include "sys/_stdint.h"
#include <cmsis_os2.h>
#include <furi-hal-interrupt.h>
#include <furi-hal-resources.h>

#include <stdint.h>
#include <stm32wbxx_ll_tim.h>
#include <stm32wbxx_ll_gpio.h>

#include <stdio.h>
#include <furi.h>
#include <math.h>
#include <main.h>
#include <furi-hal-pwm.h>

#define IRDA_TX_DEBUG 0

#if IRDA_TX_DEBUG == 1
#define gpio_irda_tx gpio_irda_tx_debug
const GpioPin gpio_irda_tx_debug = {.port = GPIOA, .pin = GPIO_PIN_7};
#endif

#define IRDA_TIM_TX_DMA_BUFFER_SIZE         200
#define IRDA_POLARITY_SHIFT                 1

#define IRDA_TX_CCMR_HIGH    (TIM_CCMR2_OC3PE | LL_TIM_OCMODE_PWM2)              /* Mark time - enable PWM2 mode */
#define IRDA_TX_CCMR_LOW     (TIM_CCMR2_OC3PE | LL_TIM_OCMODE_FORCED_INACTIVE)   /* Space time - force low */

typedef struct{
    FuriHalIrdaRxCaptureCallback capture_callback;
    void *capture_context;
    FuriHalIrdaRxTimeoutCallback timeout_callback;
    void *timeout_context;
} IrdaTimRx;

typedef struct{
    uint8_t* polarity;
    uint16_t* data;
    size_t size;
    bool packet_end;
    bool last_packet_end;
} IrdaTxBuf;

typedef struct {
    float cycle_duration;
    FuriHalIrdaTxGetDataISRCallback data_callback;
    FuriHalIrdaTxSignalSentISRCallback signal_sent_callback;
    void* data_context;
    void* signal_sent_context;
    IrdaTxBuf buffer[2];
    osSemaphoreId_t stop_semaphore;
    uint32_t tx_timing_rest_duration;       /** if timing is too long (> 0xFFFF), send it in few iterations */
    bool tx_timing_rest_level;
    FuriHalIrdaTxGetDataState tx_timing_rest_status;
} IrdaTimTx;

typedef enum {
    IrdaStateIdle,                  /** Furi Hal Irda is ready to start RX or TX */
    IrdaStateAsyncRx,               /** Async RX started */
    IrdaStateAsyncTx,               /** Async TX started, DMA and timer is on */
    IrdaStateAsyncTxStopReq,        /** Async TX started, async stop request received */
    IrdaStateAsyncTxStopInProgress, /** Async TX started, stop request is processed and we wait for last data to be sent */
    IrdaStateAsyncTxStopped,        /** Async TX complete, cleanup needed */
    IrdaStateMAX,
} IrdaState;

static volatile IrdaState furi_hal_irda_state = IrdaStateIdle;
static IrdaTimTx irda_tim_tx;
static IrdaTimRx irda_tim_rx;

static void furi_hal_irda_tx_fill_buffer(uint8_t buf_num, uint8_t polarity_shift);
static void furi_hal_irda_async_tx_free_resources(void);
static void furi_hal_irda_tx_dma_set_polarity(uint8_t buf_num, uint8_t polarity_shift);
static void furi_hal_irda_tx_dma_set_buffer(uint8_t buf_num);
static void furi_hal_irda_tx_fill_buffer_last(uint8_t buf_num);
static uint8_t furi_hal_irda_get_current_dma_tx_buffer(void);
static void furi_hal_irda_tx_dma_polarity_isr();
static void furi_hal_irda_tx_dma_isr();

static void furi_hal_irda_tim_rx_isr() {
    static uint32_t previous_captured_ch2 = 0;

    /* Timeout */
    if(LL_TIM_IsActiveFlag_CC3(TIM2)) {
        LL_TIM_ClearFlag_CC3(TIM2);
        furi_assert(furi_hal_irda_state == IrdaStateAsyncRx);

        /* Timers CNT register starts to counting from 0 to ARR, but it is
         * reseted when Channel 1 catches interrupt. It is not reseted by
         * channel 2, though, so we have to distract it's values (see TimerIRQSourceCCI1 ISR).
         * This can cause false timeout: when time is over, but we started
         * receiving new signal few microseconds ago, because CNT register
         * is reseted once per period, not per sample. */
        if (LL_GPIO_IsInputPinSet(gpio_irda_rx.port, gpio_irda_rx.pin) != 0) {
            if (irda_tim_rx.timeout_callback)
                irda_tim_rx.timeout_callback(irda_tim_rx.timeout_context);
        }
    }

    /* Rising Edge */
    if(LL_TIM_IsActiveFlag_CC1(TIM2)) {
        LL_TIM_ClearFlag_CC1(TIM2);
        furi_assert(furi_hal_irda_state == IrdaStateAsyncRx);

        if(READ_BIT(TIM2->CCMR1, TIM_CCMR1_CC1S)) {
            /* Low pin level is a Mark state of IRDA signal. Invert level for further processing. */
            uint32_t duration = LL_TIM_IC_GetCaptureCH1(TIM2) - previous_captured_ch2;
            if (irda_tim_rx.capture_callback)
                irda_tim_rx.capture_callback(irda_tim_rx.capture_context, 1, duration);
        } else {
            furi_assert(0);
        }
    }

    /* Falling Edge */
    if(LL_TIM_IsActiveFlag_CC2(TIM2)) {
        LL_TIM_ClearFlag_CC2(TIM2);
        furi_assert(furi_hal_irda_state == IrdaStateAsyncRx);

        if(READ_BIT(TIM2->CCMR1, TIM_CCMR1_CC2S)) {
            /* High pin level is a Space state of IRDA signal. Invert level for further processing. */
            uint32_t duration = LL_TIM_IC_GetCaptureCH2(TIM2);
            previous_captured_ch2 = duration;
            if (irda_tim_rx.capture_callback)
                irda_tim_rx.capture_callback(irda_tim_rx.capture_context, 0, duration);
        } else {
            furi_assert(0);
        }
    }
}

void furi_hal_irda_async_rx_start(void) {
    furi_assert(furi_hal_irda_state == IrdaStateIdle);

    FURI_CRITICAL_ENTER();
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
    FURI_CRITICAL_EXIT();

    hal_gpio_init_ex(&gpio_irda_rx, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedLow, GpioAltFn1TIM2);

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

    furi_hal_interrupt_set_timer_isr(TIM2, furi_hal_irda_tim_rx_isr);
    furi_hal_irda_state = IrdaStateAsyncRx;

    LL_TIM_EnableIT_CC1(TIM2);
    LL_TIM_EnableIT_CC2(TIM2);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH2);

    LL_TIM_SetCounter(TIM2, 0);
    LL_TIM_EnableCounter(TIM2);

    NVIC_SetPriority(TIM2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(TIM2_IRQn);
}

void furi_hal_irda_async_rx_stop(void) {
    furi_assert(furi_hal_irda_state == IrdaStateAsyncRx);
    LL_TIM_DeInit(TIM2);
    furi_hal_interrupt_set_timer_isr(TIM2, NULL);
    LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_TIM2);
    furi_hal_irda_state = IrdaStateIdle;
}

void furi_hal_irda_async_rx_set_timeout(uint32_t timeout_us) {
    furi_assert(LL_APB1_GRP1_IsEnabledClock(LL_APB1_GRP1_PERIPH_TIM2));

    LL_TIM_OC_SetCompareCH3(TIM2, timeout_us);
    LL_TIM_OC_SetMode(TIM2, LL_TIM_CHANNEL_CH3, LL_TIM_OCMODE_ACTIVE);
    LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH3);
    LL_TIM_EnableIT_CC3(TIM2);
}

bool furi_hal_irda_is_busy(void) {
    return furi_hal_irda_state != IrdaStateIdle;
}

void furi_hal_irda_async_rx_set_capture_isr_callback(FuriHalIrdaRxCaptureCallback callback, void *ctx) {
    irda_tim_rx.capture_callback = callback;
    irda_tim_rx.capture_context = ctx;
}

void furi_hal_irda_async_rx_set_timeout_isr_callback(FuriHalIrdaRxTimeoutCallback callback, void *ctx) {
    irda_tim_rx.timeout_callback = callback;
    irda_tim_rx.timeout_context = ctx;
}

static void furi_hal_irda_tx_dma_terminate(void) {
    LL_DMA_DisableIT_TC(DMA1, LL_DMA_CHANNEL_1);
    LL_DMA_DisableIT_HT(DMA1, LL_DMA_CHANNEL_2);
    LL_DMA_DisableIT_TC(DMA1, LL_DMA_CHANNEL_2);

    furi_assert(furi_hal_irda_state == IrdaStateAsyncTxStopInProgress);

    LL_DMA_DisableIT_TC(DMA1, LL_DMA_CHANNEL_1);
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_1);
    LL_TIM_DisableCounter(TIM1);
    osStatus_t status = osSemaphoreRelease(irda_tim_tx.stop_semaphore);
    furi_check(status == osOK);
    furi_hal_irda_state = IrdaStateAsyncTxStopped;
}

static uint8_t furi_hal_irda_get_current_dma_tx_buffer(void) {
    uint8_t buf_num = 0;
    uint32_t buffer_adr = LL_DMA_GetMemoryAddress(DMA1, LL_DMA_CHANNEL_2);
    if (buffer_adr == (uint32_t) irda_tim_tx.buffer[0].data) {
        buf_num = 0;
    } else if (buffer_adr == (uint32_t) irda_tim_tx.buffer[1].data) {
        buf_num = 1;
    } else {
        furi_assert(0);
    }
    return buf_num;
}

static void furi_hal_irda_tx_dma_polarity_isr() {
    if (LL_DMA_IsActiveFlag_TE1(DMA1)) {
        LL_DMA_ClearFlag_TE1(DMA1);
        furi_crash(NULL);
    }
    if (LL_DMA_IsActiveFlag_TC1(DMA1) && LL_DMA_IsEnabledIT_TC(DMA1, LL_DMA_CHANNEL_1)) {
        LL_DMA_ClearFlag_TC1(DMA1);

        furi_check((furi_hal_irda_state == IrdaStateAsyncTx)
                    || (furi_hal_irda_state == IrdaStateAsyncTxStopReq)
                    || (furi_hal_irda_state == IrdaStateAsyncTxStopInProgress));
        /* actually TC2 is processed and buffer is next buffer */
        uint8_t next_buf_num = furi_hal_irda_get_current_dma_tx_buffer();
        furi_hal_irda_tx_dma_set_polarity(next_buf_num, 0);
    }
}

static void furi_hal_irda_tx_dma_isr() {
    if (LL_DMA_IsActiveFlag_TE2(DMA1)) {
        LL_DMA_ClearFlag_TE2(DMA1);
        furi_crash(NULL);
    }
    if (LL_DMA_IsActiveFlag_HT2(DMA1) && LL_DMA_IsEnabledIT_HT(DMA1, LL_DMA_CHANNEL_2)) {
        LL_DMA_ClearFlag_HT2(DMA1);
        uint8_t buf_num = furi_hal_irda_get_current_dma_tx_buffer();
        uint8_t next_buf_num = !buf_num;
        if (irda_tim_tx.buffer[buf_num].last_packet_end) {
            LL_DMA_DisableIT_HT(DMA1, LL_DMA_CHANNEL_2);
        } else if (!irda_tim_tx.buffer[buf_num].packet_end || (furi_hal_irda_state == IrdaStateAsyncTx)) {
            furi_hal_irda_tx_fill_buffer(next_buf_num, 0);
            if (irda_tim_tx.buffer[next_buf_num].last_packet_end) {
                LL_DMA_DisableIT_HT(DMA1, LL_DMA_CHANNEL_2);
            }
        } else if (furi_hal_irda_state == IrdaStateAsyncTxStopReq) {
            /* fallthrough */
        } else {
            furi_crash(NULL);
        }
    }
    if (LL_DMA_IsActiveFlag_TC2(DMA1) && LL_DMA_IsEnabledIT_TC(DMA1, LL_DMA_CHANNEL_2)) {
        LL_DMA_ClearFlag_TC2(DMA1);
        furi_check((furi_hal_irda_state == IrdaStateAsyncTxStopInProgress)
                    || (furi_hal_irda_state == IrdaStateAsyncTxStopReq)
                    || (furi_hal_irda_state == IrdaStateAsyncTx));

        uint8_t buf_num = furi_hal_irda_get_current_dma_tx_buffer();
        uint8_t next_buf_num = !buf_num;
        if (furi_hal_irda_state == IrdaStateAsyncTxStopInProgress) {
            furi_hal_irda_tx_dma_terminate();
        } else if (irda_tim_tx.buffer[buf_num].last_packet_end
           || (irda_tim_tx.buffer[buf_num].packet_end && (furi_hal_irda_state == IrdaStateAsyncTxStopReq))) {
            furi_hal_irda_state = IrdaStateAsyncTxStopInProgress;
            furi_hal_irda_tx_fill_buffer_last(next_buf_num);
            furi_hal_irda_tx_dma_set_buffer(next_buf_num);
        } else {
            /* if it's not end of the packet - continue receiving */
            furi_hal_irda_tx_dma_set_buffer(next_buf_num);
        }
        if (irda_tim_tx.signal_sent_callback && irda_tim_tx.buffer[buf_num].packet_end && (furi_hal_irda_state != IrdaStateAsyncTxStopped)) {
            irda_tim_tx.signal_sent_callback(irda_tim_tx.signal_sent_context);
        }
    }
}

static void furi_hal_irda_configure_tim_pwm_tx(uint32_t freq, float duty_cycle)
{
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1);
/*    LL_DBGMCU_APB2_GRP1_FreezePeriph(LL_DBGMCU_APB2_GRP1_TIM1_STOP); */

    LL_TIM_DisableCounter(TIM1);
    LL_TIM_SetRepetitionCounter(TIM1, 0);
    LL_TIM_SetCounter(TIM1, 0);
    LL_TIM_SetPrescaler(TIM1, 0);
    LL_TIM_SetCounterMode(TIM1, LL_TIM_COUNTERMODE_UP);
    LL_TIM_EnableARRPreload(TIM1);
    LL_TIM_SetAutoReload(TIM1, __LL_TIM_CALC_ARR(SystemCoreClock, LL_TIM_GetPrescaler(TIM1), freq));
#if IRDA_TX_DEBUG == 1
    LL_TIM_OC_SetCompareCH1(TIM1, ( (LL_TIM_GetAutoReload(TIM1) + 1 ) * (1 - duty_cycle)));
    LL_TIM_OC_EnablePreload(TIM1, LL_TIM_CHANNEL_CH1);
    /* LL_TIM_OCMODE_PWM2 set by DMA */
    LL_TIM_OC_SetMode(TIM1, LL_TIM_CHANNEL_CH1, LL_TIM_OCMODE_FORCED_INACTIVE);
    LL_TIM_OC_SetPolarity(TIM1, LL_TIM_CHANNEL_CH1N, LL_TIM_OCPOLARITY_HIGH);
    LL_TIM_OC_DisableFast(TIM1, LL_TIM_CHANNEL_CH1);
    LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH1N);
    LL_TIM_DisableIT_CC1(TIM1);
#else
    LL_TIM_OC_SetCompareCH3(TIM1, ( (LL_TIM_GetAutoReload(TIM1) + 1 ) * (1 - duty_cycle)));
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

static void furi_hal_irda_configure_tim_cmgr2_dma_tx(void) {
    LL_C2_AHB1_GRP1_EnableClock(LL_C2_AHB1_GRP1_PERIPH_DMA1);

    LL_DMA_InitTypeDef dma_config = {0};
#if IRDA_TX_DEBUG == 1
    dma_config.PeriphOrM2MSrcAddress = (uint32_t)&(TIM1->CCMR1);
#else
    dma_config.PeriphOrM2MSrcAddress = (uint32_t)&(TIM1->CCMR2);
#endif
    dma_config.MemoryOrM2MDstAddress = (uint32_t) NULL;
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
    furi_hal_interrupt_set_dma_channel_isr(DMA1, LL_DMA_CHANNEL_1, furi_hal_irda_tx_dma_polarity_isr);
    LL_DMA_ClearFlag_TE1(DMA1);
    LL_DMA_ClearFlag_TC1(DMA1);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_1);
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_1);

    NVIC_SetPriority(DMA1_Channel1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 4, 0));
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}

static void furi_hal_irda_configure_tim_rcr_dma_tx(void) {
    LL_C2_AHB1_GRP1_EnableClock(LL_C2_AHB1_GRP1_PERIPH_DMA1);

    LL_DMA_InitTypeDef dma_config = {0};
    dma_config.PeriphOrM2MSrcAddress = (uint32_t)&(TIM1->RCR);
    dma_config.MemoryOrM2MDstAddress = (uint32_t) NULL;
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
    furi_hal_interrupt_set_dma_channel_isr(DMA1, LL_DMA_CHANNEL_2, furi_hal_irda_tx_dma_isr);
    LL_DMA_ClearFlag_TC2(DMA1);
    LL_DMA_ClearFlag_HT2(DMA1);
    LL_DMA_ClearFlag_TE2(DMA1);
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_2);
    LL_DMA_EnableIT_HT(DMA1, LL_DMA_CHANNEL_2);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_2);

    NVIC_SetPriority(DMA1_Channel2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(DMA1_Channel2_IRQn);
}

static void furi_hal_irda_tx_fill_buffer_last(uint8_t buf_num) {
    furi_assert(buf_num < 2);
    furi_assert(furi_hal_irda_state != IrdaStateAsyncRx);
    furi_assert(furi_hal_irda_state < IrdaStateMAX);
    furi_assert(irda_tim_tx.data_callback);
    IrdaTxBuf* buffer = &irda_tim_tx.buffer[buf_num];
    furi_assert(buffer->data != NULL);
    (void)buffer->data;
    furi_assert(buffer->polarity != NULL);
    (void)buffer->polarity;

    irda_tim_tx.buffer[buf_num].data[0] = 0;       // 1 pulse
    irda_tim_tx.buffer[buf_num].polarity[0] = IRDA_TX_CCMR_LOW;
    irda_tim_tx.buffer[buf_num].data[1] = 0;       // 1 pulse
    irda_tim_tx.buffer[buf_num].polarity[1] = IRDA_TX_CCMR_LOW;
    irda_tim_tx.buffer[buf_num].size = 2;
    irda_tim_tx.buffer[buf_num].last_packet_end = true;
    irda_tim_tx.buffer[buf_num].packet_end = true;
}

static void furi_hal_irda_tx_fill_buffer(uint8_t buf_num, uint8_t polarity_shift) {
    furi_assert(buf_num < 2);
    furi_assert(furi_hal_irda_state != IrdaStateAsyncRx);
    furi_assert(furi_hal_irda_state < IrdaStateMAX);
    furi_assert(irda_tim_tx.data_callback);
    IrdaTxBuf* buffer = &irda_tim_tx.buffer[buf_num];
    furi_assert(buffer->data != NULL);
    furi_assert(buffer->polarity != NULL);

    FuriHalIrdaTxGetDataState status = FuriHalIrdaTxGetDataStateOk;
    uint32_t duration = 0;
    bool level = 0;
    size_t *size = &buffer->size;
    size_t polarity_counter = 0;
    while (polarity_shift--) {
        buffer->polarity[polarity_counter++] = IRDA_TX_CCMR_LOW;
    }

    for (*size = 0; (*size < IRDA_TIM_TX_DMA_BUFFER_SIZE) && (status == FuriHalIrdaTxGetDataStateOk);) {
        if (irda_tim_tx.tx_timing_rest_duration > 0) {
            if (irda_tim_tx.tx_timing_rest_duration > 0xFFFF) {
                buffer->data[*size] = 0xFFFF;
                status = FuriHalIrdaTxGetDataStateOk;
            } else {
                buffer->data[*size] = irda_tim_tx.tx_timing_rest_duration;
                status = irda_tim_tx.tx_timing_rest_status;
            }
            irda_tim_tx.tx_timing_rest_duration -= buffer->data[*size];
            buffer->polarity[polarity_counter] = irda_tim_tx.tx_timing_rest_level ? IRDA_TX_CCMR_HIGH : IRDA_TX_CCMR_LOW;
            ++(*size);
            ++polarity_counter;
            continue;
        }

        status = irda_tim_tx.data_callback(irda_tim_tx.data_context, &duration, &level);

        uint32_t num_of_impulses = roundf(duration / irda_tim_tx.cycle_duration);

        if (num_of_impulses == 0) {
            if ((*size == 0) && (status == FuriHalIrdaTxGetDataStateDone)) {
                /* if this is one sample in current buffer, but we
                 * have more to send - continue
                 */
                status = FuriHalIrdaTxGetDataStateOk;
            }
        } else if ((num_of_impulses - 1) > 0xFFFF) {
            irda_tim_tx.tx_timing_rest_duration = num_of_impulses - 1;
            irda_tim_tx.tx_timing_rest_status = status;
            irda_tim_tx.tx_timing_rest_level = level;
            status = FuriHalIrdaTxGetDataStateOk;
        } else {
            buffer->polarity[polarity_counter] = level ? IRDA_TX_CCMR_HIGH : IRDA_TX_CCMR_LOW;
            buffer->data[*size] = num_of_impulses - 1;
            ++(*size);
            ++polarity_counter;
        }
    }

    buffer->last_packet_end = (status == FuriHalIrdaTxGetDataStateLastDone);
    buffer->packet_end = buffer->last_packet_end || (status == FuriHalIrdaTxGetDataStateDone);

    if (*size == 0) {
        buffer->data[0] = 0;       // 1 pulse
        buffer->polarity[0] = IRDA_TX_CCMR_LOW;
        buffer->size = 1;
    }
}

static void furi_hal_irda_tx_dma_set_polarity(uint8_t buf_num, uint8_t polarity_shift) {
    furi_assert(buf_num < 2);
    furi_assert(furi_hal_irda_state < IrdaStateMAX);
    IrdaTxBuf* buffer = &irda_tim_tx.buffer[buf_num];
    furi_assert(buffer->polarity != NULL);

    __disable_irq();
    bool channel_enabled = LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNEL_1);
    if (channel_enabled) {
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_1);
    }
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_1, (uint32_t) buffer->polarity);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_1, buffer->size + polarity_shift);
    if (channel_enabled) {
        LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);
    }
    __enable_irq();
}

static void furi_hal_irda_tx_dma_set_buffer(uint8_t buf_num) {
    furi_assert(buf_num < 2);
    furi_assert(furi_hal_irda_state < IrdaStateMAX);
    IrdaTxBuf* buffer = &irda_tim_tx.buffer[buf_num];
    furi_assert(buffer->data != NULL);

    /* non-circular mode requires disabled channel before setup */
    __disable_irq();
    bool channel_enabled = LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNEL_2);
    if (channel_enabled) {
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
    }
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_2, (uint32_t)buffer->data);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, buffer->size);
    if (channel_enabled) {
        LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);
    }
    __enable_irq();
}

static void furi_hal_irda_async_tx_free_resources(void) {
    furi_assert((furi_hal_irda_state == IrdaStateIdle) || (furi_hal_irda_state == IrdaStateAsyncTxStopped));
    osStatus_t status;

    hal_gpio_init(&gpio_irda_tx, GpioModeOutputOpenDrain, GpioPullDown, GpioSpeedLow);
    furi_hal_interrupt_set_dma_channel_isr(DMA1, LL_DMA_CHANNEL_1, NULL);
    furi_hal_interrupt_set_dma_channel_isr(DMA1, LL_DMA_CHANNEL_2, NULL);
    LL_TIM_DeInit(TIM1);
    LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_TIM1);
    LL_C2_AHB1_GRP1_DisableClock(LL_C2_AHB1_GRP1_PERIPH_DMA1);

    status = osSemaphoreDelete(irda_tim_tx.stop_semaphore);
    furi_check(status == osOK);
    free(irda_tim_tx.buffer[0].data);
    free(irda_tim_tx.buffer[1].data);
    free(irda_tim_tx.buffer[0].polarity);
    free(irda_tim_tx.buffer[1].polarity);

    irda_tim_tx.buffer[0].data = NULL;
    irda_tim_tx.buffer[1].data = NULL;
    irda_tim_tx.buffer[0].polarity = NULL;
    irda_tim_tx.buffer[1].polarity = NULL;
}

void furi_hal_irda_async_tx_start(uint32_t freq, float duty_cycle) {
    if ((duty_cycle > 1) || (duty_cycle <= 0) || (freq > IRDA_MAX_FREQUENCY) || (freq < IRDA_MIN_FREQUENCY) || (irda_tim_tx.data_callback == NULL)) {
        furi_crash(NULL);
    }

    furi_assert(furi_hal_irda_state == IrdaStateIdle);
    furi_assert(irda_tim_tx.buffer[0].data == NULL);
    furi_assert(irda_tim_tx.buffer[1].data == NULL);
    furi_assert(irda_tim_tx.buffer[0].polarity == NULL);
    furi_assert(irda_tim_tx.buffer[1].polarity == NULL);

    size_t alloc_size_data = IRDA_TIM_TX_DMA_BUFFER_SIZE * sizeof(uint16_t);
    irda_tim_tx.buffer[0].data = furi_alloc(alloc_size_data);
    irda_tim_tx.buffer[1].data = furi_alloc(alloc_size_data);

    size_t alloc_size_polarity = (IRDA_TIM_TX_DMA_BUFFER_SIZE + IRDA_POLARITY_SHIFT) * sizeof(uint8_t);
    irda_tim_tx.buffer[0].polarity = furi_alloc(alloc_size_polarity);
    irda_tim_tx.buffer[1].polarity = furi_alloc(alloc_size_polarity);

    irda_tim_tx.stop_semaphore = osSemaphoreNew(1, 0, NULL);
    irda_tim_tx.cycle_duration = 1000000.0 / freq;
    irda_tim_tx.tx_timing_rest_duration = 0;

    furi_hal_irda_tx_fill_buffer(0, IRDA_POLARITY_SHIFT);

    furi_hal_irda_configure_tim_pwm_tx(freq, duty_cycle);
    furi_hal_irda_configure_tim_cmgr2_dma_tx();
    furi_hal_irda_configure_tim_rcr_dma_tx();
    furi_hal_irda_tx_dma_set_polarity(0, IRDA_POLARITY_SHIFT);
    furi_hal_irda_tx_dma_set_buffer(0);

    furi_hal_irda_state = IrdaStateAsyncTx;

    LL_TIM_ClearFlag_UPDATE(TIM1);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);
    delay_us(5);
    LL_TIM_GenerateEvent_UPDATE(TIM1);  /* DMA -> TIMx_RCR */
    delay_us(5);
    LL_GPIO_ResetOutputPin(gpio_irda_tx.port, gpio_irda_tx.pin);    /* when disable it prevents false pulse */
    hal_gpio_init_ex(&gpio_irda_tx, GpioModeAltFunctionPushPull, GpioPullUp, GpioSpeedHigh, GpioAltFn1TIM1);

    __disable_irq();
    LL_TIM_GenerateEvent_UPDATE(TIM1);  /* TIMx_RCR -> Repetition counter */
    LL_TIM_EnableCounter(TIM1);
    __enable_irq();
}

void furi_hal_irda_async_tx_wait_termination(void) {
    furi_assert(furi_hal_irda_state >= IrdaStateAsyncTx);
    furi_assert(furi_hal_irda_state < IrdaStateMAX);

    osStatus_t status;
    status = osSemaphoreAcquire(irda_tim_tx.stop_semaphore, osWaitForever);
    furi_check(status == osOK);
    furi_hal_irda_async_tx_free_resources();
    furi_hal_irda_state = IrdaStateIdle;
}

void furi_hal_irda_async_tx_stop(void) {
    furi_assert(furi_hal_irda_state >= IrdaStateAsyncTx);
    furi_assert(furi_hal_irda_state < IrdaStateMAX);

    __disable_irq();
    if (furi_hal_irda_state == IrdaStateAsyncTx)
        furi_hal_irda_state = IrdaStateAsyncTxStopReq;
    __enable_irq();

    furi_hal_irda_async_tx_wait_termination();
}

void furi_hal_irda_async_tx_set_data_isr_callback(FuriHalIrdaTxGetDataISRCallback callback, void* context) {
    furi_assert(furi_hal_irda_state == IrdaStateIdle);
    irda_tim_tx.data_callback = callback;
    irda_tim_tx.data_context = context;
}

void furi_hal_irda_async_tx_set_signal_sent_isr_callback(FuriHalIrdaTxSignalSentISRCallback callback, void* context) {
    irda_tim_tx.signal_sent_callback = callback;
    irda_tim_tx.signal_sent_context = context;
}

