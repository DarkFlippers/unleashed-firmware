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
