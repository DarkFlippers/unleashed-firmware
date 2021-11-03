#include "furi-hal-lock.h"
#include <stm32wbxx_ll_rtc.h>

#define FLIPPER_LOCKED_VALUE 0x5432FAFA

bool furi_hal_lock_get() {
    return FLIPPER_LOCKED_VALUE == LL_RTC_BAK_GetRegister(RTC, LL_RTC_BKP_DR3);
}

void furi_hal_lock_set(bool locked) {
    if (locked) {
        LL_RTC_BAK_SetRegister(RTC, LL_RTC_BKP_DR3, FLIPPER_LOCKED_VALUE);
    } else {
        LL_RTC_BAK_SetRegister(RTC, LL_RTC_BKP_DR3, 0);
    }
}

