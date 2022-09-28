#include "furi_hal_cortex.h"

#include <stm32wbxx.h>

void furi_hal_cortex_init_early() {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT = 0U;

    /* Enable instruction prefetch */
    SET_BIT(FLASH->ACR, FLASH_ACR_PRFTEN);
}

void furi_hal_cortex_delay_us(uint32_t microseconds) {
    uint32_t start = DWT->CYCCNT;
    uint32_t time_ticks = SystemCoreClock / 1000000 * microseconds;
    while((DWT->CYCCNT - start) < time_ticks) {
    };
}

uint32_t furi_hal_cortex_instructions_per_microsecond() {
    return SystemCoreClock / 1000000;
}
