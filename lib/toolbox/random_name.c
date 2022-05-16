#include "random_name.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <furi.h>
#include <furi_hal.h>

#define FILE_PRE  "scan_"
#define FILE_DATE_FORMAT "%s%.4d%.2d%.2d%.2d%.2d"
typedef struct {
    FuriHalRtcDateTime datetime;
} ClockState;
static void clock_state_init(ClockState* const state) {
    furi_hal_rtc_get_datetime(&state->datetime);
}

void set_random_name(char* name, uint8_t max_name_size) {
    static bool rand_generator_inited = false;

    if(!rand_generator_inited) {
        srand(DWT->CYCCNT);
        rand_generator_inited = true;
    }
    
    ClockState* plugin_state = malloc(sizeof(ClockState));
            clock_state_init(plugin_state);
            char strings[1][65];
            sprintf(strings[0], FILE_DATE_FORMAT, FILE_PRE, plugin_state->datetime.year, plugin_state->datetime.month, plugin_state->datetime.day
                , plugin_state->datetime.hour, plugin_state->datetime.minute
            );

    sniprintf(name, max_name_size, "%s", strings[0]);
    // Set first symbol to upper case
    name[0] = name[0] - 0x20;
}
