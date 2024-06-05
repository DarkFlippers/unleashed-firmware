#include "icon.h"
#include "icon_i.h" // IWYU pragma: keep
#include <furi.h>

#include <furi.h>

uint16_t icon_get_width(const Icon* instance) {
    furi_check(instance);

    return instance->width;
}

uint16_t icon_get_height(const Icon* instance) {
    furi_check(instance);

    return instance->height;
}

const uint8_t* icon_get_data(const Icon* instance) {
    furi_check(instance);

    return icon_get_frame_data(instance, 0);
}

uint32_t icon_get_frame_count(const Icon* instance) {
    return instance->frame_count;
}

const uint8_t* icon_get_frame_data(const Icon* instance, uint32_t frame) {
    furi_check(frame < instance->frame_count);
    return instance->frames[frame];
}
