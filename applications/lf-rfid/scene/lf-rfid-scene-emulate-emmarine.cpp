#include "lf-rfid-scene-emulate-emmarine.h"

#include "../lf-rfid-app.h"
#include "../lf-rfid-view-manager.h"
#include "../lf-rfid-event.h"
#include "../helpers/key-info.h"

void LfrfidSceneEmulateEMMarine::on_enter(LfrfidApp* app) {
    LfrfidAppViewManager* view_manager = app->get_view_manager();

    Popup* popup = view_manager->get_popup();
    popup_set_header(popup, "LF-RFID", 64, 16, AlignCenter, AlignBottom);
    app->set_text_store("EM emulation");
    popup_set_text(popup, app->get_text_store(), 64, 22, AlignCenter, AlignTop);

    view_manager->switch_to(LfrfidAppViewManager::ViewType::Popup);
    app->get_emulator()->start(LfrfidKeyType::KeyEM4100, data, 5);
}

bool LfrfidSceneEmulateEMMarine::on_event(LfrfidApp* app, LfrfidEvent* event) {
    bool consumed = false;

    return consumed;
}

void LfrfidSceneEmulateEMMarine::on_exit(LfrfidApp* app) {
    LfrfidAppViewManager* view_manager = app->get_view_manager();

    Popup* popup = view_manager->get_popup();
    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignBottom);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);

    app->get_emulator()->stop();
}