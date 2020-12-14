#pragma once

#include "icon.h"

#include <stdint.h>

struct IconData {
    const uint8_t width;
    const uint8_t height;
    const uint8_t frame_count;
    const uint8_t frame_rate;
    const uint8_t** frames;
};

struct Icon {
    const IconData* data;
    uint8_t frame;
    uint32_t tick;
};

/*
 * Get pointer to current frame data
 */
const uint8_t* icon_get_data(Icon* icon);

/*
 * Advance to next frame
 */
void icon_next_frame(Icon* icon);
