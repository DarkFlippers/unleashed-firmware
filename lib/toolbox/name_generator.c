#include "name_generator.h"

#include <stdio.h>
#include <stdint.h>
#include <furi_hal_rtc.h>
#include <stdlib.h>
#include <stdbool.h>
#include <furi.h>

const char* const name_generator_left[] = {
    "ancient",  "hollow", "strange",   "disappeared", "unknown",    "unthinkable", "unnameable",
    "nameless", "my",     "concealed", "forgotten",   "hidden",     "mysterious",  "obscure",
    "random",   "remote", "uncharted", "undefined",   "untraveled", "untold",
};

const char* const name_generator_right[] = {
    "door",
    "entrance",
    "doorway",
    "entry",
    "portal",
    "entree",
    "opening",
    "crack",
    "access",
    "corridor",
    "passage",
    "port",
};

void name_generator_make_auto(char* name, size_t max_name_size, const char* prefix) {
    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDetailedFilename)) {
        name_generator_make_detailed(name, max_name_size, prefix);
    } else {
        name_generator_make_random(name, max_name_size);
    }
}

void name_generator_make_random(char* name, size_t max_name_size) {
    furi_check(name);
    furi_check(max_name_size);

    uint8_t name_generator_left_i = rand() % COUNT_OF(name_generator_left);
    uint8_t name_generator_right_i = rand() % COUNT_OF(name_generator_right);

    snprintf(
        name,
        max_name_size,
        "%s_%s",
        name_generator_left[name_generator_left_i],
        name_generator_right[name_generator_right_i]);

    // Set first symbol to upper case
    name[0] = name[0] - 0x20;
}

void name_generator_make_detailed(char* name, size_t max_name_size, const char* prefix) {
    furi_check(name);
    furi_check(max_name_size);
    furi_check(prefix);

    DateTime dateTime;
    furi_hal_rtc_get_datetime(&dateTime);

    snprintf(
        name,
        max_name_size,
        "%s-%.4d_%.2d_%.2d-%.2d_%.2d",
        prefix,
        dateTime.year,
        dateTime.month,
        dateTime.day,
        dateTime.hour,
        dateTime.minute);
}
