#include "furi_hal_delay.h"

#include <furi.h>
#include <cmsis_os2.h>

#define TAG "FuriHalDelay"
uint32_t instructions_per_us;
static volatile uint32_t tick_cnt = 0;

void furi_hal_delay_init(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT = 0U;
    instructions_per_us = SystemCoreClock / 1000000.0f;
    FURI_LOG_I(TAG, "Init OK");
}

void furi_hal_tick(void) {
    tick_cnt++;
}

uint32_t furi_hal_get_tick(void) {
    return tick_cnt;
}

void furi_hal_delay_us(float microseconds) {
    uint32_t start = DWT->CYCCNT;
    uint32_t time_ticks = microseconds * instructions_per_us;
    while((DWT->CYCCNT - start) < time_ticks) {
    };
}

// cannot be used in ISR
// TODO add delay_ISR variant
void furi_hal_delay_ms(float milliseconds) {
    uint32_t ticks = milliseconds / (1000.0f / osKernelGetTickFreq());
    osStatus_t result = osDelay(ticks);
    (void)result;
    furi_assert(result == osOK);
}
