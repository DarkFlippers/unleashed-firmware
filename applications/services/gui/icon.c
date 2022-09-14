#include "icon_i.h"

uint8_t icon_get_width(const Icon* instance) {
    return instance->width;
}

uint8_t icon_get_height(const Icon* instance) {
    return instance->height;
}

const uint8_t* icon_get_data(const Icon* instance) {
    return instance->frames[0];
}
