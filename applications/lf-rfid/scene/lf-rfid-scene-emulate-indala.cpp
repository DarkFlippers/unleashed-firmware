#include "lf-rfid-scene-emulate-indala.h"

#include "../lf-rfid-app.h"
#include "../lf-rfid-view-manager.h"
#include "../lf-rfid-event.h"
#include "../helpers/key-info.h"

void LfrfidSceneEmulateIndala::on_enter(LfrfidApp* app) {
    LfrfidAppViewManager* view_manager = app->get_view_manager();

    Popup* popup = view_manager->get_popup();
    popup_set_header(popup, "LF-RFID", 64, 16, AlignCenter, AlignBottom);
    app->set_text_store("Indala 40134 emulation");
    popup_set_text(popup, app->get_text_store(), 64, 22, AlignCenter, AlignTop);

    view_manager->switch_to(LfrfidAppViewManager::ViewType::Popup);
    app->get_emulator()->start(LfrfidKeyType::KeyI40134, data, 3);
}

bool LfrfidSceneEmulateIndala::on_event(LfrfidApp* app, LfrfidEvent* event) {
    bool consumed = false;

    return consumed;
}

void LfrfidSceneEmulateIndala::on_exit(LfrfidApp* app) {
    LfrfidAppViewManager* view_manager = app->get_view_manager();

    Popup* popup = view_manager->get_popup();
    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignBottom);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);

    app->get_emulator()->stop();
}