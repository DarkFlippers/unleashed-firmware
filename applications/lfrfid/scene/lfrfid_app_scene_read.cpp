#include "lfrfid_app_scene_read.h"
#include <dolphin/dolphin.h>

static void lfrfid_read_callback(LFRFIDWorkerReadResult result, ProtocolId protocol, void* ctx) {
    LfRfidApp* app = static_cast<LfRfidApp*>(ctx);
    LfRfidApp::Event event;

    switch(result) {
    case LFRFIDWorkerReadSenseStart:
        event.type = LfRfidApp::EventType::ReadEventSenseStart;
        break;
    case LFRFIDWorkerReadSenseEnd:
        event.type = LfRfidApp::EventType::ReadEventSenseEnd;
        break;
    case LFRFIDWorkerReadSenseCardStart:
        event.type = LfRfidApp::EventType::ReadEventSenseCardStart;
        break;
    case LFRFIDWorkerReadSenseCardEnd:
        event.type = LfRfidApp::EventType::ReadEventSenseCardEnd;
        break;
    case LFRFIDWorkerReadDone:
        event.type = LfRfidApp::EventType::ReadEventDone;
        break;
    case LFRFIDWorkerReadStartASK:
        event.type = LfRfidApp::EventType::ReadEventStartASK;
        break;
    case LFRFIDWorkerReadStartPSK:
        event.type = LfRfidApp::EventType::ReadEventStartPSK;
        break;
    }

    event.payload.signed_int = protocol;

    app->view_controller.send_event(&event);
}

void LfRfidAppSceneRead::on_enter(LfRfidApp* app, bool /* need_restore */) {
    auto popup = app->view_controller.get<PopupVM>();

    DOLPHIN_DEED(DolphinDeedRfidRead);
    if(app->read_type == LFRFIDWorkerReadTypePSKOnly) {
        popup->set_header("Reading\nLF RFID\nPSK", 89, 30, AlignCenter, AlignTop);
    } else {
        popup->set_header("Reading\nLF RFID\nASK", 89, 30, AlignCenter, AlignTop);
    }

    popup->set_icon(0, 3, &I_RFIDDolphinReceive_97x61);

    app->view_controller.switch_to<PopupVM>();
    lfrfid_worker_start_thread(app->lfworker);
    lfrfid_worker_read_start(app->lfworker, app->read_type, lfrfid_read_callback, app);

    notification_message(app->notification, &sequence_blink_start_cyan);
}

bool LfRfidAppSceneRead::on_event(LfRfidApp* app, LfRfidApp::Event* event) {
    bool consumed = true;
    auto popup = app->view_controller.get<PopupVM>();

    switch(event->type) {
    case LfRfidApp::EventType::ReadEventSenseStart:
        notification_message(app->notification, &sequence_blink_stop);
        notification_message(app->notification, &sequence_blink_start_yellow);
        break;
    case LfRfidApp::EventType::ReadEventSenseCardStart:
        notification_message(app->notification, &sequence_blink_stop);
        notification_message(app->notification, &sequence_blink_start_green);
        break;
    case LfRfidApp::EventType::ReadEventSenseEnd:
    case LfRfidApp::EventType::ReadEventSenseCardEnd:
        notification_message(app->notification, &sequence_blink_stop);
        notification_message(app->notification, &sequence_blink_start_cyan);
        break;
    case LfRfidApp::EventType::ReadEventDone:
        app->protocol_id = event->payload.signed_int;
        DOLPHIN_DEED(DolphinDeedRfidReadSuccess);
        notification_message(app->notification, &sequence_success);
        string_reset(app->file_name);
        app->scene_controller.switch_to_next_scene(LfRfidApp::SceneType::ReadSuccess);
        break;
    case LfRfidApp::EventType::ReadEventStartPSK:
        popup->set_header("Reading\nLF RFID\nPSK", 89, 30, AlignCenter, AlignTop);
        break;
    case LfRfidApp::EventType::ReadEventStartASK:
        popup->set_header("Reading\nLF RFID\nASK", 89, 30, AlignCenter, AlignTop);
        break;
    default:
        consumed = false;
        break;
    }

    return consumed;
}

void LfRfidAppSceneRead::on_exit(LfRfidApp* app) {
    notification_message(app->notification, &sequence_blink_stop);
    app->view_controller.get<PopupVM>()->clean();
    lfrfid_worker_stop(app->lfworker);
    lfrfid_worker_stop_thread(app->lfworker);
}
