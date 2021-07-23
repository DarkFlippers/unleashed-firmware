#include "dolphin_state.h"
#include <storage/storage.h>
#include <furi.h>
#include <math.h>

#define DOLPHIN_STORE_KEY "/int/dolphin.state"
#define DOLPHIN_STORE_HEADER_MAGIC 0xD0
#define DOLPHIN_STORE_HEADER_VERSION 0x01
#define DOLPHIN_LVL_THRESHOLD 20.0f

typedef struct {
    uint8_t magic;
    uint8_t version;
    uint8_t checksum;
    uint8_t flags;
    uint32_t timestamp;
} DolphinStoreHeader;

typedef struct {
    uint32_t limit_ibutton;
    uint32_t limit_nfc;
    uint32_t limit_ir;
    uint32_t limit_rfid;

    uint32_t flags;
    uint32_t icounter;
    uint32_t butthurt;
} DolphinStoreData;

typedef struct {
    DolphinStoreHeader header;
    DolphinStoreData data;
} DolphinStore;

struct DolphinState {
    Storage* fs_api;
    DolphinStoreData data;
};

DolphinState* dolphin_state_alloc() {
    DolphinState* dolphin_state = furi_alloc(sizeof(DolphinState));
    dolphin_state->fs_api = furi_record_open("storage");
    return dolphin_state;
}

void dolphin_state_free(DolphinState* dolphin_state) {
    furi_record_close("storage");
    free(dolphin_state);
}

bool dolphin_state_save(DolphinState* dolphin_state) {
    DolphinStore store;
    FURI_LOG_I("dolphin-state", "Saving state to \"%s\"", DOLPHIN_STORE_KEY);
    // Calculate checksum
    uint8_t* source = (uint8_t*)&dolphin_state->data;
    uint8_t checksum = 0;
    for(size_t i = 0; i < sizeof(DolphinStoreData); i++) {
        checksum += source[i];
    }
    // Set header
    store.header.magic = DOLPHIN_STORE_HEADER_MAGIC;
    store.header.version = DOLPHIN_STORE_HEADER_VERSION;
    store.header.checksum = checksum;
    store.header.flags = 0;
    store.header.timestamp = 0;
    // Set data
    store.data = dolphin_state->data;

    // Store
    File* file = storage_file_alloc(dolphin_state->fs_api);
    bool save_result = storage_file_open(file, DOLPHIN_STORE_KEY, FSAM_WRITE, FSOM_CREATE_ALWAYS);

    if(save_result) {
        uint16_t bytes_count = storage_file_write(file, &store, sizeof(DolphinStore));

        if(bytes_count != sizeof(DolphinStore)) {
            save_result = false;
        }
    }

    if(!save_result) {
        FURI_LOG_E(
            "dolphin-state",
            "Save failed. Storage returned: %s",
            storage_file_get_error_desc(file));
    }

    storage_file_close(file);
    storage_file_free(file);

    FURI_LOG_I("dolphin-state", "Saved");
    return save_result;
}

bool dolphin_state_load(DolphinState* dolphin_state) {
    DolphinStore store;
    // Read Dolphin State Store
    FURI_LOG_I("dolphin-state", "Loading state from \"%s\"", DOLPHIN_STORE_KEY);

    File* file = storage_file_alloc(dolphin_state->fs_api);
    bool load_result = storage_file_open(file, DOLPHIN_STORE_KEY, FSAM_READ, FSOM_OPEN_EXISTING);

    if(load_result) {
        uint16_t bytes_count = storage_file_read(file, &store, sizeof(DolphinStore));

        if(bytes_count != sizeof(DolphinStore)) {
            load_result = false;
        }
    }

    if(!load_result) {
        FURI_LOG_E(
            "dolphin-state",
            "Load failed. Storage returned: %s",
            storage_file_get_error_desc(file));
    } else {
        FURI_LOG_I("dolphin-state", "State loaded, verifying header");
        if(store.header.magic == DOLPHIN_STORE_HEADER_MAGIC &&
           store.header.version == DOLPHIN_STORE_HEADER_VERSION) {
            FURI_LOG_I(
                "dolphin-state",
                "Magic(%d) and Version(%d) match",
                store.header.magic,
                store.header.version);
            uint8_t checksum = 0;
            const uint8_t* source = (const uint8_t*)&store.data;
            for(size_t i = 0; i < sizeof(DolphinStoreData); i++) {
                checksum += source[i];
            }

            if(store.header.checksum == checksum) {
                FURI_LOG_I("dolphin-state", "Checksum(%d) match", store.header.checksum);
                dolphin_state->data = store.data;
            } else {
                FURI_LOG_E(
                    "dolphin-state",
                    "Checksum(%d != %d) mismatch",
                    store.header.checksum,
                    checksum);
                load_result = false;
            }
        } else {
            FURI_LOG_E(
                "dolphin-state",
                "Magic(%d != %d) and Version(%d != %d) mismatch",
                store.header.magic,
                DOLPHIN_STORE_HEADER_MAGIC,
                store.header.version,
                DOLPHIN_STORE_HEADER_VERSION);
            load_result = false;
        }
    }

    storage_file_close(file);
    storage_file_free(file);
    return load_result;
}

void dolphin_state_clear(DolphinState* dolphin_state) {
    memset(&dolphin_state->data, 0, sizeof(DolphinStoreData));
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

uint32_t dolphin_state_get_level(DolphinState* dolphin_state) {
    return 0.5f +
           sqrtf(1.0f + 8.0f * ((float)dolphin_state->data.icounter / DOLPHIN_LVL_THRESHOLD)) /
               2.0f;
}

uint32_t dolphin_state_xp_to_levelup(DolphinState* dolphin_state, uint32_t level, bool remaining) {
    return (DOLPHIN_LVL_THRESHOLD * level * (level + 1) / 2) -
           (remaining ? dolphin_state->data.icounter : 0);
}