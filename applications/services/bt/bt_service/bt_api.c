#include "bt_i.h"
#include <profiles/serial_profile.h>

FuriHalBleProfileBase* bt_profile_start(
    Bt* bt,
    const FuriHalBleProfileTemplate* profile_template,
    FuriHalBleProfileParams params) {
    furi_check(bt);

    // Send message
    FuriHalBleProfileBase* profile_instance = NULL;

    BtMessage message = {
        .lock = api_lock_alloc_locked(),
        .type = BtMessageTypeSetProfile,
        .profile_instance = &profile_instance,
        .data.profile.params = params,
        .data.profile.template = profile_template,
    };
    furi_check(
        furi_message_queue_put(bt->message_queue, &message, FuriWaitForever) == FuriStatusOk);
    // Wait for unlock
    api_lock_wait_unlock_and_free(message.lock);

    bt->current_profile = profile_instance;
    return profile_instance;
}

bool bt_profile_restore_default(Bt* bt) {
    bt->current_profile = bt_profile_start(bt, ble_profile_serial, NULL);
    return bt->current_profile != NULL;
}

void bt_disconnect(Bt* bt) {
    furi_check(bt);

    // Send message
    BtMessage message = {.lock = api_lock_alloc_locked(), .type = BtMessageTypeDisconnect};
    furi_check(
        furi_message_queue_put(bt->message_queue, &message, FuriWaitForever) == FuriStatusOk);
    // Wait for unlock
    api_lock_wait_unlock_and_free(message.lock);
}

void bt_set_status_changed_callback(Bt* bt, BtStatusChangedCallback callback, void* context) {
    furi_check(bt);

    bt->status_changed_cb = callback;
    bt->status_changed_ctx = context;
}

void bt_forget_bonded_devices(Bt* bt) {
    furi_check(bt);
    BtMessage message = {.type = BtMessageTypeForgetBondedDevices};
    furi_check(
        furi_message_queue_put(bt->message_queue, &message, FuriWaitForever) == FuriStatusOk);
}

void bt_keys_storage_set_storage_path(Bt* bt, const char* keys_storage_path) {
    furi_check(bt);
    furi_check(bt->keys_storage);
    furi_check(keys_storage_path);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FuriString* path = furi_string_alloc_set(keys_storage_path);
    storage_common_resolve_path_and_ensure_app_directory(storage, path);

    bt_keys_storage_set_file_path(bt->keys_storage, furi_string_get_cstr(path));

    furi_string_free(path);
    furi_record_close(RECORD_STORAGE);
}

void bt_keys_storage_set_default_path(Bt* bt) {
    furi_check(bt);
    furi_check(bt->keys_storage);

    bt_keys_storage_set_file_path(bt->keys_storage, BT_KEYS_STORAGE_PATH);
}

/*
 * Private API for the Settings app
 */

void bt_get_settings(Bt* bt, BtSettings* settings) {
    furi_assert(bt);
    furi_assert(settings);

    BtMessage message = {
        .lock = api_lock_alloc_locked(),
        .type = BtMessageTypeGetSettings,
        .data.settings = settings,
    };

    furi_check(
        furi_message_queue_put(bt->message_queue, &message, FuriWaitForever) == FuriStatusOk);

    api_lock_wait_unlock_and_free(message.lock);
}

void bt_set_settings(Bt* bt, const BtSettings* settings) {
    furi_assert(bt);
    furi_assert(settings);

    BtMessage message = {
        .lock = api_lock_alloc_locked(),
        .type = BtMessageTypeSetSettings,
        .data.csettings = settings,
    };

    furi_check(
        furi_message_queue_put(bt->message_queue, &message, FuriWaitForever) == FuriStatusOk);

    api_lock_wait_unlock_and_free(message.lock);
}
