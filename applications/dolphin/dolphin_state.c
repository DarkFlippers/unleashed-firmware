#include "dolphin_state.h"
#include <flipper_v2.h>

typedef struct {
    uint32_t ibutton;
    uint32_t nfc;
    uint32_t ir;
    uint32_t rfid;
} DolphinLimit;

struct DolphinState {
    uint32_t icounter;
    uint32_t butthurt;

    DolphinLimit limit;
};

DolphinState* dolphin_state_alloc() {
    DolphinState* dolphin_state = furi_alloc(sizeof(DolphinState));
    return dolphin_state;
}

void dolphin_state_release(DolphinState* dolphin_state) {
    free(dolphin_state);
}

void dolphin_state_save(DolphinState* dolphin_state) {
}

void dolphin_state_load(DolphinState* dolphin_state) {
}

void dolphin_state_clear(DolphinState* dolphin_state) {
    memset(dolphin_state, 0, sizeof(DolphinState));
}

void dolphin_state_on_deed(DolphinState* dolphin_state, DolphinDeed deed) {
    const DolphinDeedWeight* deed_weight = dolphin_deed_weight(deed);
    int32_t icounter = dolphin_state->icounter + deed_weight->icounter;

    if(icounter >= 0) {
        dolphin_state->icounter = icounter;
    }
}

uint32_t dolphin_state_get_icounter(DolphinState* dolphin_state) {
    return dolphin_state->icounter;
}

uint32_t dolphin_state_get_butthurt(DolphinState* dolphin_state) {
    return dolphin_state->butthurt;
}
