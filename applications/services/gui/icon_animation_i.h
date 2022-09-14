/**
 * @file icon_animation_i.h
 * GUI: internal IconAnimation API
 */

#pragma once

#include "icon_animation.h"

#include <furi.h>

struct IconAnimation {
    const Icon* icon;
    uint8_t frame;
    bool animating;
    FuriTimer* timer;
    IconAnimationCallback callback;
    void* callback_context;
};

/** Get pointer to current frame data
 *
 * @param      instance  IconAnimation instance
 *
 * @return     pointer to current frame XBM bitmap data
 */
const uint8_t* icon_animation_get_data(IconAnimation* instance);

/** Advance to next frame
 *
 * @param      instance  IconAnimation instance
 */
void icon_animation_next_frame(IconAnimation* instance);

/** IconAnimation timer callback
 *
 * @param      context  pointer to IconAnimation
 */
void icon_animation_timer_callback(void* context);
