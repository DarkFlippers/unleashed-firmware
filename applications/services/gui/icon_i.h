/**
 * @file icon_i.h
 * GUI: internal Icon API
 */

#pragma once
#include <stdint.h>

struct Icon {
    const uint16_t width;
    const uint16_t height;
    const uint8_t frame_count;
    const uint8_t frame_rate;
    const uint8_t* const* frames;
};
