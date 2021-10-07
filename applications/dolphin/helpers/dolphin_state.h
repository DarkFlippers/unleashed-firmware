#pragma once

#include "dolphin_deed.h"
#include <stdbool.h>
#include <stdint.h>
#include <rtc.h>
#include <time.h>

typedef struct DolphinState DolphinState;

DolphinState* dolphin_state_alloc();

void dolphin_state_free(DolphinState* dolphin_state);

bool dolphin_state_save(DolphinState* dolphin_state);

bool dolphin_state_load(DolphinState* dolphin_state);

void dolphin_state_clear(DolphinState* dolphin_state);

uint64_t dolphin_state_timestamp();

void dolphin_state_on_deed(DolphinState* dolphin_state, DolphinDeed deed);

void dolphin_state_butthurted(DolphinState* dolphin_state);

uint32_t dolphin_state_get_icounter(DolphinState* dolphin_state);

uint32_t dolphin_state_get_butthurt(DolphinState* dolphin_state);

uint64_t dolphin_state_get_timestamp(DolphinState* dolphin_state);

uint32_t dolphin_state_get_level(uint32_t icounter);

uint32_t dolphin_state_xp_to_levelup(uint32_t icounter, uint32_t level, bool remaining);