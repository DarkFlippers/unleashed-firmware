#include "../lfrfid_i.h"
#include <bit_lib.h>
#include "gui/scene_manager.h"

// Every verdict but success draws the same screen with three things changed, so they are a table
// rather than nine copies of the geometry. The order is the order they are reached in: what was
// on the coil, then what the write made of it.
typedef struct {
    LFRFIDWorkerWriteResult result;
    uint32_t event;
    const char* header;
    const char* text;
    const NotificationSequence* sound;
} LfRfidSetPassVerdict;

static const LfRfidSetPassVerdict lfrfid_set_pass_verdicts[] = {
    {LFRFIDWorkerWriteProtocolCannotBeWritten,
     LfRfidEventWriteProtocolCannotBeWritten,
     "Error",
     "This protocol cannot\nbe written with a\npassword",
     &sequence_blink_start_red},
    // Nothing answered a block read, and nothing answered the ordinary reader either.
    {LFRFIDWorkerWriteTooLongToWrite,
     LfRfidEventWriteTooLongToWrite,
     "Still trying to write...",
     "Make sure this\ncard is writable\nand not\nprotected.",
     &sequence_blink_start_yellow},
    // Something is there, but not something whose blocks can be read back - and every step of
    // setting a password rests on reading one back.
    {LFRFIDWorkerWriteCannotRead,
     LfRfidEventWriteCannotRead,
     "Cannot read it",
     "A tag is there, but\nnot one this can read\nblock by block. Wipe\nit first, or clear\nits password.",
     &sequence_blink_start_red},
    // Writing a protected tag needs the password it already has, which is a different job: this
    // one only ever sends frames that carry none.
    {LFRFIDWorkerWriteAlreadyProtected,
     LfRfidEventWriteAlreadyProtected,
     "Has a password",
     "Clear it first:\nthe T5577 app,\nClear Password",
     &sequence_blink_start_red},
    // Part of the key is on the tag and nothing is protected, which is a different thing to tell
    // someone than "the card cannot be written".
    {LFRFIDWorkerWriteKeyNotStored,
     LfRfidEventWriteKeyNotStored,
     "Not written",
     "A block would not\ntake the key, so the\ntag holds part of it\nand no password.",
     &sequence_blink_start_red},
    // Nothing was protected, so the tag is still open - which is the whole point of reading
    // block 7 back before block 0 is touched.
    {LFRFIDWorkerWritePasswordNotStored,
     LfRfidEventWritePasswordNotStored,
     "Not protected",
     "Block 7 would not\ntake the password.\nThe key is written\nand the tag is open.",
     &sequence_blink_start_red},
    // The password is on the chip and was read back off it, so whoever turns protection on later
    // - this tool or another - will find the tag asking for that word.
    {LFRFIDWorkerWriteProtectionRefused,
     LfRfidEventWriteProtectionRefused,
     "Not protected",
     "Block 7 holds the\npassword, but the\ntag refused to\nturn it on.",
     &sequence_blink_start_red},
    // Not a failure and not a success: the password is on the chip, block 0 was sent, and this
    // reader cannot see the answer - either the tag left, or the configuration it was just given
    // is one it does not decode.
    {LFRFIDWorkerWriteProtectionUnchecked,
     LfRfidEventWriteProtectionUnchecked,
     "Not confirmed",
     "The password is on\nthe tag. Whether it\nasks for it yet was\nnot read back.",
     &sequence_blink_start_yellow},
};

static const LfRfidSetPassVerdict* lfrfid_set_pass_verdict_for_event(uint32_t event) {
    for(size_t i = 0; i < COUNT_OF(lfrfid_set_pass_verdicts); i++) {
        if(lfrfid_set_pass_verdicts[i].event == event) return &lfrfid_set_pass_verdicts[i];
    }

    return NULL;
}

// The protocol, and under it whatever the worker is on. Every stage of this is a block write
// followed by a read of the same block, a second or more each: a popup that says nothing for that
// long is one the user is entitled to read as hung.
static void lfrfid_scene_write_and_set_pass_doing(LfRfid* app, const char* stage) {
    snprintf(
        app->text_store,
        LFRFID_TEXT_STORE_SIZE,
        "[%s]\n%s",
        protocol_dict_get_name(app->dict, app->protocol_id),
        stage);
}

static void lfrfid_write_and_set_pass_callback(LFRFIDWorkerWriteResult result, void* context) {
    LfRfid* app = context;

    if(result == LFRFIDWorkerWriteOK) {
        view_dispatcher_send_custom_event(app->view_dispatcher, LfRfidEventWriteOK);
        return;
    }

    if(result == LFRFIDWorkerWriteStartTarget) {
        view_dispatcher_send_custom_event(app->view_dispatcher, LfRfidEventWriteProgress);
        return;
    }

    for(size_t i = 0; i < COUNT_OF(lfrfid_set_pass_verdicts); i++) {
        if(lfrfid_set_pass_verdicts[i].result != result) continue;

        view_dispatcher_send_custom_event(app->view_dispatcher, lfrfid_set_pass_verdicts[i].event);
        return;
    }

    // Custom event 0 is not a LfRfidCustomEvent, so an unmapped result would be dropped without
    // a trace rather than reaching a scene.
    FURI_LOG_E("LfRfid", "Unhandled write result %d", result);
}

void lfrfid_scene_write_and_set_pass_on_enter(void* context) {
    LfRfid* app = context;
    Popup* popup = app->popup;

    popup_set_header(popup, "Writing\nwith\npassword", 94, 4, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 8, &I_NFC_manual_60x50);
    // Mirror the Write scene: show the protocol and the target chip. This path always writes a
    // T5577 (the only chip that supports a password), so the target is fixed.
    lfrfid_scene_write_and_set_pass_doing(app, "(T5577)");
    popup_set_text(popup, app->text_store, 94, 41, AlignCenter, AlignTop);

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewPopup);

    // Confirming the tag is still there means reading it as a reader would, which decodes into
    // the dictionary over the key being written. It goes back on the way out.
    size_t size = protocol_dict_get_data_size(app->dict, app->protocol_id);
    protocol_dict_get_data(app->dict, app->protocol_id, app->old_key_data, size);

    lfrfid_worker_set_password(
        app->lfworker, bit_lib_bytes_to_num_be(app->password, sizeof(app->password)));
    lfrfid_worker_start_thread(app->lfworker);
    lfrfid_worker_write_and_set_pass_start(
        app->lfworker, (LFRFIDProtocol)app->protocol_id, lfrfid_write_and_set_pass_callback, app);
    notification_message(app->notifications, &sequence_blink_start_magenta);
}

bool lfrfid_scene_write_and_set_pass_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;

    if(event.event == LfRfidEventWriteProgress) {
        lfrfid_scene_write_and_set_pass_doing(
            app, lfrfid_worker_get_write_chip_name(app->lfworker));
        // The popup holds the buffer by pointer, so rewriting it in place is the whole update.
        popup_set_text(app->popup, app->text_store, 94, 41, AlignCenter, AlignTop);
        return true;
    }

    if(event.event == LfRfidEventWriteOK) {
        notification_message(app->notifications, &sequence_success);
        scene_manager_next_scene(app->scene_manager, LfRfidSceneWriteSuccess);
        return true;
    }

    const LfRfidSetPassVerdict* verdict = lfrfid_set_pass_verdict_for_event(event.event);
    if(!verdict) return false;

    popup_set_icon(app->popup, 83, 22, &I_WarningDolphinFlip_45x42);
    popup_set_header(app->popup, verdict->header, 64, 3, AlignCenter, AlignTop);
    popup_set_text(app->popup, verdict->text, 3, 17, AlignLeft, AlignTop);
    notification_message(app->notifications, verdict->sound);

    return true;
}

void lfrfid_scene_write_and_set_pass_on_exit(void* context) {
    LfRfid* app = context;
    notification_message(app->notifications, &sequence_blink_stop);
    popup_reset(app->popup);
    lfrfid_worker_stop(app->lfworker);
    lfrfid_worker_stop_thread(app->lfworker);

    size_t size = protocol_dict_get_data_size(app->dict, app->protocol_id);
    protocol_dict_set_data(app->dict, app->protocol_id, app->old_key_data, size);
}
