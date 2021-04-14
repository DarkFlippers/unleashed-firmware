#include "ibutton-scene-cli-emulate.h"
#include "../ibutton-app.h"
#include "../ibutton-view-manager.h"
#include "../ibutton-event.h"
#include "../ibutton-key.h"
#include <callback-connector.h>

void iButtonSceneCliEmulate::on_enter(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    Popup* popup = view_manager->get_popup();
    iButtonKey* key = app->get_key();
    uint8_t* key_data = key->get_data();
    const char* key_name = key->get_name();
    uint8_t line_count = 2;
    timeout = 50; // 5s timeout

    // check that stored key has name
    if(strcmp(key_name, "") != 0) {
        app->set_text_store("emulating\n%s", key_name);
        line_count = 2;
    } else {
        // if not, show key data
        switch(key->get_key_type()) {
        case iButtonKeyType::KeyDallas:
            app->set_text_store(
                "emulating\n%02X %02X %02X %02X\n%02X %02X %02X %02X",
                key_data[0],
                key_data[1],
                key_data[2],
                key_data[3],
                key_data[4],
                key_data[5],
                key_data[6],
                key_data[7]);
            line_count = 3;
            break;
        case iButtonKeyType::KeyCyfral:
            app->set_text_store("emulating\n%02X %02X", key_data[0], key_data[1]);
            line_count = 2;
            break;
        case iButtonKeyType::KeyMetakom:
            app->set_text_store(
                "emulating\n%02X %02X %02X %02X",
                key_data[0],
                key_data[1],
                key_data[2],
                key_data[3]);
            line_count = 2;
            break;
        }
    }

    switch(line_count) {
    case 3:
        popup_set_header(popup, "iButton", 92, 18, AlignCenter, AlignBottom);
        popup_set_text(popup, app->get_text_store(), 92, 22, AlignCenter, AlignTop);
        break;

    default:
        popup_set_header(popup, "iButton", 92, 24, AlignCenter, AlignBottom);
        popup_set_text(popup, app->get_text_store(), 92, 28, AlignCenter, AlignTop);
        break;
    }

    popup_set_icon(popup, 10, 10, I_iButtonKey_49x44);

    view_manager->switch_to(iButtonAppViewManager::Type::iButtonAppViewPopup);
    app->get_key_worker()->start_emulate(app->get_key());
}

bool iButtonSceneCliEmulate::on_event(iButtonApp* app, iButtonEvent* event) {
    bool consumed = false;

    if(event->type == iButtonEvent::Type::EventTypeTick) {
        consumed = true;
        if(!timeout--) {
            app->cli_send_event(iButtonApp::CliEvent::CliTimeout);
            app->search_and_switch_to_previous_scene({iButtonApp::Scene::SceneStart});
        } else {
            if(app->get_key_worker()->emulated()) {
                app->notify_yellow_blink();
            } else {
                app->notify_red_blink();
            }
        }
    } else if(event->type == iButtonEvent::Type::EventTypeBack) {
        consumed = false;
        app->cli_send_event(iButtonApp::CliEvent::CliInterrupt);
    }

    return consumed;
}

void iButtonSceneCliEmulate::on_exit(iButtonApp* app) {
    app->get_key_worker()->stop_emulate();

    Popup* popup = app->get_view_manager()->get_popup();

    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignBottom);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, -1, -1, I_DolphinWait_61x59);
}