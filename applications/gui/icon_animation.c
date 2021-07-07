#include "icon_animation_i.h"
#include "icon_i.h"

#include <furi.h>

IconAnimation* icon_animation_alloc(const Icon* icon) {
    furi_assert(icon);
    IconAnimation* instance = furi_alloc(sizeof(IconAnimation));
    instance->icon = icon;
    return instance;
}

void icon_animation_free(IconAnimation* instance) {
    furi_assert(instance);
    free(instance);
}

const uint8_t* icon_animation_get_data(IconAnimation* instance) {
    furi_assert(instance);
    if(instance->tick) {
        uint32_t now = osKernelGetTickCount();
        if(now < instance->tick) {
            instance->tick = now;
            icon_animation_next_frame(instance);
        } else if(now - instance->tick > osKernelGetTickFreq() / instance->icon->frame_rate) {
            instance->tick = now;
            icon_animation_next_frame(instance);
        }
    }
    return instance->icon->frames[instance->frame];
}

void icon_animation_next_frame(IconAnimation* instance) {
    furi_assert(instance);
    instance->frame = (instance->frame + 1) % instance->icon->frame_count;
}

uint8_t icon_animation_get_width(IconAnimation* instance) {
    furi_assert(instance);
    return instance->icon->width;
}

uint8_t icon_animation_get_height(IconAnimation* instance) {
    furi_assert(instance);
    return instance->icon->height;
}

bool icon_animation_is_animated(IconAnimation* instance) {
    furi_assert(instance);
    return instance->icon->frame_count > 1;
}

bool icon_animation_is_animating(IconAnimation* instance) {
    furi_assert(instance);
    return instance->tick > 0;
}

void icon_animation_start(IconAnimation* instance) {
    furi_assert(instance);
    instance->tick = osKernelGetTickCount();
}

void icon_animation_stop(IconAnimation* instance) {
    furi_assert(instance);
    instance->tick = 0;
    instance->frame = 0;
}

uint8_t icon_animation_get_current_frame(IconAnimation* instance) {
    furi_assert(instance);
    return instance->frame;
}

bool icon_animation_is_last_frame(IconAnimation* instance) {
    furi_assert(instance);
    return instance->icon->frame_count - instance->frame <= 1;
}