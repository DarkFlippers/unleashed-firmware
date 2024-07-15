#include "subghz_threshold_rssi.h"
#include "../views/subghz_read_raw.h"
#include <float_tools.h>

#define TAG "SubGhzThresholdRssi"

#define THRESHOLD_RSSI_LOW_COUNT 10

struct SubGhzThresholdRssi {
    float threshold_rssi;
    uint8_t threshold_rssi_low_count;
};

SubGhzThresholdRssi* subghz_threshold_rssi_alloc(void) {
    SubGhzThresholdRssi* instance = malloc(sizeof(SubGhzThresholdRssi));
    instance->threshold_rssi = SUBGHZ_RAW_THRESHOLD_MIN;
    instance->threshold_rssi_low_count = THRESHOLD_RSSI_LOW_COUNT;
    return instance;
}

void subghz_threshold_rssi_free(SubGhzThresholdRssi* instance) {
    furi_assert(instance);
    free(instance);
}

void subghz_threshold_rssi_set(SubGhzThresholdRssi* instance, float rssi) {
    furi_assert(instance);
    instance->threshold_rssi = rssi;
}

float subghz_threshold_rssi_get(SubGhzThresholdRssi* instance) {
    furi_assert(instance);
    return instance->threshold_rssi;
}

SubGhzThresholdRssiData subghz_threshold_get_rssi_data(SubGhzThresholdRssi* instance, float rssi) {
    furi_assert(instance);
    SubGhzThresholdRssiData ret = {.rssi = rssi, .is_above = false};

    if(float_is_equal(instance->threshold_rssi, SUBGHZ_RAW_THRESHOLD_MIN)) {
        ret.is_above = true;
    } else {
        if(rssi < instance->threshold_rssi) {
            instance->threshold_rssi_low_count++;
            if(instance->threshold_rssi_low_count > THRESHOLD_RSSI_LOW_COUNT) {
                instance->threshold_rssi_low_count = THRESHOLD_RSSI_LOW_COUNT;
            }
            ret.is_above = false;
        } else {
            instance->threshold_rssi_low_count = 0;
        }

        if(instance->threshold_rssi_low_count == THRESHOLD_RSSI_LOW_COUNT) {
            ret.is_above = false;
        } else {
            ret.is_above = true;
        }
    }
    return ret;
}
