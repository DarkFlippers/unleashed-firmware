#pragma once

#include "dolphin_deed.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct DolphinState DolphinState;

DolphinState* dolphin_state_alloc();

void dolphin_state_release(DolphinState* dolphin_state);

bool dolphin_state_save(DolphinState* dolphin_state);

bool dolphin_state_load(DolphinState* dolphin_state);

void dolphin_state_clear(DolphinState* dolphin_state);

void dolphin_state_on_deed(DolphinState* dolphin_state, DolphinDeed deed);

uint32_t dolphin_state_get_icounter(DolphinState* dolphin_state);

uint32_t dolphin_state_get_butthurt(DolphinState* dolphin_state);

uint32_t dolphin_state_get_level(DolphinState* dolphin_state);

uint32_t dolphin_state_xp_to_levelup(DolphinState* dolphin_state, uint32_t level, bool remaining);