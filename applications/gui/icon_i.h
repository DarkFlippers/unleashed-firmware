#pragma once

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

const uint8_t* icon_get_data(Icon* icon);

void icon_next_frame(Icon* icon);
