#include "api-hal-delay.h"

void delay_us_init_DWT(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT = 0U;
}

void delay_us(float time) {
    uint32_t start = DWT->CYCCNT;
    uint32_t time_ticks = time * (SystemCoreClock / 1000000);
    while((DWT->CYCCNT - start) < time_ticks) {
    };
}