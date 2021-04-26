#pragma once

#include "dolphin_deed.h"
#include <stdbool.h>
#include <stdint.h>

#define DOLPHIN_DATA_PAGE 0xC0
#define DOLPHIN_DATA_HEADER_ADDRESS 0x080C0000U
#define DOLPHIN_DATA_DATA_ADDRESS (DOLPHIN_DATA_HEADER_ADDRESS + sizeof(DolphinDataHeader))

#define DOLPHIN_DATA_HEADER_MAGIC 0xD0
#define DOLPHIN_DATA_HEADER_VERSION 0x01

#define DOLPHIN_LVL_THRESHOLD 20.0f

typedef struct {
    uint8_t magic;
    uint8_t version;
    uint8_t checksum;
    uint8_t flags;
    uint32_t timestamp;
} DolphinDataHeader;

typedef struct {
    uint32_t limit_ibutton;
    uint32_t limit_nfc;
    uint32_t limit_ir;
    uint32_t limit_rfid;

    uint32_t flags;
    uint32_t icounter;
    uint32_t butthurt;
} DolphinData;

struct DolphinState {
    DolphinData data;
};

typedef struct DolphinState DolphinState;

DolphinState* dolphin_state_alloc();

void dolphin_state_free(DolphinState* dolphin_state);

bool dolphin_state_save(DolphinState* dolphin_state);

bool dolphin_state_load(DolphinState* dolphin_state);

void dolphin_state_clear(DolphinState* dolphin_state);

void dolphin_state_on_deed(DolphinState* dolphin_state, DolphinDeed deed);

uint32_t dolphin_state_get_icounter(DolphinState* dolphin_state);

uint32_t dolphin_state_get_butthurt(DolphinState* dolphin_state);

uint32_t dolphin_state_get_level(DolphinState* dolphin_state);

uint32_t dolphin_state_xp_to_levelup(DolphinState* dolphin_state, uint32_t level, bool remaining);