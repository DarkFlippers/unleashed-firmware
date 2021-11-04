#include "dolphin_state.h"
#include <storage/storage.h>
#include <furi.h>
#include <math.h>
#include <toolbox/saved_struct.h>

#define DOLPHIN_STATE_TAG "DolphinState"
#define DOLPHIN_STATE_PATH "/int/dolphin.state"
#define DOLPHIN_STATE_HEADER_MAGIC 0xD0
#define DOLPHIN_STATE_HEADER_VERSION 0x01
#define DOLPHIN_LVL_THRESHOLD 20.0f

typedef struct {
    uint32_t limit_ibutton;
    uint32_t limit_nfc;
    uint32_t limit_ir;
    uint32_t limit_rfid;

    uint32_t flags;
    uint32_t icounter;
    uint32_t butthurt;
    uint64_t timestamp;
} DolphinStoreData;

struct DolphinState {
    DolphinStoreData data;
    bool dirty;
};

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
        FURI_LOG_I(DOLPHIN_STATE_TAG, "State saved");
        dolphin_state->dirty = false;
    } else {
        FURI_LOG_E(DOLPHIN_STATE_TAG, "Failed to save state");
    }

    return result;
}

bool dolphin_state_load(DolphinState* dolphin_state) {
    bool loaded = saved_struct_load(
        DOLPHIN_STATE_PATH,
        &dolphin_state->data,
        sizeof(DolphinStoreData),
        DOLPHIN_STATE_HEADER_MAGIC,
        DOLPHIN_STATE_HEADER_VERSION);

    if(!loaded) {
        FURI_LOG_W(DOLPHIN_STATE_TAG, "Reset dolphin-state");
        memset(dolphin_state, 0, sizeof(*dolphin_state));
        dolphin_state->dirty = true;
    }

    return loaded;
}

uint64_t dolphin_state_timestamp() {
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;
    struct tm current;

    HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);

    current.tm_year = date.Year + 100;
    current.tm_mday = date.Date;
    current.tm_mon = date.Month - 1;

    current.tm_hour = time.Hours;
    current.tm_min = time.Minutes;
    current.tm_sec = time.Seconds;

    return mktime(&current);
}

void dolphin_state_on_deed(DolphinState* dolphin_state, DolphinDeed deed) {
    const DolphinDeedWeight* deed_weight = dolphin_deed_weight(deed);
    int32_t icounter = dolphin_state->data.icounter + deed_weight->icounter;
    int32_t butthurt = dolphin_state->data.butthurt;

    if(icounter >= 0) {
        dolphin_state->data.icounter = icounter;
        dolphin_state->data.butthurt = MAX(butthurt - deed_weight->icounter, 0);
        dolphin_state->data.timestamp = dolphin_state_timestamp();
    }

    dolphin_state->dirty = true;
}

void dolphin_state_butthurted(DolphinState* dolphin_state) {
    dolphin_state->data.butthurt++;
    dolphin_state->data.timestamp = dolphin_state_timestamp();
    dolphin_state->dirty = true;
}

uint32_t dolphin_state_get_icounter(DolphinState* dolphin_state) {
    return dolphin_state->data.icounter;
}

uint32_t dolphin_state_get_butthurt(DolphinState* dolphin_state) {
    return dolphin_state->data.butthurt;
}

uint64_t dolphin_state_get_timestamp(DolphinState* dolphin_state) {
    return dolphin_state->data.timestamp;
}

uint32_t dolphin_state_get_level(uint32_t icounter) {
    return 0.5f + sqrtf(1.0f + 8.0f * ((float)icounter / DOLPHIN_LVL_THRESHOLD)) / 2.0f;
}

uint32_t dolphin_state_xp_to_levelup(uint32_t icounter, uint32_t level, bool remaining) {
    return (DOLPHIN_LVL_THRESHOLD * level * (level + 1) / 2) - (remaining ? icounter : 0);
}
