#include <furi_hal_random.h>
#include <furi_hal_bus.h>
#include <furi.h>

#include <stm32wbxx_ll_rng.h>
#include <stm32wbxx_ll_rcc.h>
#include <stm32wbxx_ll_hsem.h>

#include <hsem_map.h>

#define TAG "FuriHalRandom"

static uint32_t furi_hal_random_read_rng(void) {
    while(LL_RNG_IsActiveFlag_CECS(RNG) || LL_RNG_IsActiveFlag_SECS(RNG) ||
          !LL_RNG_IsActiveFlag_DRDY(RNG)) {
        /* Error handling as described in RM0434, pg. 582-583 */
        if(LL_RNG_IsActiveFlag_CECS(RNG)) {
            /* Clock error occurred */
            LL_RNG_ClearFlag_CEIS(RNG);
        }

        if(LL_RNG_IsActiveFlag_SECS(RNG)) {
            /* Noise source error occurred */
            LL_RNG_ClearFlag_SEIS(RNG);

            for(uint32_t i = 0; i < 12; ++i) {
                const volatile uint32_t discard = LL_RNG_ReadRandData32(RNG);
                UNUSED(discard);
            }
        }
    }

    return LL_RNG_ReadRandData32(RNG);
}

void furi_hal_random_init(void) {
    furi_hal_bus_enable(FuriHalBusRNG);
    LL_RCC_SetRNGClockSource(LL_RCC_RNG_CLKSOURCE_CLK48);
}

uint32_t furi_hal_random_get(void) {
    while(LL_HSEM_1StepLock(HSEM, CFG_HW_RNG_SEMID))
        ;
    LL_RNG_Enable(RNG);

    const uint32_t random_val = furi_hal_random_read_rng();

    LL_RNG_Disable(RNG);
    ;
    LL_HSEM_ReleaseLock(HSEM, CFG_HW_RNG_SEMID, 0);

    return random_val;
}

void furi_hal_random_fill_buf(uint8_t* buf, uint32_t len) {
    furi_check(buf);
    furi_check(len);

    while(LL_HSEM_1StepLock(HSEM, CFG_HW_RNG_SEMID))
        ;
    LL_RNG_Enable(RNG);

    for(uint32_t i = 0; i < len; i += 4) {
        const uint32_t random_val = furi_hal_random_read_rng();
        uint8_t len_cur = ((i + 4) < len) ? (4) : (len - i);
        memcpy(&buf[i], &random_val, len_cur);
    }

    LL_RNG_Disable(RNG);
    LL_HSEM_ReleaseLock(HSEM, CFG_HW_RNG_SEMID, 0);
}

void srand(unsigned seed) {
    UNUSED(seed);
}

int rand(void) {
    return furi_hal_random_get() & RAND_MAX;
}

long random(void) {
    return furi_hal_random_get() & RAND_MAX;
}
