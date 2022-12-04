#include "random_name.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <furi.h>
#include <stm32wbxx_ll_rtc.h>

void set_random_name(char* name, uint8_t max_name_size) {
    uint32_t time = LL_RTC_TIME_Get(RTC); // 0x00HHMMSS
    uint32_t date = LL_RTC_DATE_Get(RTC); // 0xWWDDMMYY
    char strings[1][25];
    snprintf(
        strings[0],
        18,
        "%s%.4d%.2d%.2d%.2d%.2d",
        "s",
        __LL_RTC_CONVERT_BCD2BIN((date >> 0) & 0xFF) + 2000 // YEAR
        ,
        __LL_RTC_CONVERT_BCD2BIN((date >> 8) & 0xFF) // MONTH
        ,
        __LL_RTC_CONVERT_BCD2BIN((date >> 16) & 0xFF) // DAY
        ,
        __LL_RTC_CONVERT_BCD2BIN((time >> 16) & 0xFF) // HOUR
        ,
        __LL_RTC_CONVERT_BCD2BIN((time >> 8) & 0xFF) // MIN
    );
    snprintf(name, max_name_size, "%s", strings[0]);
    // Set first symbol to upper case
    name[0] = name[0] - 0x20;
}
