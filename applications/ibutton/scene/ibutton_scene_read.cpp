#include "ibutton_scene_read.h"
#include "../ibutton_app.h"
#include "../ibutton_view_manager.h"
#include "../ibutton_event.h"

void iButtonSceneRead::on_enter(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    Popup* popup = view_manager->get_popup();

    popup_set_header(popup, "iButton", 95, 26, AlignCenter, AlignBottom);
    popup_set_text(popup, "waiting\nfor key ...", 95, 30, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 5, &I_DolphinWait_61x59);

    view_manager->switch_to(iButtonAppViewManager::Type::iButtonAppViewPopup);
    app->get_key()->set_name("");

    app->get_key_worker()->start_read();
}

bool iButtonSceneRead::on_event(iButtonApp* app, iButtonEvent* event) {
    bool consumed = false;

    if(event->type == iButtonEvent::Type::EventTypeTick) {
        consumed = true;
        app->notify_red_blink();

        switch(app->get_key_worker()->read(app->get_key())) {
        case KeyReader::Error::EMPTY:
            break;
        case KeyReader::Error::OK:
            app->switch_to_next_scene(iButtonApp::Scene::SceneReadSuccess);
            break;
        case KeyReader::Error::CRC_ERROR:
            app->switch_to_next_scene(iButtonApp::Scene::SceneReadCRCError);
            break;
        case KeyReader::Error::NOT_ARE_KEY:
            app->switch_to_next_scene(iButtonApp::Scene::SceneReadNotKeyError);
            break;
        }
    }

    return consumed;
}

void iButtonSceneRead::on_exit(iButtonApp* app) {
    app->get_key_worker()->stop_read();

    Popup* popup = app->get_view_manager()->get_popup();

    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignBottom);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 0, NULL);
}