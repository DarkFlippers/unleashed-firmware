#include "value_index.h"

uint8_t value_index_uint32(const uint32_t value, const uint32_t values[], uint8_t values_count) {
    int64_t last_value = INT64_MIN;
    uint8_t index = 0;
    for(uint8_t i = 0; i < values_count; i++) {
        if((value >= last_value) && (value <= values[i])) {
            index = i;
            break;
        }
        last_value = values[i];
    }
    return index;
}

uint8_t value_index_float(const float value, const float values[], uint8_t values_count) {
    const float epsilon = 0.01f;
    float last_value = values[0];
    uint8_t index = 0;
    for(uint8_t i = 0; i < values_count; i++) {
        if((value >= last_value - epsilon) && (value <= values[i] + epsilon)) {
            index = i;
            break;
        }
        last_value = values[i];
    }
    return index;
}

uint8_t value_index_bool(const bool value, const bool values[], uint8_t values_count) {
    uint8_t index = 0;
    for(uint8_t i = 0; i < values_count; i++) {
        if(value == values[i]) {
            index = i;
            break;
        }
    }
    return index;
}
