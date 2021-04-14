#include "ibutton-scene-cli-read.h"
#include "../ibutton-app.h"
#include "../ibutton-view-manager.h"
#include "../ibutton-event.h"

void iButtonSceneCliRead::on_enter(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    Popup* popup = view_manager->get_popup();
    timeout = 50; // 5s timeout

    popup_set_header(popup, "iButton", 95, 26, AlignCenter, AlignBottom);
    popup_set_text(popup, "waiting\nfor key ...", 95, 30, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 5, I_DolphinWait_61x59);

    view_manager->switch_to(iButtonAppViewManager::Type::iButtonAppViewPopup);
    app->get_key()->set_name("");

    app->get_key_worker()->start_read();
}

bool iButtonSceneCliRead::on_event(iButtonApp* app, iButtonEvent* event) {
    bool consumed = false;

    if(event->type == iButtonEvent::Type::EventTypeTick) {
        consumed = true;
        app->notify_red_blink();
        if(!timeout--) {
            app->cli_send_event(iButtonApp::CliEvent::CliTimeout);
            app->search_and_switch_to_previous_scene({iButtonApp::Scene::SceneStart});
            return consumed;
        } else {
            switch(app->get_key_worker()->read(app->get_key())) {
            case KeyReader::Error::EMPTY:
                break;
            case KeyReader::Error::OK:
                app->cli_send_event(iButtonApp::CliEvent::CliReadSuccess);
                app->search_and_switch_to_previous_scene({iButtonApp::Scene::SceneStart});
                break;
            case KeyReader::Error::CRC_ERROR:
                app->cli_send_event(iButtonApp::CliEvent::CliReadCRCError);
                app->search_and_switch_to_previous_scene({iButtonApp::Scene::SceneStart});
                break;
            case KeyReader::Error::NOT_ARE_KEY:
                app->cli_send_event(iButtonApp::CliEvent::CliReadNotKeyError);
                app->search_and_switch_to_previous_scene({iButtonApp::Scene::SceneStart});
                break;
            }
        }
    } else if(event->type == iButtonEvent::Type::EventTypeBack) {
        consumed = false;
        app->cli_send_event(iButtonApp::CliEvent::CliInterrupt);
    }

    return consumed;
}

void iButtonSceneCliRead::on_exit(iButtonApp* app) {
    app->get_key_worker()->stop_read();

    Popup* popup = app->get_view_manager()->get_popup();

    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignBottom);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, -1, -1, I_DolphinWait_61x59);
}