#include "dolphin_state.h"
#include <stdint.h>
#include <storage/storage.h>
#include <furi.h>
#include <furi-hal.h>
#include <math.h>
#include <toolbox/saved_struct.h>

#define TAG "DolphinState"
#define DOLPHIN_STATE_PATH "/int/dolphin.state"
#define DOLPHIN_STATE_HEADER_MAGIC 0xD0
#define DOLPHIN_STATE_HEADER_VERSION 0x01
#define DOLPHIN_LVL_THRESHOLD 20.0f
#define LEVEL2_THRESHOLD 20
#define LEVEL3_THRESHOLD 100
#define BUTTHURT_MAX 14
#define BUTTHURT_MIN 0

DolphinState* dolphin_state_alloc() {
    return furi_alloc(sizeof(DolphinState));
}

void dolphin_state_free(DolphinState* dolphin_state) {
    free(dolphin_state);
}

bool dolphin_state_save(DolphinState* dolphin_state) {
    if(!dolphin_state->dirty) {
        return true;
    }

    bool result = saved_struct_save(
        DOLPHIN_STATE_PATH,
        &dolphin_state->data,
        sizeof(DolphinStoreData),
        DOLPHIN_STATE_HEADER_MAGIC,
        DOLPHIN_STATE_HEADER_VERSION);

    if(result) {
        FURI_LOG_I(TAG, "State saved");
        dolphin_state->dirty = false;
    } else {
        FURI_LOG_E(TAG, "Failed to save state");
    }

    return result;
}

bool dolphin_state_load(DolphinState* dolphin_state) {
    bool success = saved_struct_load(
        DOLPHIN_STATE_PATH,
        &dolphin_state->data,
        sizeof(DolphinStoreData),
        DOLPHIN_STATE_HEADER_MAGIC,
        DOLPHIN_STATE_HEADER_VERSION);

    if(success) {
        if((dolphin_state->data.butthurt > BUTTHURT_MAX) ||
           (dolphin_state->data.butthurt < BUTTHURT_MIN)) {
            success = false;
        }
    }

    if(!success) {
        FURI_LOG_W(TAG, "Reset dolphin-state");
        memset(dolphin_state, 0, sizeof(*dolphin_state));
        dolphin_state->dirty = true;
    }

    return success;
}

uint64_t dolphin_state_timestamp() {
    FuriHalRtcDateTime datetime;
    struct tm current;

    furi_hal_rtc_get_datetime(&datetime);

    current.tm_year = datetime.year - 1900;
    current.tm_mday = datetime.day;
    current.tm_mon = datetime.month - 1;

    current.tm_hour = datetime.hour;
    current.tm_min = datetime.minute;
    current.tm_sec = datetime.second;

    return mktime(&current);
}

bool dolphin_state_is_levelup(uint32_t icounter) {
    return (icounter == LEVEL2_THRESHOLD) || (icounter == LEVEL3_THRESHOLD);
}

uint8_t dolphin_get_level(uint32_t icounter) {
    if(icounter <= LEVEL2_THRESHOLD) {
        return 1;
    } else if(icounter <= LEVEL3_THRESHOLD) {
        return 2;
    } else {
        return 3;
    }
}

uint32_t dolphin_state_xp_above_last_levelup(uint32_t icounter) {
    uint32_t threshold = 0;
    if(icounter <= LEVEL2_THRESHOLD) {
        threshold = 0;
    } else if(icounter <= LEVEL3_THRESHOLD) {
        threshold = LEVEL2_THRESHOLD + 1;
    } else {
        threshold = LEVEL3_THRESHOLD + 1;
    }
    return icounter - threshold;
}

uint32_t dolphin_state_xp_to_levelup(uint32_t icounter) {
    uint32_t threshold = 0;
    if(icounter <= LEVEL2_THRESHOLD) {
        threshold = LEVEL2_THRESHOLD;
    } else if(icounter <= LEVEL3_THRESHOLD) {
        threshold = LEVEL3_THRESHOLD;
    } else {
        threshold = (uint32_t)-1;
    }
    return threshold - icounter;
}

bool dolphin_state_on_deed(DolphinState* dolphin_state, DolphinDeed deed) {
    const DolphinDeedWeight* deed_weight = dolphin_deed_weight(deed);
    int32_t icounter = dolphin_state->data.icounter + deed_weight->icounter;
    bool level_up = false;
    bool mood_changed = false;

    if(icounter <= 0) {
        icounter = 0;
        if(dolphin_state->data.icounter == 0) {
            return false;
        }
    }

    uint8_t xp_to_levelup = dolphin_state_xp_to_levelup(dolphin_state->data.icounter);
    if(xp_to_levelup) {
        level_up = true;
        dolphin_state->data.icounter += MIN(xp_to_levelup, deed_weight->icounter);
    }

    uint32_t new_butthurt = CLAMP(
        ((int32_t)dolphin_state->data.butthurt) + deed_weight->butthurt,
        BUTTHURT_MAX,
        BUTTHURT_MIN);

    if(!!dolphin_state->data.butthurt != !!new_butthurt) {
        mood_changed = true;
    }
    dolphin_state->data.butthurt = new_butthurt;
    dolphin_state->data.timestamp = dolphin_state_timestamp();
    dolphin_state->dirty = true;

    return level_up || mood_changed;
}

void dolphin_state_butthurted(DolphinState* dolphin_state) {
    if(dolphin_state->data.butthurt < BUTTHURT_MAX) {
        dolphin_state->data.butthurt++;
        FURI_LOG_I("DolphinState", "Increasing butthurt");
        dolphin_state->data.timestamp = dolphin_state_timestamp();
        dolphin_state->dirty = true;
    }
}

void dolphin_state_increase_level(DolphinState* dolphin_state) {
    ++dolphin_state->data.icounter;
    dolphin_state->dirty = true;
}
