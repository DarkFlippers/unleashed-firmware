#include "bt_i.h"

bool bt_set_profile(Bt* bt, BtProfile profile) {
    furi_assert(bt);

    // Send message
    bool result = false;
    BtMessage message = {
        .type = BtMessageTypeSetProfile, .data.profile = profile, .result = &result};
    furi_check(
        furi_message_queue_put(bt->message_queue, &message, FuriWaitForever) == FuriStatusOk);
    // Wait for unlock
    furi_event_flag_wait(bt->api_event, BT_API_UNLOCK_EVENT, FuriFlagWaitAny, FuriWaitForever);

    return result;
}

void bt_disconnect(Bt* bt) {
    furi_assert(bt);

    // Send message
    BtMessage message = {.type = BtMessageTypeDisconnect};
    furi_check(
        furi_message_queue_put(bt->message_queue, &message, FuriWaitForever) == FuriStatusOk);
    // Wait for unlock
    furi_event_flag_wait(bt->api_event, BT_API_UNLOCK_EVENT, FuriFlagWaitAny, FuriWaitForever);
}

void bt_set_status_changed_callback(Bt* bt, BtStatusChangedCallback callback, void* context) {
    furi_assert(bt);

    bt->status_changed_cb = callback;
    bt->status_changed_ctx = context;
}

void bt_forget_bonded_devices(Bt* bt) {
    furi_assert(bt);
    BtMessage message = {.type = BtMessageTypeForgetBondedDevices};
    furi_check(
        furi_message_queue_put(bt->message_queue, &message, FuriWaitForever) == FuriStatusOk);
}

void bt_keys_storage_set_storage_path(Bt* bt, const char* keys_storage_path) {
    furi_assert(bt);
    furi_assert(bt->keys_storage);
    furi_assert(keys_storage_path);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FuriString* path = furi_string_alloc_set(keys_storage_path);
    storage_common_resolve_path_and_ensure_app_directory(storage, path);

    bt_keys_storage_set_file_path(bt->keys_storage, furi_string_get_cstr(path));

    furi_string_free(path);
    furi_record_close(RECORD_STORAGE);
}

void bt_keys_storage_set_default_path(Bt* bt) {
    furi_assert(bt);
    furi_assert(bt->keys_storage);

    bt_keys_storage_set_file_path(bt->keys_storage, BT_KEYS_STORAGE_PATH);
}
