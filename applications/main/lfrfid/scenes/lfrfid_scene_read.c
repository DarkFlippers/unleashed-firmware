#include "../lfrfid_i.h"
#include <dolphin/dolphin.h>

static const NotificationSequence sequence_blink_set_yellow = {
    &message_blink_set_color_yellow,
    NULL,
};

static const NotificationSequence sequence_blink_set_green = {
    &message_blink_set_color_green,
    NULL,
};

static const NotificationSequence sequence_blink_set_cyan = {
    &message_blink_set_color_cyan,
    NULL,
};

static void
    lfrfid_read_callback(LFRFIDWorkerReadResult result, ProtocolId protocol, void* context) {
    LfRfid* app = context;
    uint32_t event = 0;

    if(result == LFRFIDWorkerReadSenseStart) {
        event = LfRfidEventReadSenseStart;
    } else if(result == LFRFIDWorkerReadSenseEnd) {
        event = LfRfidEventReadSenseEnd;
    } else if(result == LFRFIDWorkerReadSenseCardStart) {
        event = LfRfidEventReadSenseCardStart;
    } else if(result == LFRFIDWorkerReadSenseCardEnd) {
        event = LfRfidEventReadSenseCardEnd;
    } else if(result == LFRFIDWorkerReadDone) {
        event = LfRfidEventReadDone;
        app->protocol_id_next = protocol;
    } else if(result == LFRFIDWorkerReadStartASK) {
        event = LfRfidEventReadStartASK;
    } else if(result == LFRFIDWorkerReadStartPSK) {
        event = LfRfidEventReadStartPSK;
    } else {
        return;
    }

    view_dispatcher_send_custom_event(app->view_dispatcher, event);
}

void lfrfid_scene_read_on_enter(void* context) {
    LfRfid* app = context;

    if(app->read_type == LFRFIDWorkerReadTypePSKOnly) {
        lfrfid_view_read_set_read_mode(app->read_view, LfRfidReadPskOnly);
    } else if(app->read_type == LFRFIDWorkerReadTypeASKOnly) {
        lfrfid_view_read_set_read_mode(app->read_view, LfRfidReadAskOnly);
    }

    lfrfid_worker_start_thread(app->lfworker);
    lfrfid_worker_read_start(app->lfworker, app->read_type, lfrfid_read_callback, app);

    notification_message(app->notifications, &sequence_blink_start_cyan);

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewRead);
}

bool lfrfid_scene_read_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == LfRfidEventReadSenseStart) {
            notification_message(app->notifications, &sequence_blink_set_yellow);
            consumed = true;
        } else if(event.event == LfRfidEventReadSenseCardStart) {
            notification_message(app->notifications, &sequence_blink_set_green);
            consumed = true;
        } else if(
            (event.event == LfRfidEventReadSenseEnd) ||
            (event.event == LfRfidEventReadSenseCardEnd)) {
            notification_message(app->notifications, &sequence_blink_set_cyan);
            consumed = true;
        } else if(event.event == LfRfidEventReadDone) {
            app->protocol_id = app->protocol_id_next;
            notification_message(app->notifications, &sequence_success);
            furi_string_reset(app->file_name);
            scene_manager_next_scene(app->scene_manager, LfRfidSceneReadSuccess);
            dolphin_deed(DolphinDeedRfidReadSuccess);
            consumed = true;
        } else if(event.event == LfRfidEventReadStartPSK) {
            if(app->read_type == LFRFIDWorkerReadTypeAuto) {
                lfrfid_view_read_set_read_mode(app->read_view, LfRfidReadPsk);
            }
            consumed = true;
        } else if(event.event == LfRfidEventReadStartASK) {
            if(app->read_type == LFRFIDWorkerReadTypeAuto) {
                lfrfid_view_read_set_read_mode(app->read_view, LfRfidReadAsk);
            }
            consumed = true;
        }
    }

    return consumed;
}

void lfrfid_scene_read_on_exit(void* context) {
    LfRfid* app = context;
    notification_message(app->notifications, &sequence_blink_stop);
    popup_reset(app->popup);
    lfrfid_worker_stop(app->lfworker);
    lfrfid_worker_stop_thread(app->lfworker);
}
