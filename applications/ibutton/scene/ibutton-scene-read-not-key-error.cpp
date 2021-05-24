#include "ibutton-scene-read-not-key-error.h"
#include "../ibutton-app.h"
#include "../ibutton-view-manager.h"
#include "../ibutton-event.h"
#include <callback-connector.h>

void iButtonSceneReadNotKeyError::on_enter(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    DialogEx* dialog_ex = view_manager->get_dialog_ex();
    auto callback = cbc::obtain_connector(this, &iButtonSceneReadNotKeyError::dialog_ex_callback);

    iButtonKey* key = app->get_key();
    uint8_t* key_data = key->get_data();

    app->set_text_store(
        "THIS IS NOT A KEY\n%02X %02X %02X %02X %02X %02X %02X %02X",
        key_data[0],
        key_data[1],
        key_data[2],
        key_data[3],
        key_data[4],
        key_data[5],
        key_data[6],
        key_data[7],
        maxim_crc8(key_data, 7));

    dialog_ex_set_header(dialog_ex, "ERROR:", 64, 10, AlignCenter, AlignCenter);
    dialog_ex_set_text(dialog_ex, app->get_text_store(), 64, 19, AlignCenter, AlignTop);
    dialog_ex_set_left_button_text(dialog_ex, "Retry");
    dialog_ex_set_right_button_text(dialog_ex, "More");
    dialog_ex_set_result_callback(dialog_ex, callback);
    dialog_ex_set_context(dialog_ex, app);

    view_manager->switch_to(iButtonAppViewManager::Type::iButtonAppViewDialogEx);
    app->notify_error();
    app->notify_red_on();
}

bool iButtonSceneReadNotKeyError::on_event(iButtonApp* app, iButtonEvent* event) {
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

void iButtonSceneReadNotKeyError::on_exit(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    DialogEx* dialog_ex = view_manager->get_dialog_ex();

    app->set_text_store("");

    dialog_ex_set_header(dialog_ex, NULL, 0, 0, AlignCenter, AlignCenter);
    dialog_ex_set_text(dialog_ex, NULL, 0, 0, AlignCenter, AlignTop);
    dialog_ex_set_left_button_text(dialog_ex, NULL);
    dialog_ex_set_result_callback(dialog_ex, NULL);
    dialog_ex_set_context(dialog_ex, NULL);

    app->notify_red_off();
}

void iButtonSceneReadNotKeyError::dialog_ex_callback(DialogExResult result, void* context) {
    iButtonApp* app = static_cast<iButtonApp*>(context);
    iButtonEvent event;

    event.type = iButtonEvent::Type::EventTypeDialogResult;
    event.payload.dialog_result = result;

    app->get_view_manager()->send_event(&event);
}