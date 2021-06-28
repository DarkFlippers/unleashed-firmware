#include "ibutton-scene-read-success.h"
#include "../ibutton-app.h"
#include "../ibutton-view-manager.h"
#include "../ibutton-event.h"
#include <callback-connector.h>

void iButtonSceneReadSuccess::on_enter(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    DialogEx* dialog_ex = view_manager->get_dialog_ex();
    auto callback = cbc::obtain_connector(this, &iButtonSceneReadSuccess::dialog_ex_callback);

    iButtonKey* key = app->get_key();
    uint8_t* key_data = key->get_data();

    switch(key->get_key_type()) {
    case iButtonKeyType::KeyDallas:
        app->set_text_store(
            "Dallas\n%02X %02X %02X %02X\n%02X %02X %02X %02X",
            key_data[0],
            key_data[1],
            key_data[2],
            key_data[3],
            key_data[4],
            key_data[5],
            key_data[6],
            key_data[7]);
        break;
    case iButtonKeyType::KeyCyfral:
        app->set_text_store("Cyfral\n%02X %02X", key_data[0], key_data[1]);
        break;
    case iButtonKeyType::KeyMetakom:
        app->set_text_store(
            "Metakom\n%02X %02X %02X %02X", key_data[0], key_data[1], key_data[2], key_data[3]);
        break;
    }

    dialog_ex_set_text(dialog_ex, app->get_text_store(), 95, 30, AlignCenter, AlignCenter);
    dialog_ex_set_left_button_text(dialog_ex, "Retry");
    dialog_ex_set_right_button_text(dialog_ex, "More");
    dialog_ex_set_icon(dialog_ex, 0, 1, I_DolphinExcited_64x63);
    dialog_ex_set_result_callback(dialog_ex, callback);
    dialog_ex_set_context(dialog_ex, app);

    view_manager->switch_to(iButtonAppViewManager::Type::iButtonAppViewDialogEx);

    app->notify_success();
    app->notify_green_on();
}

bool iButtonSceneReadSuccess::on_event(iButtonApp* app, iButtonEvent* event) {
    bool consumed = false;

    if(event->type == iButtonEvent::Type::EventTypeDialogResult) {
        if(event->payload.dialog_result == DialogExResultRight) {
            app->switch_to_next_scene(iButtonApp::Scene::SceneReadedKeyMenu);
        } else {
            app->switch_to_previous_scene();
        }

        consumed = true;
    }

    return consumed;
}

void iButtonSceneReadSuccess::on_exit(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    DialogEx* dialog_ex = view_manager->get_dialog_ex();

    app->set_text_store("");

    dialog_ex_set_text(dialog_ex, NULL, 0, 0, AlignCenter, AlignTop);
    dialog_ex_set_left_button_text(dialog_ex, NULL);
    dialog_ex_set_right_button_text(dialog_ex, NULL);
    dialog_ex_set_result_callback(dialog_ex, NULL);
    dialog_ex_set_context(dialog_ex, NULL);
    dialog_ex_set_icon(dialog_ex, 0, 0, I_Empty_1x1);

    app->notify_green_off();
}

void iButtonSceneReadSuccess::dialog_ex_callback(DialogExResult result, void* context) {
    iButtonApp* app = static_cast<iButtonApp*>(context);
    iButtonEvent event;

    event.type = iButtonEvent::Type::EventTypeDialogResult;
    event.payload.dialog_result = result;

    app->get_view_manager()->send_event(&event);
}