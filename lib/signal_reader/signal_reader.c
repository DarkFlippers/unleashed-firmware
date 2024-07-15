#include "signal_reader.h"

#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_gpio.h>

#include <stm32wbxx_ll_dma.h>
#include <stm32wbxx_ll_dmamux.h>
#include <stm32wbxx_ll_tim.h>
#include <stm32wbxx_ll_exti.h>

#include <furi_hal_bus.h>

#define SIGNAL_READER_DMA DMA2

#define SIGNAL_READER_CAPTURE_TIM         (TIM16)
#define SIGNAL_READER_CAPTURE_TIM_CHANNEL LL_TIM_CHANNEL_CH1

#define SIGNAL_READER_DMA_GPIO     LL_DMA_CHANNEL_2
#define SIGNAL_READER_DMA_GPIO_IRQ FuriHalInterruptIdDma2Ch2
#define SIGNAL_READER_DMA_GPIO_DEF SIGNAL_READER_DMA, SIGNAL_READER_DMA_GPIO

#define SIGNAL_READER_DMA_TRIGGER     LL_DMA_CHANNEL_3
#define SIGNAL_READER_DMA_TRIGGER_IRQ FuriHalInterruptIdDma2Ch3
#define SIGNAL_READER_DMA_TRIGGER_DEF SIGNAL_READER_DMA, SIGNAL_READER_DMA_TRIGGER

#define SIGNAL_READER_DMA_CNT_SYNC     LL_DMA_CHANNEL_5
#define SIGNAL_READER_DMA_CNT_SYNC_IRQ FuriHalInterruptIdDma2Ch5
#define SIGNAL_READER_DMA_CNT_SYNC_DEF SIGNAL_READER_DMA, SIGNAL_READER_DMA_CNT_SYNC

struct SignalReader {
    size_t buffer_size;
    const GpioPin* pin;
    GpioPull pull;
    SignalReaderPolarity polarity;
    SignalReaderTrigger trigger;

    uint16_t* gpio_buffer;
    uint8_t* bitstream_buffer;
    uint32_t cnt_en;

    uint32_t tim_cnt_compensation;
    uint32_t tim_arr;

    SignalReaderEvent event;
    SignalReaderEventData event_data;

    SignalReaderCallback callback;
    void* context;
};

#define GPIO_PIN_MAP(pin, prefix)               \
    (((pin) == (LL_GPIO_PIN_0))  ? prefix##0 :  \
     ((pin) == (LL_GPIO_PIN_1))  ? prefix##1 :  \
     ((pin) == (LL_GPIO_PIN_2))  ? prefix##2 :  \
     ((pin) == (LL_GPIO_PIN_3))  ? prefix##3 :  \
     ((pin) == (LL_GPIO_PIN_4))  ? prefix##4 :  \
     ((pin) == (LL_GPIO_PIN_5))  ? prefix##5 :  \
     ((pin) == (LL_GPIO_PIN_6))  ? prefix##6 :  \
     ((pin) == (LL_GPIO_PIN_7))  ? prefix##7 :  \
     ((pin) == (LL_GPIO_PIN_8))  ? prefix##8 :  \
     ((pin) == (LL_GPIO_PIN_9))  ? prefix##9 :  \
     ((pin) == (LL_GPIO_PIN_10)) ? prefix##10 : \
     ((pin) == (LL_GPIO_PIN_11)) ? prefix##11 : \
     ((pin) == (LL_GPIO_PIN_12)) ? prefix##12 : \
     ((pin) == (LL_GPIO_PIN_13)) ? prefix##13 : \
     ((pin) == (LL_GPIO_PIN_14)) ? prefix##14 : \
                                   prefix##15)

#define GET_DMAMUX_EXTI_LINE(pin) GPIO_PIN_MAP(pin, LL_DMAMUX_REQ_GEN_EXTI_LINE)

SignalReader* signal_reader_alloc(const GpioPin* gpio_pin, uint32_t size) {
    SignalReader* instance = malloc(sizeof(SignalReader));

    instance->pin = gpio_pin;
    instance->pull = GpioPullNo;

    instance->buffer_size = size;
    instance->gpio_buffer = malloc(sizeof(uint16_t) * size * 8);
    instance->bitstream_buffer = malloc(size);

    instance->event.data = &instance->event_data;

    return instance;
}

void signal_reader_free(SignalReader* instance) {
    furi_check(instance);
    furi_check(instance->gpio_buffer);
    furi_check(instance->bitstream_buffer);

    free(instance->gpio_buffer);
    free(instance->bitstream_buffer);
    free(instance);
}

void signal_reader_set_pull(SignalReader* instance, GpioPull pull) {
    furi_check(instance);

    instance->pull = pull;
}

void signal_reader_set_polarity(SignalReader* instance, SignalReaderPolarity polarity) {
    furi_check(instance);

    instance->polarity = polarity;
}

void signal_reader_set_sample_rate(
    SignalReader* instance,
    SignalReaderTimeUnit time_unit,
    uint32_t time) {
    furi_check(instance);
    UNUSED(time_unit);

    instance->tim_arr = time;
}

void signal_reader_set_trigger(SignalReader* instance, SignalReaderTrigger trigger) {
    furi_check(instance);

    instance->trigger = trigger;
}

static void furi_hal_sw_digital_pin_dma_rx_isr(void* context) {
    SignalReader* instance = context;

    uint16_t* gpio_buff_start = NULL;
    uint8_t* bitstream_buff_start = NULL;

    if(LL_DMA_IsActiveFlag_HT2(SIGNAL_READER_DMA)) {
        LL_DMA_ClearFlag_HT2(SIGNAL_READER_DMA);
        instance->event.type = SignalReaderEventTypeHalfBufferFilled;
        gpio_buff_start = instance->gpio_buffer;
        bitstream_buff_start = instance->bitstream_buffer;

        if(instance->callback) {
            furi_assert(gpio_buff_start);
            furi_assert(bitstream_buff_start);

            for(size_t i = 0; i < instance->buffer_size * 4; i++) {
                if((i % 8) == 0) {
                    bitstream_buff_start[i / 8] = 0;
                }
                uint8_t bit = 0;
                if(instance->polarity == SignalReaderPolarityNormal) {
                    bit = (gpio_buff_start[i] & instance->pin->pin) == instance->pin->pin;
                } else {
                    bit = (gpio_buff_start[i] & instance->pin->pin) == 0;
                }
                bitstream_buff_start[i / 8] |= bit << (i % 8);
            }
            instance->event_data.data = bitstream_buff_start;
            instance->event_data.len = instance->buffer_size / 2;
            instance->callback(instance->event, instance->context);
        }
    }
    if(LL_DMA_IsActiveFlag_TC2(SIGNAL_READER_DMA)) {
        LL_DMA_ClearFlag_TC2(SIGNAL_READER_DMA);
        instance->event.type = SignalReaderEventTypeFullBufferFilled;
        gpio_buff_start = &instance->gpio_buffer[instance->buffer_size * 4];
        bitstream_buff_start = &instance->bitstream_buffer[instance->buffer_size / 2];

        if(instance->callback) {
            furi_assert(gpio_buff_start);
            furi_assert(bitstream_buff_start);

            for(size_t i = 0; i < instance->buffer_size * 4; i++) {
                if((i % 8) == 0) {
                    bitstream_buff_start[i / 8] = 0;
                }
                uint8_t bit = 0;
                if(instance->polarity == SignalReaderPolarityNormal) {
                    bit = (gpio_buff_start[i] & instance->pin->pin) == instance->pin->pin;
                } else {
                    bit = (gpio_buff_start[i] & instance->pin->pin) == 0;
                }
                bitstream_buff_start[i / 8] |= bit << (i % 8);
            }
            instance->event_data.data = bitstream_buff_start;
            instance->event_data.len = instance->buffer_size / 2;
            instance->callback(instance->event, instance->context);
        }
    }
}

void signal_reader_start(SignalReader* instance, SignalReaderCallback callback, void* context) {
    furi_check(instance);
    furi_check(callback);

    instance->callback = callback;
    instance->context = context;

    // EXTI delay compensation
    instance->tim_cnt_compensation = 9;
    instance->cnt_en = SIGNAL_READER_CAPTURE_TIM->CR1;
    instance->cnt_en |= TIM_CR1_CEN;

    furi_hal_bus_enable(FuriHalBusTIM16);

    // Capture timer config
    LL_TIM_SetPrescaler(SIGNAL_READER_CAPTURE_TIM, 0);
    LL_TIM_SetCounterMode(SIGNAL_READER_CAPTURE_TIM, LL_TIM_COUNTERMODE_UP);
    LL_TIM_SetAutoReload(SIGNAL_READER_CAPTURE_TIM, instance->tim_arr);
    LL_TIM_SetClockDivision(SIGNAL_READER_CAPTURE_TIM, LL_TIM_CLOCKDIVISION_DIV1);

    LL_TIM_DisableARRPreload(SIGNAL_READER_CAPTURE_TIM);
    LL_TIM_SetClockSource(SIGNAL_READER_CAPTURE_TIM, LL_TIM_CLOCKSOURCE_INTERNAL);

    // Configure TIM channel CC1
    LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {};
    TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_FROZEN;
    TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
    TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
    TIM_OC_InitStruct.CompareValue = (instance->tim_arr / 2);
    TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
    LL_TIM_OC_Init(
        SIGNAL_READER_CAPTURE_TIM, SIGNAL_READER_CAPTURE_TIM_CHANNEL, &TIM_OC_InitStruct);
    LL_TIM_OC_DisableFast(SIGNAL_READER_CAPTURE_TIM, SIGNAL_READER_CAPTURE_TIM_CHANNEL);

    LL_TIM_SetTriggerOutput(SIGNAL_READER_CAPTURE_TIM, LL_TIM_TRGO_RESET);
    LL_TIM_DisableMasterSlaveMode(SIGNAL_READER_CAPTURE_TIM);

    // Start
    LL_TIM_GenerateEvent_UPDATE(SIGNAL_READER_CAPTURE_TIM);

    /* We need the EXTI to be configured as interrupt generating line, but no ISR registered */
    furi_hal_gpio_init(
        instance->pin, GpioModeInterruptRiseFall, instance->pull, GpioSpeedVeryHigh);
    furi_hal_gpio_enable_int_callback(instance->pin);

    /* Set DMAMUX request generation signal ID on specified DMAMUX channel */
    LL_DMAMUX_SetRequestSignalID(
        DMAMUX1, LL_DMAMUX_REQ_GEN_0, GET_DMAMUX_EXTI_LINE(instance->pin->pin));
    /* Set the polarity of the signal on which the DMA request is generated */
    LL_DMAMUX_SetRequestGenPolarity(DMAMUX1, LL_DMAMUX_REQ_GEN_0, LL_DMAMUX_REQ_GEN_POL_RISING);
    /* Set the number of DMA requests that will be authorized after a generation event */
    LL_DMAMUX_SetGenRequestNb(DMAMUX1, LL_DMAMUX_REQ_GEN_0, 1);

    // Configure DMA Sync
    LL_DMA_SetMemoryAddress(
        SIGNAL_READER_DMA_CNT_SYNC_DEF, (uint32_t)&instance->tim_cnt_compensation);
    LL_DMA_SetPeriphAddress(
        SIGNAL_READER_DMA_CNT_SYNC_DEF, (uint32_t) & (SIGNAL_READER_CAPTURE_TIM->CNT));
    LL_DMA_ConfigTransfer(
        SIGNAL_READER_DMA_CNT_SYNC_DEF,
        LL_DMA_DIRECTION_MEMORY_TO_PERIPH | LL_DMA_MODE_CIRCULAR | LL_DMA_PERIPH_NOINCREMENT |
            LL_DMA_MEMORY_NOINCREMENT | LL_DMA_PDATAALIGN_HALFWORD | LL_DMA_MDATAALIGN_HALFWORD |
            LL_DMA_PRIORITY_VERYHIGH);
    LL_DMA_SetDataLength(SIGNAL_READER_DMA_CNT_SYNC_DEF, 1);
    LL_DMA_SetPeriphRequest(SIGNAL_READER_DMA_CNT_SYNC_DEF, LL_DMAMUX_REQ_GENERATOR0);

    // Configure DMA Sync
    LL_DMA_SetMemoryAddress(SIGNAL_READER_DMA_TRIGGER_DEF, (uint32_t)&instance->cnt_en);
    LL_DMA_SetPeriphAddress(
        SIGNAL_READER_DMA_TRIGGER_DEF, (uint32_t) & (SIGNAL_READER_CAPTURE_TIM->CR1));
    LL_DMA_ConfigTransfer(
        SIGNAL_READER_DMA_TRIGGER_DEF,
        LL_DMA_DIRECTION_MEMORY_TO_PERIPH | LL_DMA_PERIPH_NOINCREMENT | LL_DMA_MEMORY_NOINCREMENT |
            LL_DMA_PDATAALIGN_HALFWORD | LL_DMA_MDATAALIGN_HALFWORD | LL_DMA_PRIORITY_VERYHIGH);
    LL_DMA_SetDataLength(SIGNAL_READER_DMA_TRIGGER_DEF, 1);
    LL_DMA_SetPeriphRequest(SIGNAL_READER_DMA_TRIGGER_DEF, LL_DMAMUX_REQ_GENERATOR0);

    // Configure DMA Rx pin
    LL_DMA_SetMemoryAddress(SIGNAL_READER_DMA_GPIO_DEF, (uint32_t)instance->gpio_buffer);
    LL_DMA_SetPeriphAddress(SIGNAL_READER_DMA_GPIO_DEF, (uint32_t) & (instance->pin->port->IDR));
    LL_DMA_ConfigTransfer(
        SIGNAL_READER_DMA_GPIO_DEF,
        LL_DMA_DIRECTION_PERIPH_TO_MEMORY | LL_DMA_MODE_CIRCULAR | LL_DMA_PERIPH_NOINCREMENT |
            LL_DMA_MEMORY_INCREMENT | LL_DMA_PDATAALIGN_HALFWORD | LL_DMA_MDATAALIGN_HALFWORD |
            LL_DMA_PRIORITY_HIGH);
    LL_DMA_SetDataLength(SIGNAL_READER_DMA_GPIO_DEF, instance->buffer_size * 8);
    LL_DMA_SetPeriphRequest(SIGNAL_READER_DMA_GPIO_DEF, LL_DMAMUX_REQ_TIM16_CH1);

    // Configure DMA Channel CC1
    LL_TIM_EnableDMAReq_CC1(SIGNAL_READER_CAPTURE_TIM);
    LL_TIM_CC_EnableChannel(SIGNAL_READER_CAPTURE_TIM, SIGNAL_READER_CAPTURE_TIM_CHANNEL);

    // Start DMA irq, higher priority than normal
    furi_hal_interrupt_set_isr_ex(
        SIGNAL_READER_DMA_GPIO_IRQ,
        FuriHalInterruptPriorityHighest,
        furi_hal_sw_digital_pin_dma_rx_isr,
        instance);

    // Start DMA Sync timer
    LL_DMA_EnableChannel(SIGNAL_READER_DMA_CNT_SYNC_DEF);

    // Start DMA Rx pin
    LL_DMA_EnableChannel(SIGNAL_READER_DMA_GPIO_DEF);
    // Strat timer
    LL_TIM_SetCounter(SIGNAL_READER_CAPTURE_TIM, 0);
    if(instance->trigger == SignalReaderTriggerNone) {
        LL_TIM_EnableCounter(SIGNAL_READER_CAPTURE_TIM);
    } else {
        LL_DMA_EnableChannel(SIGNAL_READER_DMA_TRIGGER_DEF);
    }

    LL_DMAMUX_EnableRequestGen(DMAMUX1, LL_DMAMUX_REQ_GEN_0);
    // Need to clear flags before enabling DMA !!!!
    if(LL_DMA_IsActiveFlag_TC2(SIGNAL_READER_DMA)) LL_DMA_ClearFlag_TC1(SIGNAL_READER_DMA);
    if(LL_DMA_IsActiveFlag_TE2(SIGNAL_READER_DMA)) LL_DMA_ClearFlag_TE1(SIGNAL_READER_DMA);
    LL_DMA_EnableIT_TC(SIGNAL_READER_DMA_GPIO_DEF);
    LL_DMA_EnableIT_HT(SIGNAL_READER_DMA_GPIO_DEF);
}

void signal_reader_stop(SignalReader* instance) {
    furi_check(instance);

    furi_hal_interrupt_set_isr(SIGNAL_READER_DMA_GPIO_IRQ, NULL, NULL);

    furi_hal_gpio_disable_int_callback(instance->pin);

    // Deinit DMA Rx pin
    LL_DMA_DeInit(SIGNAL_READER_DMA_GPIO_DEF);
    // Deinit DMA Sync timer
    LL_DMA_DeInit(SIGNAL_READER_DMA_CNT_SYNC_DEF);
    // Deinit DMA Trigger timer
    LL_DMA_DeInit(SIGNAL_READER_DMA_TRIGGER_DEF);

    furi_hal_bus_disable(FuriHalBusTIM16);
}
