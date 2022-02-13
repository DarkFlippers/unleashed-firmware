/**
 * @file icon_i.h
 * GUI: internal Icon API
 */

#pragma once
#include "icon.h"

struct Icon {
    const uint8_t width;
    const uint8_t height;
    const uint8_t frame_count;
    const uint8_t frame_rate;
    const uint8_t* const* frames;
};
