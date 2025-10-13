#include "bt_keys_storage.h"

#include <furi.h>
#include <furi_hal_bt.h>
#include <furi_hal_random.h>

#include <gap.h>

#include <storage/storage.h>
#include <toolbox/saved_struct.h>

#define BT_KEYS_STORAGE_MAGIC          (0x18)
#define BT_KEYS_STORAGE_VERSION        (1)
#define BT_KEYS_STORAGE_LEGACY_VERSION (0) // Legacy version with no root keys

#define TAG "BtKeyStorage"

// Identity root key
static const uint8_t gap_legacy_irk[16] =
    {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0};
// Encryption root key
static const uint8_t gap_legacy_erk[16] =
    {0xfe, 0xdc, 0xba, 0x09, 0x87, 0x65, 0x43, 0x21, 0xfe, 0xdc, 0xba, 0x09, 0x87, 0x65, 0x43, 0x21};

struct BtKeysStorage {
    uint8_t* nvm_sram_buff;
    uint16_t nvm_sram_buff_size;
    uint16_t current_size;
    FuriString* file_path;
    GapRootSecurityKeys root_keys;
};

typedef struct {
    GapRootSecurityKeys root_keys;
    uint8_t pairing_data[];
} BtKeysStorageFile;

static bool bt_keys_storage_save(BtKeysStorage* instance) {
    furi_assert(instance);

    size_t total_size = sizeof(BtKeysStorageFile) + instance->current_size;
    BtKeysStorageFile* save_data = malloc(total_size);

    memcpy(&save_data->root_keys, &instance->root_keys, sizeof(GapRootSecurityKeys));

    furi_hal_bt_nvm_sram_sem_acquire();
    memcpy(save_data->pairing_data, instance->nvm_sram_buff, instance->current_size);
    furi_hal_bt_nvm_sram_sem_release();

    bool saved = saved_struct_save(
        furi_string_get_cstr(instance->file_path),
        save_data,
        sizeof(GapRootSecurityKeys) + instance->current_size,
        BT_KEYS_STORAGE_MAGIC,
        BT_KEYS_STORAGE_VERSION);

    free(save_data);
    return saved;
}

static bool bt_keys_storage_load_keys_and_pairings(
    BtKeysStorage* instance,
    const uint8_t* file_data,
    size_t data_size) {
    if(data_size < sizeof(GapRootSecurityKeys)) {
        return false;
    }

    const BtKeysStorageFile* loaded = (const BtKeysStorageFile*)file_data;
    memcpy(&instance->root_keys, &loaded->root_keys, sizeof(GapRootSecurityKeys));

    size_t ble_data_size = data_size - sizeof(GapRootSecurityKeys);
    if(ble_data_size > instance->nvm_sram_buff_size) {
        FURI_LOG_E(TAG, "BLE data too large for SRAM buffer");
        return false;
    }

    furi_hal_bt_nvm_sram_sem_acquire();
    memcpy(instance->nvm_sram_buff, loaded->pairing_data, ble_data_size);
    instance->current_size = ble_data_size;
    furi_hal_bt_nvm_sram_sem_release();

    return true;
}

static void bt_keys_storage_regenerate_root_keys(BtKeysStorage* instance) {
    furi_hal_random_fill_buf(instance->root_keys.erk, sizeof(instance->root_keys.erk));
    furi_hal_random_fill_buf(instance->root_keys.irk, sizeof(instance->root_keys.irk));
}

bool bt_keys_storage_delete(BtKeysStorage* instance) {
    furi_assert(instance);

    bool delete_succeed = false;
    bool bt_is_active = furi_hal_bt_is_active();

    furi_hal_bt_stop_advertising();
    delete_succeed = furi_hal_bt_clear_white_list();

    FURI_LOG_I(TAG, "Root keys regen");
    bt_keys_storage_regenerate_root_keys(instance);

    instance->current_size = 0;
    if(!bt_keys_storage_save(instance)) {
        FURI_LOG_E(TAG, "Save after delete failed");
    }

    if(bt_is_active) {
        furi_hal_bt_start_advertising();
    }

    return delete_succeed;
}

BtKeysStorage* bt_keys_storage_alloc(const char* keys_storage_path) {
    furi_assert(keys_storage_path);

    BtKeysStorage* instance = malloc(sizeof(BtKeysStorage));
    // Set default nvm ram parameters
    furi_hal_bt_get_key_storage_buff(&instance->nvm_sram_buff, &instance->nvm_sram_buff_size);
    // Set key storage file
    instance->file_path = furi_string_alloc();
    furi_string_set_str(instance->file_path, keys_storage_path);

    bt_keys_storage_regenerate_root_keys(instance);
    return instance;
}

void bt_keys_storage_free(BtKeysStorage* instance) {
    furi_assert(instance);

    furi_string_free(instance->file_path);
    free(instance);
}

void bt_keys_storage_set_file_path(BtKeysStorage* instance, const char* path) {
    furi_assert(instance);
    furi_assert(path);

    furi_string_set_str(instance->file_path, path);
}

void bt_keys_storage_set_ram_params(BtKeysStorage* instance, uint8_t* buff, uint16_t size) {
    furi_assert(instance);
    furi_assert(buff);

    instance->nvm_sram_buff = buff;
    instance->nvm_sram_buff_size = size;
}

static bool bt_keys_storage_file_exists(const char* file_path) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FileInfo file_info;
    const bool ret = storage_common_stat(storage, file_path, &file_info) == FSE_OK &&
                     file_info.size != 0;
    furi_record_close(RECORD_STORAGE);
    return ret;
}

static bool bt_keys_storage_validate_file(
    const char* file_path,
    size_t* payload_size,
    uint8_t* file_version) {
    uint8_t magic, version;
    size_t size;

    if(!saved_struct_get_metadata(file_path, &magic, &version, &size)) {
        FURI_LOG_W(TAG, "Failed to get metadata");
        return false;

    } else if(magic != BT_KEYS_STORAGE_MAGIC) {
        FURI_LOG_W(TAG, "File magic mismatch");
        return false;
    } else if(version > BT_KEYS_STORAGE_VERSION) {
        FURI_LOG_E(
            TAG,
            "File version %d is newer than supported version %d",
            version,
            BT_KEYS_STORAGE_VERSION);
        return false;
    }

    *payload_size = size;
    *file_version = version;
    return true;
}

bool bt_keys_storage_is_changed(BtKeysStorage* instance) {
    furi_assert(instance);

    bool is_changed = false;
    uint8_t* data_buffer = NULL;

    do {
        const char* file_path = furi_string_get_cstr(instance->file_path);
        size_t payload_size;
        uint8_t file_version;

        if(!bt_keys_storage_file_exists(file_path)) {
            FURI_LOG_W(TAG, "Missing or empty file");
            is_changed = true;
            break;
        }

        if(!bt_keys_storage_validate_file(file_path, &payload_size, &file_version)) {
            FURI_LOG_W(TAG, "Invalid or corrupted file");
            is_changed = true;
            break;
        }

        // Early check for legacy version: always considered changed, no need to load
        if(file_version == BT_KEYS_STORAGE_LEGACY_VERSION) {
            is_changed = true;
            break;
        }

        data_buffer = malloc(payload_size);
        const bool data_loaded = saved_struct_load(
            file_path, data_buffer, payload_size, BT_KEYS_STORAGE_MAGIC, file_version);

        if(!data_loaded) {
            FURI_LOG_E(TAG, "Failed to load file");
            break;
        }

        // At this point, it's version 1 file we have
        const BtKeysStorageFile* loaded = (const BtKeysStorageFile*)data_buffer;
        size_t expected_file_size = sizeof(GapRootSecurityKeys) + instance->current_size;
        if(payload_size == expected_file_size) {
            furi_hal_bt_nvm_sram_sem_acquire();
            is_changed =
                memcmp(loaded->pairing_data, instance->nvm_sram_buff, instance->current_size);
            furi_hal_bt_nvm_sram_sem_release();
        } else {
            FURI_LOG_D(TAG, "NVRAM sz mismatch (v1)");
            is_changed = true;
        }
    } while(false);

    if(data_buffer) {
        free(data_buffer);
    }

    return is_changed;
}

static bool bt_keys_storage_load_legacy_pairings(
    BtKeysStorage* instance,
    const uint8_t* file_data,
    size_t payload_size) {
    FURI_LOG_I(TAG, "Loaded v0, upgrading to v1");
    memcpy(instance->root_keys.irk, gap_legacy_irk, sizeof(instance->root_keys.irk));
    memcpy(instance->root_keys.erk, gap_legacy_erk, sizeof(instance->root_keys.erk));
    if(payload_size > instance->nvm_sram_buff_size) {
        FURI_LOG_E(TAG, "Pairing too large");
        return false;
    }
    furi_hal_bt_nvm_sram_sem_acquire();
    memcpy(instance->nvm_sram_buff, file_data, payload_size);
    instance->current_size = payload_size;
    furi_hal_bt_nvm_sram_sem_release();
    if(!bt_keys_storage_save(instance)) {
        FURI_LOG_W(TAG, "Upgrade to v1 failed");
    } else {
        FURI_LOG_I(TAG, "Upgraded to v1");
    }
    return true;
}

bool bt_keys_storage_load(BtKeysStorage* instance) {
    furi_assert(instance);

    const char* file_path = furi_string_get_cstr(instance->file_path);
    size_t payload_size;
    uint8_t file_version;
    if(!bt_keys_storage_validate_file(file_path, &payload_size, &file_version)) {
        FURI_LOG_E(TAG, "Invalid or corrupted file");
        return false;
    }

    bool loaded = false;
    uint8_t* file_data = malloc(payload_size);

    do {
        if(!saved_struct_load(
               file_path, file_data, payload_size, BT_KEYS_STORAGE_MAGIC, file_version)) {
            FURI_LOG_E(TAG, "Failed to load");
            break;
        }

        if(file_version == BT_KEYS_STORAGE_LEGACY_VERSION) {
            loaded = bt_keys_storage_load_legacy_pairings(instance, file_data, payload_size);
            break;
        }
        // Only v1 left
        loaded = bt_keys_storage_load_keys_and_pairings(instance, file_data, payload_size);
    } while(false);

    free(file_data);
    return loaded;
}

bool bt_keys_storage_update(BtKeysStorage* instance, uint8_t* start_addr, uint32_t size) {
    furi_assert(instance);
    furi_assert(start_addr);

    bool updated = false;

    FURI_LOG_I(
        TAG,
        "Base address: %p. Start update address: %p. Size changed: %lu",
        (void*)instance->nvm_sram_buff,
        start_addr,
        size);

    do {
        size_t new_size = start_addr - instance->nvm_sram_buff + size;
        if(new_size > instance->nvm_sram_buff_size) {
            FURI_LOG_E(TAG, "NVM RAM buffer overflow");
            break;
        }

        instance->current_size = new_size;

        // Save using version 1 format with embedded root keys
        bool data_updated = bt_keys_storage_save(instance);

        if(!data_updated) {
            FURI_LOG_E(TAG, "Failed to update key storage");
            break;
        }

        updated = true;
    } while(false);

    return updated;
}

const GapRootSecurityKeys* bt_keys_storage_get_root_keys(BtKeysStorage* instance) {
    furi_assert(instance);

    return &instance->root_keys;
}
