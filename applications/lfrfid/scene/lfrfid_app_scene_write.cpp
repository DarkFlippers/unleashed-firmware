#include "lfrfid_app_scene_write.h"

void LfRfidAppSceneWrite::on_enter(LfRfidApp* app, bool /* need_restore */) {
    card_not_supported = false;
    string_init(data_string);

    const uint8_t* data = app->worker.key.get_data();

    for(uint8_t i = 0; i < app->worker.key.get_type_data_count(); i++) {
        string_cat_printf(data_string, "%02X", data[i]);
    }

    auto popup = app->view_controller.get<PopupVM>();

    popup->set_header("Writing", 89, 30, AlignCenter, AlignTop);
    if(strlen(app->worker.key.get_name())) {
        popup->set_text(app->worker.key.get_name(), 89, 43, AlignCenter, AlignTop);
    } else {
        popup->set_text(string_get_cstr(data_string), 89, 43, AlignCenter, AlignTop);
    }
    popup->set_icon(0, 3, &I_RFIDDolphinSend_97x61);

    app->view_controller.switch_to<PopupVM>();
    app->worker.start_write();
}

bool LfRfidAppSceneWrite::on_event(LfRfidApp* app, LfRfidApp::Event* event) {
    bool consumed = false;

    if(event->type == LfRfidApp::EventType::Tick) {
        RfidWorker::WriteResult result = app->worker.write();

        switch(result) {
        case RfidWorker::WriteResult::Nothing:
            notification_message(app->notification, &sequence_blink_magenta_10);
            break;
        case RfidWorker::WriteResult::Ok:
            notification_message(app->notification, &sequence_success);
            app->scene_controller.switch_to_next_scene(LfRfidApp::SceneType::WriteSuccess);
            break;
        case RfidWorker::WriteResult::NotWritable:
            if(!card_not_supported) {
                auto popup = app->view_controller.get<PopupVM>();
                popup->set_icon(72, 17, &I_DolphinCommon_56x48);
                popup->set_header("Still trying to write...", 64, 3, AlignCenter, AlignTop);
                popup->set_text(
                    "Make sure this\ncard is writable\nand not\nprotected.",
                    3,
                    17,
                    AlignLeft,
                    AlignTop);
                card_not_supported = true;
            }
            notification_message(app->notification, &sequence_blink_yellow_10);
            break;
        }
    }

    return consumed;
}

void LfRfidAppSceneWrite::on_exit(LfRfidApp* app) {
    app->view_controller.get<PopupVM>()->clean();
    app->worker.stop_write();
    string_clear(data_string);
}
