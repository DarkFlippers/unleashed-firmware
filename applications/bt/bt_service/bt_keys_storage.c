#include "bt_keys_storage.h"
#include <furi.h>
#include <file_worker.h>

#define BT_KEYS_STORAGE_TAG "bt keys storage"
#define BT_KEYS_STORAGE_PATH "/int/bt.keys"

bool bt_load_key_storage(Bt* bt) {
    furi_assert(bt);

    bool file_loaded = false;
    furi_hal_bt_get_key_storage_buff(&bt->bt_keys_addr_start, &bt->bt_keys_size);

    FileWorker* file_worker = file_worker_alloc(true);
    if(file_worker_open(file_worker, BT_KEYS_STORAGE_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
        furi_hal_bt_nvm_sram_sem_acquire();
        if(file_worker_read(file_worker, bt->bt_keys_addr_start, bt->bt_keys_size)) {
            file_loaded = true;
        }
        furi_hal_bt_nvm_sram_sem_release();
    }
    file_worker_free(file_worker);
    return file_loaded;
}

bool bt_save_key_storage(Bt* bt) {
    furi_assert(bt);
    furi_assert(bt->bt_keys_addr_start);

    bool file_saved = false;
    FileWorker* file_worker = file_worker_alloc(true);
    if(file_worker_open(file_worker, BT_KEYS_STORAGE_PATH, FSAM_WRITE, FSOM_OPEN_ALWAYS)) {
        furi_hal_bt_nvm_sram_sem_acquire();
        if(file_worker_write(file_worker, bt->bt_keys_addr_start, bt->bt_keys_size)) {
            file_saved = true;
        }
        furi_hal_bt_nvm_sram_sem_release();
    }
    file_worker_free(file_worker);
    return file_saved;
}

bool bt_delete_key_storage(Bt* bt) {
    furi_assert(bt);
    bool delete_succeed = false;

    furi_hal_bt_stop_advertising();
    delete_succeed = furi_hal_bt_clear_white_list();
    if(bt->bt_settings.enabled) {
        furi_hal_bt_start_advertising();
    }

    return delete_succeed;
}
