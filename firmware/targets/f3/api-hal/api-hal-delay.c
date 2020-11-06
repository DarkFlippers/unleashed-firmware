#include "api-hal-delay.h"
#include "assert.h"
#include "cmsis_os2.h"

void delay_us_init_DWT(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT = 0U;
}

void delay_us(float microseconds) {
    uint32_t start = DWT->CYCCNT;
    uint32_t time_ticks = microseconds * (SystemCoreClock / 1000000.0f);
    while((DWT->CYCCNT - start) < time_ticks) {
    };
}

// cannot be used in ISR
// TODO add delay_ISR variant
void delay(float milliseconds) {
    uint32_t ticks = milliseconds / (1000.0f / osKernelGetTickFreq());
    osStatus_t result = osDelay(ticks);
    assert(result == osOK);
}