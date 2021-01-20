#include "dolphin_state.h"
#include <furi.h>

typedef struct {
    uint8_t magic;
    uint8_t version;
    uint8_t checksum;
    uint8_t flags;
    uint32_t timestamp;
} DolphinDataHeader;

#define DOLPHIN_DATA_PAGE 0xC0
#define DOLPHIN_DATA_HEADER_ADDRESS 0x080C0000U
#define DOLPHIN_DATA_DATA_ADDRESS (DOLPHIN_DATA_HEADER_ADDRESS + sizeof(DolphinDataHeader))

#define DOLPHIN_DATA_HEADER_MAGIC 0xD0
#define DOLPHIN_DATA_HEADER_VERSION 0x00

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

DolphinState* dolphin_state_alloc() {
    DolphinState* dolphin_state = furi_alloc(sizeof(DolphinState));
    return dolphin_state;
}

void dolphin_state_release(DolphinState* dolphin_state) {
    free(dolphin_state);
}

bool dolphin_state_save(DolphinState* dolphin_state) {
    if(!api_hal_flash_erase(DOLPHIN_DATA_PAGE, 1)) {
        return false;
    }

    uint8_t* source = (uint8_t*)&dolphin_state->data;
    uint8_t checksum = 0;
    for(size_t i = 0; i < sizeof(DolphinData); i++) {
        checksum += source[i];
    }
    DolphinDataHeader header;
    header.magic = DOLPHIN_DATA_HEADER_MAGIC;
    header.version = DOLPHIN_DATA_HEADER_VERSION;
    header.checksum = checksum;
    header.flags = 0;
    header.timestamp = 0;
    if(!api_hal_flash_write_dword(DOLPHIN_DATA_HEADER_ADDRESS, *(uint64_t*)&header)) {
        return false;
    }

    uint8_t destination[sizeof(uint64_t)];
    size_t block_count = sizeof(DolphinData) / sizeof(uint64_t) + 1;
    size_t offset = 0;
    for(size_t i = 0; i < block_count; i++) {
        for(size_t n = 0; n < sizeof(uint64_t); n++) {
            if(offset < sizeof(DolphinData)) {
                destination[n] = source[offset];
            } else {
                destination[n] = 0;
            }
            offset++;
        }
        if(!api_hal_flash_write_dword(
               DOLPHIN_DATA_DATA_ADDRESS + i * sizeof(uint64_t), *(uint64_t*)destination)) {
            return false;
        }
    }
    return true;
}

bool dolphin_state_load(DolphinState* dolphin_state) {
    const DolphinDataHeader* header = (const DolphinDataHeader*)DOLPHIN_DATA_HEADER_ADDRESS;
    if(header->magic == DOLPHIN_DATA_HEADER_MAGIC &&
       header->version == DOLPHIN_DATA_HEADER_VERSION) {
        uint8_t checksum = 0;
        const uint8_t* source = (const uint8_t*)DOLPHIN_DATA_DATA_ADDRESS;
        for(size_t i = 0; i < sizeof(DolphinData); i++) {
            checksum += source[i];
        }
        if(header->checksum == checksum) {
            memcpy(
                &dolphin_state->data, (const void*)DOLPHIN_DATA_DATA_ADDRESS, sizeof(DolphinData));
            return true;
        }
    }
    return false;
}

void dolphin_state_clear(DolphinState* dolphin_state) {
    memset(&dolphin_state->data, 0, sizeof(DolphinData));
}

void dolphin_state_on_deed(DolphinState* dolphin_state, DolphinDeed deed) {
    const DolphinDeedWeight* deed_weight = dolphin_deed_weight(deed);
    int32_t icounter = dolphin_state->data.icounter + deed_weight->icounter;

    if(icounter >= 0) {
        dolphin_state->data.icounter = icounter;
    }
}

uint32_t dolphin_state_get_icounter(DolphinState* dolphin_state) {
    return dolphin_state->data.icounter;
}

uint32_t dolphin_state_get_butthurt(DolphinState* dolphin_state) {
    return dolphin_state->data.butthurt;
}
