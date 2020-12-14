#include "icon_i.h"

#include <cmsis_os2.h>
#include <flipper.h>
#include <flipper_v2.h>

Icon* icon_alloc(const IconData* data) {
    Icon* icon = furi_alloc(sizeof(Icon));
    icon->data = data;
    return icon;
}

void icon_free(Icon* icon) {
    furi_assert(icon);
    free(icon);
}

const uint8_t* icon_get_data(Icon* icon) {
    furi_assert(icon);
    if(icon->tick) {
        uint32_t now = osKernelGetTickCount();
        if(now < icon->tick) {
            icon->tick = now;
            icon_next_frame(icon);
        } else if(now - icon->tick > osKernelGetTickFreq() / icon->data->frame_rate) {
            icon->tick = now;
            icon_next_frame(icon);
        }
    }
    return icon->data->frames[icon->frame];
}

void icon_next_frame(Icon* icon) {
    furi_assert(icon);
    icon->frame = (icon->frame + 1) % icon->data->frame_count;
}

uint8_t icon_get_width(Icon* icon) {
    furi_assert(icon);
    return icon->data->width;
}

uint8_t icon_get_height(Icon* icon) {
    furi_assert(icon);
    return icon->data->height;
}

bool icon_is_animated(Icon* icon) {
    furi_assert(icon);
    return icon->data->frame_count > 1;
}

void icon_start_animation(Icon* icon) {
    furi_assert(icon);
    icon->tick = osKernelGetTickCount();
}

void icon_stop_animation(Icon* icon) {
    furi_assert(icon);
    icon->tick = 0;
    icon->frame = 0;
}
