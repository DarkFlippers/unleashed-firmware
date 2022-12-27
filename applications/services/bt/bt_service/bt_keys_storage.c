#include "bt_keys_storage.h"

#include <furi.h>
#include <furi_hal_bt.h>
#include <lib/toolbox/saved_struct.h>
#include <storage/storage.h>

#define BT_KEYS_STORAGE_VERSION (0)
#define BT_KEYS_STORAGE_MAGIC (0x18)

#define TAG "BtKeyStorage"

struct BtKeysStorage {
    uint8_t* nvm_sram_buff;
    uint16_t nvm_sram_buff_size;
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

bool bt_keys_storage_load(BtKeysStorage* instance) {
    furi_assert(instance);

    bool loaded = false;
    do {
        // Get payload size
        size_t payload_size = 0;
        if(!saved_struct_get_payload_size(
               furi_string_get_cstr(instance->file_path),
               BT_KEYS_STORAGE_MAGIC,
               BT_KEYS_STORAGE_VERSION,
               &payload_size)) {
            FURI_LOG_E(TAG, "Failed to read payload size");
            break;
        }

        if(payload_size > instance->nvm_sram_buff_size) {
            FURI_LOG_E(TAG, "Saved data doesn't fit ram buffer");
            break;
        }

        // Load saved data to ram
        furi_hal_bt_nvm_sram_sem_acquire();
        bool data_loaded = saved_struct_load(
            furi_string_get_cstr(instance->file_path),
            instance->nvm_sram_buff,
            payload_size,
            BT_KEYS_STORAGE_MAGIC,
            BT_KEYS_STORAGE_VERSION);
        furi_hal_bt_nvm_sram_sem_release();
        if(!data_loaded) {
            FURI_LOG_E(TAG, "Failed to load struct");
            break;
        }

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
