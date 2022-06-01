#include "random_name.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <furi.h>
#include <furi_hal.h>

void set_random_name(char* name, uint8_t max_name_size) {
    FuriHalRtcDateTime datetime;
    furi_hal_rtc_get_datetime(&datetime);
    char strings[1][25];
    sprintf(strings[0], "%s%.4d%.2d%.2d%.2d%.2d", "s"
        , datetime.year, datetime.month, datetime.day
        , datetime.hour, datetime.minute
    );
    sniprintf(name, max_name_size, "%s", strings[0]);
}
