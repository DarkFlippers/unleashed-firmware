#include "timezone_utils.h"

int32_t timezone_offset_from_hours(float hours) {
    return hours * 3600.0f;
}

uint64_t timezone_offset_apply(uint64_t time, int32_t offset) {
    uint64_t for_time_adjusted;
    if(offset > 0) {
        for_time_adjusted = time - offset;
    } else {
        for_time_adjusted = time + (-offset);
    }

    return for_time_adjusted;
}
