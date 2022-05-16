#include "random_name.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <furi.h>
#include <furi_hal.h>

void set_random_name(char* name, uint8_t max_name_size) {
    static bool rand_generator_inited = false;
    if(!rand_generator_inited) {
        srand(DWT->CYCCNT);
        rand_generator_inited = true;
    }

    FuriHalRtcDateTime datetime;
    furi_hal_rtc_get_datetime(&datetime);
    char strings[1][25];
    sprintf(strings[0], "%s%.4d%.2d%.2d%.2d%.2d", "scan_"
        , datetime.year, datetime.month, datetime.day
        , datetime.hour, datetime.minute
    );

    sniprintf(name, max_name_size, "%s", strings[0]);
    // Set first symbol to upper case
    name[0] = name[0] - 0x20;
}
