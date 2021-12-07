#pragma once

#include "dolphin_deed.h"
#include <stdbool.h>
#include <stdint.h>
#include <rtc.h>
#include <time.h>

typedef struct DolphinState DolphinState;
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

DolphinState* dolphin_state_alloc();

void dolphin_state_free(DolphinState* dolphin_state);

bool dolphin_state_save(DolphinState* dolphin_state);

bool dolphin_state_load(DolphinState* dolphin_state);

void dolphin_state_clear(DolphinState* dolphin_state);

uint64_t dolphin_state_timestamp();

bool dolphin_state_on_deed(DolphinState* dolphin_state, DolphinDeed deed);

void dolphin_state_butthurted(DolphinState* dolphin_state);

uint32_t dolphin_state_xp_to_levelup(uint32_t icounter);

uint32_t dolphin_state_xp_above_last_levelup(uint32_t icounter);

bool dolphin_state_is_levelup(uint32_t icounter);

void dolphin_state_increase_level(DolphinState* dolphin_state);

uint8_t dolphin_get_level(uint32_t icounter);
