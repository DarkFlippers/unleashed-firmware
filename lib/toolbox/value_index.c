#include "value_index.h"
#include <math.h>

size_t value_index_int32(const int32_t value, const int32_t values[], size_t values_count) {
    size_t index = 0;

    for(size_t i = 0; i < values_count; i++) {
        if(value == values[i]) {
            index = i;
            break;
        }
    }

    return index;
}

size_t value_index_uint32(const uint32_t value, const uint32_t values[], size_t values_count) {
    size_t index = 0;

    for(size_t i = 0; i < values_count; i++) {
        if(value == values[i]) {
            index = i;
            break;
        }
    }

    return index;
}

size_t value_index_float(const float value, const float values[], size_t values_count) {
    size_t index = 0;

    for(size_t i = 0; i < values_count; i++) {
        const float epsilon = fabsf(values[i] * 0.01f);
        if(fabsf(values[i] - value) <= epsilon) {
            index = i;
            break;
        }
    }

    return index;
}

size_t value_index_bool(const bool value, const bool values[], size_t values_count) {
    size_t index = 0;

    for(size_t i = 0; i < values_count; i++) {
        if(value == values[i]) {
            index = i;
            break;
        }
    }

    return index;
}
