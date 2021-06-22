#include "ibutton-scene-delete-confirm.h"
#include "../ibutton-app.h"
#include "../ibutton-view-manager.h"
#include "../ibutton-event.h"
#include <callback-connector.h>

void iButtonSceneDeleteConfirm::on_enter(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    DialogEx* dialog_ex = view_manager->get_dialog_ex();
    auto callback = cbc::obtain_connector(this, &iButtonSceneDeleteConfirm::dialog_ex_callback);

    iButtonKey* key = app->get_key();
    uint8_t* key_data = key->get_data();

    switch(key->get_key_type()) {
    case iButtonKeyType::KeyDallas:
        app->set_text_store(
            "Delete %s?\nID: %02X %02X %02X %02X %02X %02X %02X %02X\nType: Dallas",
            key->get_name(),
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
        app->set_text_store(
            "Delete %s?\nID: %02X %02X\nType: Cyfral", key->get_name(), key_data[0], key_data[1]);
        break;
    case iButtonKeyType::KeyMetakom:
        app->set_text_store(
            "Delete %s?\nID: %02X %02X %02X %02X\nType: Metakom",
            key->get_name(),
            key_data[0],
            key_data[1],
            key_data[2],
            key_data[3]);
        break;
    }

    dialog_ex_set_text(dialog_ex, app->get_text_store(), 64, 20, AlignCenter, AlignCenter);
    dialog_ex_set_left_button_text(dialog_ex, "Back");
    dialog_ex_set_right_button_text(dialog_ex, "Delete");
    dialog_ex_set_result_callback(dialog_ex, callback);
    dialog_ex_set_context(dialog_ex, app);

    view_manager->switch_to(iButtonAppViewManager::Type::iButtonAppViewDialogEx);
}

bool iButtonSceneDeleteConfirm::on_event(iButtonApp* app, iButtonEvent* event) {
    bool consumed = false;

    if(event->type == iButtonEvent::Type::EventTypeDialogResult) {
        if(event->payload.dialog_result == DialogExResultRight) {
            if(app->delete_key()) {
                app->switch_to_next_scene(iButtonApp::Scene::SceneDeleteSuccess);
            }
        } else {
            app->switch_to_previous_scene();
        }

        consumed = true;
    }

    return consumed;
}

void iButtonSceneDeleteConfirm::on_exit(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    DialogEx* dialog_ex = view_manager->get_dialog_ex();

    app->set_text_store("");

    dialog_ex_set_text(dialog_ex, NULL, 0, 0, AlignCenter, AlignTop);
    dialog_ex_set_left_button_text(dialog_ex, NULL);
    dialog_ex_set_right_button_text(dialog_ex, NULL);
    dialog_ex_set_result_callback(dialog_ex, NULL);
    dialog_ex_set_context(dialog_ex, NULL);
}

void iButtonSceneDeleteConfirm::dialog_ex_callback(DialogExResult result, void* context) {
    iButtonApp* app = static_cast<iButtonApp*>(context);
    iButtonEvent event;

    event.type = iButtonEvent::Type::EventTypeDialogResult;
    event.payload.dialog_result = result;

    app->get_view_manager()->send_event(&event);
}