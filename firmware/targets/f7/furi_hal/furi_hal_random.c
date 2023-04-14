#include <furi_hal_random.h>
#include <furi.h>
#include <furi_hal.h>

#include <stm32wbxx_ll_rng.h>
#include <stm32wbxx_ll_hsem.h>

#include <hw_conf.h>

#define TAG "FuriHalRandom"

uint32_t furi_hal_random_get() {
    while(LL_HSEM_1StepLock(HSEM, CFG_HW_RNG_SEMID))
        ;
    LL_RNG_Enable(RNG);

    while(!LL_RNG_IsActiveFlag_DRDY(RNG))
        ;

    if((LL_RNG_IsActiveFlag_CECS(RNG)) || (LL_RNG_IsActiveFlag_SECS(RNG))) {
        furi_crash("TRNG error");
    }

    uint32_t random_val = LL_RNG_ReadRandData32(RNG);

    LL_RNG_Disable(RNG);
    LL_HSEM_ReleaseLock(HSEM, CFG_HW_RNG_SEMID, 0);

    return random_val;
}

void furi_hal_random_fill_buf(uint8_t* buf, uint32_t len) {
    while(LL_HSEM_1StepLock(HSEM, CFG_HW_RNG_SEMID))
        ;
    LL_RNG_Enable(RNG);

    for(uint32_t i = 0; i < len; i += 4) {
        while(!LL_RNG_IsActiveFlag_DRDY(RNG))
            ;

        if((LL_RNG_IsActiveFlag_CECS(RNG)) || (LL_RNG_IsActiveFlag_SECS(RNG))) {
            furi_crash("TRNG error");
        }

        uint32_t random_val = LL_RNG_ReadRandData32(RNG);

        uint8_t len_cur = ((i + 4) < len) ? (4) : (len - i);
        memcpy(&buf[i], &random_val, len_cur);
    }

    LL_RNG_Disable(RNG);
    LL_HSEM_ReleaseLock(HSEM, CFG_HW_RNG_SEMID, 0);
}

void srand(unsigned seed) {
    UNUSED(seed);
}

int rand() {
    return (furi_hal_random_get() & RAND_MAX);
}

long random() {
    return (furi_hal_random_get() & RAND_MAX);
}
