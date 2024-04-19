#include "name_generator.h"

#include <stdio.h>
#include <stdint.h>
#include <furi_hal_rtc.h>
#include <stdlib.h>
#include <stdbool.h>
#include <furi.h>

const char* const name_generator_left[] = {
    "super",  "big",   "little", "liquid", "unknown", "cheeky",  "tricky",
    "sneaky", "silly", "oh_my",  "quick",  "oh_no",   "quantum", "kurwa",
    "great",  "smart", "mini",   "ultra",  "small",   "random",  "strange",
};

const char* const name_generator_right[] = {
    "maslina",  "sus",  "anomalija", "artefact", "bobr",   "chomik",  "sidorovich",
    "stalker",  "kit",  "habar",     "jezyk",    "borov",  "juzyk",   "konserva",
    "aptechka", "door", "zalaz",     "breeky",   "bunker", "pingwin", "kot",
};

void name_generator_make_auto_datetime(
    char* name,
    size_t max_name_size,
    const char* prefix,
    DateTime* custom_time) {
    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDetailedFilename)) {
        name_generator_make_detailed_datetime(name, max_name_size, prefix, custom_time);
    } else {
        name_generator_make_random_prefixed(name, max_name_size, prefix);
    }
}

void name_generator_make_auto(char* name, size_t max_name_size, const char* prefix) {
    name_generator_make_auto_datetime(name, max_name_size, prefix, NULL);
}

void name_generator_make_auto_basic(char* name, size_t max_name_size, const char* prefix) {
    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDetailedFilename)) {
        name_generator_make_detailed_datetime(name, max_name_size, prefix, NULL);
    } else {
        name_generator_make_random(name, max_name_size);
    }
}

void name_generator_make_random_prefixed(char* name, size_t max_name_size, const char* prefix) {
    furi_check(name);
    furi_check(max_name_size);

    uint8_t name_generator_left_i = rand() % COUNT_OF(name_generator_left);
    uint8_t name_generator_right_i = rand() % COUNT_OF(name_generator_right);

    snprintf(
        name,
        max_name_size,
        "%s%s%s_%s",
        prefix ? prefix : "",
        prefix ? "_" : "",
        name_generator_left[name_generator_left_i],
        name_generator_right[name_generator_right_i]);

    // Set first symbol to upper case
    if(islower((int)name[0])) name[0] = name[0] - 0x20;
}

void name_generator_make_random(char* name, size_t max_name_size) {
    name_generator_make_random_prefixed(name, max_name_size, NULL);
}

void name_generator_make_detailed_datetime(
    char* name,
    size_t max_name_size,
    const char* prefix,
    DateTime* custom_time) {
    furi_check(name);
    furi_check(max_name_size);

    DateTime dateTime;
    if(custom_time) {
        dateTime = *custom_time;
    } else {
        furi_hal_rtc_get_datetime(&dateTime);
    }

    snprintf(
        name,
        max_name_size,
        "%s-%.4d_%.2d_%.2d-%.2d_%.2d_%.2d",
        prefix ? prefix : "S",
        dateTime.year,
        dateTime.month,
        dateTime.day,
        dateTime.hour,
        dateTime.minute,
        dateTime.second);

    // Set first symbol to upper case
    if(islower((int)name[0])) name[0] = name[0] - 0x20;
}

void name_generator_make_detailed(char* name, size_t max_name_size, const char* prefix) {
    name_generator_make_detailed_datetime(name, max_name_size, prefix, NULL);
}
