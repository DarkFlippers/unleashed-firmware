#include "../lfrfid_i.h"
#include "bit_lib.h"
#include "gui/scene_manager.h"

static void lfrfid_write_and_set_pass_callback(LFRFIDWorkerWriteResult result, void* context) {
    LfRfid* app = context;
    uint32_t event = 0;

    if(result == LFRFIDWorkerWriteOK) {
        event = LfRfidEventWriteOK;
    } else if(result == LFRFIDWorkerWriteProtocolCannotBeWritten) {
        event = LfRfidEventWriteProtocolCannotBeWritten;
    } else if(result == LFRFIDWorkerWriteFobCannotBeWritten) {
        event = LfRfidEventWriteFobCannotBeWritten;
    } else if(result == LFRFIDWorkerWriteTooLongToWrite) {
        event = LfRfidEventWriteTooLongToWrite;
    } else if(result == LFRFIDWorkerWriteAlreadyProtected) {
        event = LfRfidEventWriteAlreadyProtected;
    } else if(result == LFRFIDWorkerWritePasswordNotStored) {
        event = LfRfidEventWritePasswordNotStored;
    } else if(result == LFRFIDWorkerWriteProtectionRefused) {
        event = LfRfidEventWriteProtectionRefused;
    } else {
        // Custom event 0 is not a LfRfidCustomEvent, so an unmapped result would be dropped
        // without a trace rather than reaching a scene.
        FURI_LOG_E("LfRfid", "Unhandled write result %d", result);
        return;
    }

    view_dispatcher_send_custom_event(app->view_dispatcher, event);
}

void lfrfid_scene_write_and_set_pass_on_enter(void* context) {
    LfRfid* app = context;
    Popup* popup = app->popup;

    popup_set_header(popup, "Writing\nwith\npassword", 94, 4, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 8, &I_NFC_manual_60x50);
    // Mirror the Write scene: show the protocol and the target chip. This path always writes a
    // T5577 (the only chip that supports a password), so the target is fixed.
    snprintf(
        app->text_store,
        LFRFID_TEXT_STORE_SIZE,
        "[%s]\n(T5577)",
        protocol_dict_get_name(app->dict, app->protocol_id));
    popup_set_text(popup, app->text_store, 94, 41, AlignCenter, AlignTop);

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewPopup);

    lfrfid_worker_start_thread(app->lfworker);
    lfrfid_worker_set_password(app->lfworker, bit_lib_bytes_to_num_be(app->password, 4));
    lfrfid_worker_write_and_set_pass_start(
        app->lfworker, (LFRFIDProtocol)app->protocol_id, lfrfid_write_and_set_pass_callback, app);
    notification_message(app->notifications, &sequence_blink_start_magenta);
}

bool lfrfid_scene_write_and_set_pass_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
    Popup* popup = app->popup;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == LfRfidEventWriteOK) {
            notification_message(app->notifications, &sequence_success);
            scene_manager_next_scene(app->scene_manager, LfRfidSceneWriteSuccess);
            consumed = true;
        } else if(event.event == LfRfidEventWriteProtocolCannotBeWritten) {
            popup_set_icon(popup, 83, 22, &I_WarningDolphinFlip_45x42);
            popup_set_header(popup, "Error", 64, 3, AlignCenter, AlignTop);
            popup_set_text(popup, "This protocol\ncannot be written", 3, 17, AlignLeft, AlignTop);
            notification_message(app->notifications, &sequence_blink_start_red);
            consumed = true;
        } else if(event.event == LfRfidEventWriteAlreadyProtected) {
            // Writing a protected tag needs the password it already has, which is a different
            // job: this one only ever sends frames that carry none.
            popup_set_icon(popup, 83, 22, &I_WarningDolphinFlip_45x42);
            popup_set_header(popup, "Has a password", 64, 3, AlignCenter, AlignTop);
            popup_set_text(
                popup,
                "Clear it first:\nExtra Actions ->\nClear T5577\nPassword",
                3,
                17,
                AlignLeft,
                AlignTop);
            notification_message(app->notifications, &sequence_blink_start_red);
            consumed = true;
        } else if(event.event == LfRfidEventWritePasswordNotStored) {
            // Nothing was protected, so the tag is still open - which is the whole point of
            // reading block 7 back before block 0 is touched.
            popup_set_icon(popup, 83, 22, &I_WarningDolphinFlip_45x42);
            popup_set_header(popup, "Not protected", 64, 3, AlignCenter, AlignTop);
            popup_set_text(
                popup,
                "Block 7 would not\ntake the password,\nso the tag was\nleft open.",
                3,
                17,
                AlignLeft,
                AlignTop);
            notification_message(app->notifications, &sequence_blink_start_red);
            consumed = true;
        } else if(event.event == LfRfidEventWriteProtectionRefused) {
            // The password is on the chip and was read back off it, so whoever turns protection
            // on later - this tool or another - will find the tag asking for that word.
            popup_set_icon(popup, 83, 22, &I_WarningDolphinFlip_45x42);
            popup_set_header(popup, "Not protected", 64, 3, AlignCenter, AlignTop);
            popup_set_text(
                popup,
                "Block 7 holds the\npassword, but the\ntag refused to\nturn it on.",
                3,
                17,
                AlignLeft,
                AlignTop);
            notification_message(app->notifications, &sequence_blink_start_red);
            consumed = true;
        } else if(
            (event.event == LfRfidEventWriteFobCannotBeWritten) ||
            (event.event == LfRfidEventWriteTooLongToWrite)) {
            popup_set_icon(popup, 83, 22, &I_WarningDolphinFlip_45x42);
            popup_set_header(popup, "Still trying to write...", 64, 3, AlignCenter, AlignTop);
            popup_set_text(
                popup,
                "Make sure this\ncard is writable\nand not\nprotected.",
                3,
                17,
                AlignLeft,
                AlignTop);
            notification_message(app->notifications, &sequence_blink_start_yellow);
            consumed = true;
        }
    }

    return consumed;
}

void lfrfid_scene_write_and_set_pass_on_exit(void* context) {
    LfRfid* app = context;
    notification_message(app->notifications, &sequence_blink_stop);
    popup_reset(app->popup);
    lfrfid_worker_stop(app->lfworker);
    lfrfid_worker_stop_thread(app->lfworker);
}
