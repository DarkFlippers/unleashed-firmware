#include "bt_keys_storage.h"

#include <furi.h>
#include <furi_hal_bt.h>
#include <lib/toolbox/saved_struct.h>
#include <storage/storage.h>

#define BT_KEYS_STORAGE_VERSION (0)
#define BT_KEYS_STORAGE_MAGIC   (0x18)

#define TAG "BtKeyStorage"

struct BtKeysStorage {
    uint8_t* nvm_sram_buff;
    uint16_t nvm_sram_buff_size;
    uint16_t current_size;
    FuriString* file_path;
};

bool bt_keys_storage_delete(BtKeysStorage* instance) {
    furi_assert(instance);

    bool delete_succeed = false;
    bool bt_is_active = furi_hal_bt_is_active();

    furi_hal_bt_stop_advertising();
    delete_succeed = furi_hal_bt_clear_white_list();
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

static bool bt_keys_storage_validate_file(const char* file_path, size_t* payload_size) {
    uint8_t magic, version;
    size_t size;

    if(!saved_struct_get_metadata(file_path, &magic, &version, &size)) {
        FURI_LOG_E(TAG, "Failed to get metadata");
        return false;

    } else if(magic != BT_KEYS_STORAGE_MAGIC || version != BT_KEYS_STORAGE_VERSION) {
        FURI_LOG_E(TAG, "File version mismatch");
        return false;
    }

    *payload_size = size;
    return true;
}

bool bt_keys_storage_is_changed(BtKeysStorage* instance) {
    furi_assert(instance);

    bool is_changed = false;
    uint8_t* data_buffer = NULL;

    do {
        const char* file_path = furi_string_get_cstr(instance->file_path);
        size_t payload_size;

        if(!bt_keys_storage_file_exists(file_path)) {
            FURI_LOG_W(TAG, "Missing or empty file");
            break;

        } else if(!bt_keys_storage_validate_file(file_path, &payload_size)) {
            FURI_LOG_E(TAG, "Invalid or corrupted file");
            break;
        }

        data_buffer = malloc(payload_size);

        const bool data_loaded = saved_struct_load(
            file_path, data_buffer, payload_size, BT_KEYS_STORAGE_MAGIC, BT_KEYS_STORAGE_VERSION);

        if(!data_loaded) {
            FURI_LOG_E(TAG, "Failed to load file");
            break;

        } else if(payload_size == instance->current_size) {
            furi_hal_bt_nvm_sram_sem_acquire();
            is_changed = memcmp(data_buffer, instance->nvm_sram_buff, payload_size);
            furi_hal_bt_nvm_sram_sem_release();

        } else {
            FURI_LOG_D(TAG, "Size mismatch");
            is_changed = true;
        }
    } while(false);

    if(data_buffer) {
        free(data_buffer);
    }

    return is_changed;
}

bool bt_keys_storage_load(BtKeysStorage* instance) {
    furi_assert(instance);

    bool loaded = false;

    do {
        const char* file_path = furi_string_get_cstr(instance->file_path);

        // Get payload size
        size_t payload_size;
        if(!bt_keys_storage_validate_file(file_path, &payload_size)) {
            FURI_LOG_E(TAG, "Invalid or corrupted file");
            break;

        } else if(payload_size > instance->nvm_sram_buff_size) {
            FURI_LOG_E(TAG, "NVM RAM buffer overflow");
            break;
        }

        // Load saved data to ram
        furi_hal_bt_nvm_sram_sem_acquire();
        const bool data_loaded = saved_struct_load(
            file_path,
            instance->nvm_sram_buff,
            payload_size,
            BT_KEYS_STORAGE_MAGIC,
            BT_KEYS_STORAGE_VERSION);
        furi_hal_bt_nvm_sram_sem_release();

        if(!data_loaded) {
            FURI_LOG_E(TAG, "Failed to load file");
            break;
        }

        instance->current_size = payload_size;

        loaded = true;
    } while(false);

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

        furi_hal_bt_nvm_sram_sem_acquire();
        bool data_updated = saved_struct_save(
            furi_string_get_cstr(instance->file_path),
            instance->nvm_sram_buff,
            new_size,
            BT_KEYS_STORAGE_MAGIC,
            BT_KEYS_STORAGE_VERSION);
        furi_hal_bt_nvm_sram_sem_release();

        if(!data_updated) {
            FURI_LOG_E(TAG, "Failed to update key storage");
            break;
        }

        updated = true;
    } while(false);

    return updated;
}
