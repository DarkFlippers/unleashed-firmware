#include "extra_beacon.h"
#include "gap.h"

#include <ble/ble.h>
#include <furi.h>

#define TAG "BleExtraBeacon"

#define GAP_MS_TO_SCAN_INTERVAL(x) ((uint16_t)((x) / 0.625))

// AN5289: 4.7, in order to use flash controller interval must be at least 25ms + advertisement, which is 30 ms
// Since we don't use flash controller anymore interval can be lowered to 20ms
#define GAP_MIN_ADV_INTERVAL_MS (20U)

typedef struct {
    GapExtraBeaconConfig last_config;
    GapExtraBeaconState extra_beacon_state;
    uint8_t extra_beacon_data[EXTRA_BEACON_MAX_DATA_SIZE];
    uint8_t extra_beacon_data_len;
    FuriMutex* state_mutex;
} ExtraBeacon;

static ExtraBeacon extra_beacon = {0};

void gap_extra_beacon_init(void) {
    if(extra_beacon.state_mutex) {
        // Already initialized - restore state if needed
        FURI_LOG_I(TAG, "Restoring state");
        gap_extra_beacon_set_data(
            extra_beacon.extra_beacon_data, extra_beacon.extra_beacon_data_len);
        if(extra_beacon.extra_beacon_state == GapExtraBeaconStateStarted) {
            extra_beacon.extra_beacon_state = GapExtraBeaconStateStopped;
            gap_extra_beacon_set_config(&extra_beacon.last_config);
        }

    } else {
        // First time init
        FURI_LOG_I(TAG, "Init");
        extra_beacon.extra_beacon_state = GapExtraBeaconStateStopped;
        extra_beacon.extra_beacon_data_len = 0;
        memset(extra_beacon.extra_beacon_data, 0, EXTRA_BEACON_MAX_DATA_SIZE);
        extra_beacon.state_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    }
}

bool gap_extra_beacon_set_config(const GapExtraBeaconConfig* config) {
    furi_check(extra_beacon.state_mutex);
    furi_check(config);

    furi_check(config->min_adv_interval_ms <= config->max_adv_interval_ms);
    furi_check(config->min_adv_interval_ms >= GAP_MIN_ADV_INTERVAL_MS);

    if(extra_beacon.extra_beacon_state != GapExtraBeaconStateStopped) {
        return false;
    }

    furi_mutex_acquire(extra_beacon.state_mutex, FuriWaitForever);
    if(config != &extra_beacon.last_config) {
        memcpy(&extra_beacon.last_config, config, sizeof(GapExtraBeaconConfig));
    }
    furi_mutex_release(extra_beacon.state_mutex);

    return true;
}

bool gap_extra_beacon_start(void) {
    furi_check(extra_beacon.state_mutex);
    furi_check(extra_beacon.last_config.min_adv_interval_ms >= GAP_MIN_ADV_INTERVAL_MS);

    if(extra_beacon.extra_beacon_state != GapExtraBeaconStateStopped) {
        return false;
    }

    FURI_LOG_I(TAG, "Starting");
    furi_mutex_acquire(extra_beacon.state_mutex, FuriWaitForever);
    const GapExtraBeaconConfig* config = &extra_beacon.last_config;
    tBleStatus status = aci_gap_additional_beacon_start(
        GAP_MS_TO_SCAN_INTERVAL(config->min_adv_interval_ms),
        GAP_MS_TO_SCAN_INTERVAL(config->max_adv_interval_ms),
        (uint8_t)config->adv_channel_map,
        config->address_type,
        config->address,
        (uint8_t)config->adv_power_level);
    if(status) {
        FURI_LOG_E(TAG, "Failed to start: 0x%x", status);
        return false;
    }
    extra_beacon.extra_beacon_state = GapExtraBeaconStateStarted;
    gap_emit_ble_beacon_status_event(true);
    furi_mutex_release(extra_beacon.state_mutex);

    return true;
}

bool gap_extra_beacon_stop(void) {
    furi_check(extra_beacon.state_mutex);

    if(extra_beacon.extra_beacon_state != GapExtraBeaconStateStarted) {
        return false;
    }

    FURI_LOG_I(TAG, "Stopping");
    furi_mutex_acquire(extra_beacon.state_mutex, FuriWaitForever);
    tBleStatus status = aci_gap_additional_beacon_stop();
    if(status) {
        FURI_LOG_E(TAG, "Failed to stop: 0x%x", status);
        return false;
    }
    extra_beacon.extra_beacon_state = GapExtraBeaconStateStopped;
    gap_emit_ble_beacon_status_event(false);
    furi_mutex_release(extra_beacon.state_mutex);

    return true;
}

bool gap_extra_beacon_set_data(const uint8_t* data, uint8_t length) {
    furi_check(extra_beacon.state_mutex);
    furi_check(data);
    furi_check(length <= EXTRA_BEACON_MAX_DATA_SIZE);

    furi_mutex_acquire(extra_beacon.state_mutex, FuriWaitForever);
    if(data != extra_beacon.extra_beacon_data) {
        memcpy(extra_beacon.extra_beacon_data, data, length);
    }
    extra_beacon.extra_beacon_data_len = length;

    tBleStatus status = aci_gap_additional_beacon_set_data(length, data);
    if(status) {
        FURI_LOG_E(TAG, "Failed updating adv data: %d", status);
        return false;
    }
    furi_mutex_release(extra_beacon.state_mutex);

    return true;
}

uint8_t gap_extra_beacon_get_data(uint8_t* data) {
    furi_check(extra_beacon.state_mutex);
    furi_check(data);

    furi_mutex_acquire(extra_beacon.state_mutex, FuriWaitForever);
    memcpy(data, extra_beacon.extra_beacon_data, extra_beacon.extra_beacon_data_len);
    furi_mutex_release(extra_beacon.state_mutex);

    return extra_beacon.extra_beacon_data_len;
}

GapExtraBeaconState gap_extra_beacon_get_state(void) {
    furi_check(extra_beacon.state_mutex);

    return extra_beacon.extra_beacon_state;
}

const GapExtraBeaconConfig* gap_extra_beacon_get_config(void) {
    furi_check(extra_beacon.state_mutex);

    if(extra_beacon.last_config.min_adv_interval_ms < GAP_MIN_ADV_INTERVAL_MS) {
        return NULL;
    }

    return &extra_beacon.last_config;
}
