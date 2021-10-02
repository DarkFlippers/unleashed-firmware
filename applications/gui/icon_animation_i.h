#pragma once

#include "icon_animation.h"

#include <furi.h>

struct IconAnimation {
    const Icon* icon;
    uint8_t frame;
    bool animating;
    osTimerId_t timer;
    IconAnimationCallback callback;
    void* callback_context;
};

/** Get pointer to current frame data */
const uint8_t* icon_animation_get_data(IconAnimation* instance);

/** Advance to next frame */
void icon_animation_next_frame(IconAnimation* instance);

/** IconAnimation timer callback */
void icon_animation_timer_callback(void* context);
