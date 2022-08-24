#include "lfrfid_app_scene_write.h"

static void lfrfid_write_callback(LFRFIDWorkerWriteResult result, void* ctx) {
    LfRfidApp* app = static_cast<LfRfidApp*>(ctx);
    LfRfidApp::Event event;

    switch(result) {
    case LFRFIDWorkerWriteOK:
        event.type = LfRfidApp::EventType::WriteEventOK;
        break;
    case LFRFIDWorkerWriteProtocolCannotBeWritten:
        event.type = LfRfidApp::EventType::WriteEventProtocolCannotBeWritten;
        break;
    case LFRFIDWorkerWriteFobCannotBeWritten:
        event.type = LfRfidApp::EventType::WriteEventFobCannotBeWritten;
        break;
    case LFRFIDWorkerWriteTooLongToWrite:
        event.type = LfRfidApp::EventType::WriteEventTooLongToWrite;
        break;
    }

    app->view_controller.send_event(&event);
}

void LfRfidAppSceneWrite::on_enter(LfRfidApp* app, bool /* need_restore */) {
    auto popup = app->view_controller.get<PopupVM>();

    popup->set_header("Writing", 89, 30, AlignCenter, AlignTop);
    if(string_size(app->file_name)) {
        popup->set_text(string_get_cstr(app->file_name), 89, 43, AlignCenter, AlignTop);
    } else {
        popup->set_text(
            protocol_dict_get_name(app->dict, app->protocol_id), 89, 43, AlignCenter, AlignTop);
    }
    popup->set_icon(0, 3, &I_RFIDDolphinSend_97x61);

    app->view_controller.switch_to<PopupVM>();

    size_t size = protocol_dict_get_data_size(app->dict, app->protocol_id);
    app->old_key_data = (uint8_t*)malloc(size);
    protocol_dict_get_data(app->dict, app->protocol_id, app->old_key_data, size);

    lfrfid_worker_start_thread(app->lfworker);
    lfrfid_worker_write_start(
        app->lfworker, (LFRFIDProtocol)app->protocol_id, lfrfid_write_callback, app);
    notification_message(app->notification, &sequence_blink_start_magenta);
}

bool LfRfidAppSceneWrite::on_event(LfRfidApp* app, LfRfidApp::Event* event) {
    bool consumed = true;
    auto popup = app->view_controller.get<PopupVM>();

    switch(event->type) {
    case LfRfidApp::EventType::WriteEventOK:
        notification_message(app->notification, &sequence_success);
        app->scene_controller.switch_to_next_scene(LfRfidApp::SceneType::WriteSuccess);
        break;
    case LfRfidApp::EventType::WriteEventProtocolCannotBeWritten:
        popup->set_icon(72, 17, &I_DolphinCommon_56x48);
        popup->set_header("Error", 64, 3, AlignCenter, AlignTop);
        popup->set_text("This protocol\ncannot be written", 3, 17, AlignLeft, AlignTop);
        notification_message(app->notification, &sequence_blink_start_red);
        break;
    case LfRfidApp::EventType::WriteEventFobCannotBeWritten:
    case LfRfidApp::EventType::WriteEventTooLongToWrite:
        popup->set_icon(72, 17, &I_DolphinCommon_56x48);
        popup->set_header("Still trying to write...", 64, 3, AlignCenter, AlignTop);
        popup->set_text(
            "Make sure this\ncard is writable\nand not\nprotected.", 3, 17, AlignLeft, AlignTop);
        notification_message(app->notification, &sequence_blink_start_yellow);
        break;
    default:
        consumed = false;
    }

    return consumed;
}

void LfRfidAppSceneWrite::on_exit(LfRfidApp* app) {
    notification_message(app->notification, &sequence_blink_stop);
    app->view_controller.get<PopupVM>()->clean();
    lfrfid_worker_stop(app->lfworker);
    lfrfid_worker_stop_thread(app->lfworker);

    size_t size = protocol_dict_get_data_size(app->dict, app->protocol_id);
    protocol_dict_set_data(app->dict, app->protocol_id, app->old_key_data, size);
    free(app->old_key_data);
}
