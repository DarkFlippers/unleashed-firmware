#pragma once

#include "icon_animation.h"

#include <stdint.h>

struct IconAnimation {
    const Icon* icon;
    uint8_t frame;
    uint32_t tick;
};

/*
 * Get pointer to current frame data
 */
const uint8_t* icon_animation_get_data(IconAnimation* instance);

/*
 * Advance to next frame
 */
void icon_animation_next_frame(IconAnimation* instance);
