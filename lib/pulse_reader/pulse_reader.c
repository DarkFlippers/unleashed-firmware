#include "pulse_reader.h"

#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_gpio.h>

#include <stm32wbxx_ll_dma.h>
#include <stm32wbxx_ll_dmamux.h>
#include <stm32wbxx_ll_tim.h>
#include <stm32wbxx_ll_exti.h>

struct PulseReader {
    uint32_t* timer_buffer;
    uint32_t* gpio_buffer;
    uint32_t size;
    uint32_t pos;
    uint32_t timer_value;
    uint32_t gpio_value;
    uint32_t gpio_mask;
    uint32_t unit_multiplier;
    uint32_t unit_divider;
    uint32_t bit_time;
    uint32_t dma_channel;
    const GpioPin* gpio;
    GpioPull pull;
    LL_DMA_InitTypeDef dma_config_timer;
    LL_DMA_InitTypeDef dma_config_gpio;
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

PulseReader* pulse_reader_alloc(const GpioPin* gpio, uint32_t size) {
    PulseReader* signal = malloc(sizeof(PulseReader));
    signal->timer_buffer = malloc(size * sizeof(uint32_t));
    signal->gpio_buffer = malloc(size * sizeof(uint32_t));
    signal->dma_channel = LL_DMA_CHANNEL_4;
    signal->gpio = gpio;
    signal->pull = GpioPullNo;
    signal->size = size;
    signal->timer_value = 0;
    signal->pos = 0;

    pulse_reader_set_timebase(signal, PulseReaderUnit64MHz);
    pulse_reader_set_bittime(signal, 1);

    signal->dma_config_timer.Direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
    signal->dma_config_timer.PeriphOrM2MSrcAddress = (uint32_t) & (TIM2->CNT);
    signal->dma_config_timer.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
    signal->dma_config_timer.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_WORD;
    signal->dma_config_timer.MemoryOrM2MDstAddress = (uint32_t)signal->timer_buffer;
    signal->dma_config_timer.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
    signal->dma_config_timer.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_WORD;
    signal->dma_config_timer.Mode = LL_DMA_MODE_CIRCULAR;
    signal->dma_config_timer.PeriphRequest =
        LL_DMAMUX_REQ_GENERATOR0; /* executes LL_DMA_SetPeriphRequest */
    signal->dma_config_timer.Priority = LL_DMA_PRIORITY_VERYHIGH;

    signal->dma_config_gpio.Direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
    signal->dma_config_gpio.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
    signal->dma_config_gpio.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_WORD;
    signal->dma_config_gpio.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
    signal->dma_config_gpio.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_WORD;
    signal->dma_config_gpio.Mode = LL_DMA_MODE_CIRCULAR;
    signal->dma_config_gpio.PeriphRequest =
        LL_DMAMUX_REQ_GENERATOR0; /* executes LL_DMA_SetPeriphRequest */
    signal->dma_config_gpio.Priority = LL_DMA_PRIORITY_VERYHIGH;

    return signal;
}

void pulse_reader_set_timebase(PulseReader* signal, PulseReaderUnit unit) {
    switch(unit) {
    case PulseReaderUnit64MHz:
        signal->unit_multiplier = 1;
        signal->unit_divider = 1;
        break;
    case PulseReaderUnitPicosecond:
        signal->unit_multiplier = 15625;
        signal->unit_divider = 1;
        break;
    case PulseReaderUnitNanosecond:
        signal->unit_multiplier = 15625;
        signal->unit_divider = 1000;
        break;
    case PulseReaderUnitMicrosecond:
        signal->unit_multiplier = 15625;
        signal->unit_divider = 1000000;
        break;
    }
}

void pulse_reader_set_bittime(PulseReader* signal, uint32_t bit_time) {
    signal->bit_time = bit_time;
}

void pulse_reader_set_pull(PulseReader* signal, GpioPull pull) {
    signal->pull = pull;
}

void pulse_reader_free(PulseReader* signal) {
    furi_assert(signal);

    free(signal->timer_buffer);
    free(signal->gpio_buffer);
    free(signal);
}

uint32_t pulse_reader_samples(PulseReader* signal) {
    uint32_t dma_pos = signal->size - (uint32_t)LL_DMA_GetDataLength(DMA1, signal->dma_channel);

    return ((signal->pos + signal->size) - dma_pos) % signal->size;
}

void pulse_reader_stop(PulseReader* signal) {
    LL_DMA_DisableChannel(DMA1, signal->dma_channel);
    LL_DMA_DisableChannel(DMA1, signal->dma_channel + 1);
    LL_DMAMUX_DisableRequestGen(NULL, LL_DMAMUX_REQ_GEN_0);
    LL_TIM_DisableCounter(TIM2);
    furi_hal_bus_disable(FuriHalBusTIM2);
    furi_hal_gpio_init_simple(signal->gpio, GpioModeAnalog);
}

void pulse_reader_start(PulseReader* signal) {
    /* configure DMA to read from a timer peripheral */
    signal->dma_config_timer.NbData = signal->size;

    signal->dma_config_gpio.PeriphOrM2MSrcAddress = (uint32_t) & (signal->gpio->port->IDR);
    signal->dma_config_gpio.MemoryOrM2MDstAddress = (uint32_t)signal->gpio_buffer;
    signal->dma_config_gpio.NbData = signal->size;

    furi_hal_bus_enable(FuriHalBusTIM2);

    /* start counter */
    LL_TIM_SetCounterMode(TIM2, LL_TIM_COUNTERMODE_UP);
    LL_TIM_SetClockDivision(TIM2, LL_TIM_CLOCKDIVISION_DIV1);
    LL_TIM_SetPrescaler(TIM2, 0);
    LL_TIM_SetAutoReload(TIM2, 0xFFFFFFFF);
    LL_TIM_SetCounter(TIM2, 0);
    LL_TIM_EnableCounter(TIM2);

    /* generator 0 gets fed by EXTI_LINEn */
    LL_DMAMUX_SetRequestSignalID(
        NULL, LL_DMAMUX_REQ_GEN_0, GET_DMAMUX_EXTI_LINE(signal->gpio->pin));
    /* trigger on rising edge of the interrupt */
    LL_DMAMUX_SetRequestGenPolarity(NULL, LL_DMAMUX_REQ_GEN_0, LL_DMAMUX_REQ_GEN_POL_RISING);
    /* now enable request generation again */
    LL_DMAMUX_EnableRequestGen(NULL, LL_DMAMUX_REQ_GEN_0);

    /* we need the EXTI to be configured as interrupt generating line, but no ISR registered */
    furi_hal_gpio_init_ex(
        signal->gpio, GpioModeInterruptRiseFall, signal->pull, GpioSpeedVeryHigh, GpioAltFnUnused);

    /* capture current timer */
    signal->pos = 0;
    signal->timer_value = TIM2->CNT;
    signal->gpio_mask = signal->gpio->pin;
    signal->gpio_value = signal->gpio->port->IDR & signal->gpio_mask;

    /* now set up DMA with these settings */
    LL_DMA_Init(DMA1, signal->dma_channel, &signal->dma_config_timer);
    LL_DMA_Init(DMA1, signal->dma_channel + 1, &signal->dma_config_gpio);
    LL_DMA_EnableChannel(DMA1, signal->dma_channel);
    LL_DMA_EnableChannel(DMA1, signal->dma_channel + 1);
}

uint32_t pulse_reader_receive(PulseReader* signal, int timeout_us) {
    uint32_t start_time = DWT->CYCCNT;
    uint32_t timeout_ticks = timeout_us * (F_TIM2 / 1000000);

    do {
        /* get the DMA's next write position by reading "remaining length" register */
        uint32_t dma_pos =
            signal->size - (uint32_t)LL_DMA_GetDataLength(DMA1, signal->dma_channel);

        /* the DMA has advanced in the ringbuffer */
        if(dma_pos != signal->pos) {
            uint32_t delta = signal->timer_buffer[signal->pos] - signal->timer_value;
            uint32_t last_gpio_value = signal->gpio_value;

            signal->gpio_value = signal->gpio_buffer[signal->pos];

            /* check if the GPIO really toggled. if not, we lost an edge :( */
            if(((last_gpio_value ^ signal->gpio_value) & signal->gpio_mask) != signal->gpio_mask) {
                signal->gpio_value ^= signal->gpio_mask;
                return PULSE_READER_LOST_EDGE;
            }
            signal->timer_value = signal->timer_buffer[signal->pos];

            signal->pos++;
            signal->pos %= signal->size;

            uint32_t delta_unit = 0;

            /* probably larger values, so choose a wider data type */
            if(signal->unit_divider > 1) {
                delta_unit = (uint32_t)((uint64_t)delta * (uint64_t)signal->unit_multiplier /
                                        signal->unit_divider);
            } else {
                delta_unit = delta * signal->unit_multiplier;
            }

            /* if to be scaled to bit times, save a few instructions. should be faster */
            if(signal->bit_time > 1) {
                return (delta_unit + signal->bit_time / 2) / signal->bit_time;
            }

            return delta_unit;
        }

        /* check for timeout */
        uint32_t elapsed = DWT->CYCCNT - start_time;

        if(elapsed > timeout_ticks) {
            return PULSE_READER_NO_EDGE;
        }
    } while(true);
}
