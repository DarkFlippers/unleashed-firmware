#pragma once

#include <stdint.h>

/* Countable deed that affects icounter*/
typedef enum {
    // iButton
    DolphinDeedIButtonRead,
    DolphinDeedIButtonWrite,
    DolphinDeedIButtonEmulate,
    // for debug
    DolphinDeedWrong,
    // Special value, do not use
    DolphinDeedMax
} DolphinDeed;

typedef struct {
    int32_t icounter; // how many icounter get by Deed
    uint32_t limit_value; // how many deeds in limit interval
    uint32_t limit_interval; // interval, in minutes
} DolphinDeedWeight;

const DolphinDeedWeight* dolphin_deed_weight(DolphinDeed deed);
