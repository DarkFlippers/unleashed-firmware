#include "lfrfid_app_scene_emulate.h"
#include <dolphin/dolphin.h>

void LfRfidAppSceneEmulate::on_enter(LfRfidApp* app, bool need_restore) {
    string_init(data_string);

    DOLPHIN_DEED(DolphinDeedRfidEmulate);
    const uint8_t* data = app->worker.key.get_data();

    for(uint8_t i = 0; i < app->worker.key.get_type_data_count(); i++) {
        string_cat_printf(data_string, "%02X", data[i]);
    }

    auto popup = app->view_controller.get<PopupVM>();

    popup->set_header("Emulating", 89, 30, AlignCenter, AlignTop);
    if(strlen(app->worker.key.get_name())) {
        popup->set_text(app->worker.key.get_name(), 89, 43, AlignCenter, AlignTop);
    } else {
        popup->set_text(string_get_cstr(data_string), 89, 43, AlignCenter, AlignTop);
    }
    popup->set_icon(0, 3, &I_RFIDDolphinSend_97x61);

    app->view_controller.switch_to<PopupVM>();
    app->worker.start_emulate();
}

bool LfRfidAppSceneEmulate::on_event(LfRfidApp* app, LfRfidApp::Event* event) {
    bool consumed = false;

    if(event->type == LfRfidApp::EventType::Tick) {
        notification_message(app->notification, &sequence_blink_magenta_10);
    }

    return consumed;
}

void LfRfidAppSceneEmulate::on_exit(LfRfidApp* app) {
    app->view_controller.get<PopupVM>()->clean();
    app->worker.stop_emulate();
    string_clear(data_string);
}
