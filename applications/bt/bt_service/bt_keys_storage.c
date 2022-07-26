#include "bt_keys_storage.h"

#include <furi.h>
#include <lib/toolbox/saved_struct.h>
#include <storage/storage.h>

#define BT_KEYS_STORAGE_PATH INT_PATH(BT_KEYS_STORAGE_FILE_NAME)
#define BT_KEYS_STORAGE_VERSION (0)
#define BT_KEYS_STORAGE_MAGIC (0x18)

bool bt_keys_storage_load(Bt* bt) {
    furi_assert(bt);
    bool file_loaded = false;

    furi_hal_bt_get_key_storage_buff(&bt->bt_keys_addr_start, &bt->bt_keys_size);
    furi_hal_bt_nvm_sram_sem_acquire();
    file_loaded = saved_struct_load(
        BT_KEYS_STORAGE_PATH,
        bt->bt_keys_addr_start,
        bt->bt_keys_size,
        BT_KEYS_STORAGE_MAGIC,
        BT_KEYS_STORAGE_VERSION);
    furi_hal_bt_nvm_sram_sem_release();

    return file_loaded;
}

bool bt_keys_storage_save(Bt* bt) {
    furi_assert(bt);
    furi_assert(bt->bt_keys_addr_start);
    bool file_saved = false;

    furi_hal_bt_nvm_sram_sem_acquire();
    file_saved = saved_struct_save(
        BT_KEYS_STORAGE_PATH,
        bt->bt_keys_addr_start,
        bt->bt_keys_size,
        BT_KEYS_STORAGE_MAGIC,
        BT_KEYS_STORAGE_VERSION);
    furi_hal_bt_nvm_sram_sem_release();

    return file_saved;
}

bool bt_keys_storage_delete(Bt* bt) {
    furi_assert(bt);
    bool delete_succeed = false;
    bool bt_is_active = furi_hal_bt_is_active();

    furi_hal_bt_stop_advertising();
    delete_succeed = furi_hal_bt_clear_white_list();
    if(bt_is_active) {
        furi_hal_bt_start_advertising();
    }

    return delete_succeed;
}
