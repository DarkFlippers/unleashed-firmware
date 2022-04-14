#include "ibutton_scene_read_success.h"
#include "../ibutton_app.h"
#include <dolphin/dolphin.h>

static void dialog_ex_callback(DialogExResult result, void* context) {
    furi_assert(context);
    iButtonApp* app = static_cast<iButtonApp*>(context);
    iButtonEvent event;

    event.type = iButtonEvent::Type::EventTypeDialogResult;
    event.payload.dialog_result = result;

    app->get_view_manager()->send_event(&event);
}

void iButtonSceneReadSuccess::on_enter(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    DialogEx* dialog_ex = view_manager->get_dialog_ex();
    iButtonKey* key = app->get_key();
    const uint8_t* key_data = ibutton_key_get_data_p(key);

    switch(ibutton_key_get_type(key)) {
    case iButtonKeyDS1990:
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
    case iButtonKeyCyfral:
        app->set_text_store("Cyfral\n%02X %02X", key_data[0], key_data[1]);
        break;
    case iButtonKeyMetakom:
        app->set_text_store(
            "Metakom\n%02X %02X %02X %02X", key_data[0], key_data[1], key_data[2], key_data[3]);
        break;
    }

    dialog_ex_set_text(dialog_ex, app->get_text_store(), 95, 30, AlignCenter, AlignCenter);
    dialog_ex_set_left_button_text(dialog_ex, "Retry");
    dialog_ex_set_right_button_text(dialog_ex, "More");
    dialog_ex_set_icon(dialog_ex, 0, 1, &I_DolphinReadingSuccess_59x63);
    dialog_ex_set_result_callback(dialog_ex, dialog_ex_callback);
    dialog_ex_set_context(dialog_ex, app);

    view_manager->switch_to(iButtonAppViewManager::Type::iButtonAppViewDialogEx);
}

bool iButtonSceneReadSuccess::on_event(iButtonApp* app, iButtonEvent* event) {
    bool consumed = false;

    if(event->type == iButtonEvent::Type::EventTypeDialogResult) {
        if(event->payload.dialog_result == DialogExResultRight) {
            app->switch_to_next_scene(iButtonApp::Scene::SceneReadKeyMenu);
        } else {
            app->switch_to_next_scene(iButtonApp::Scene::SceneRetryConfirm);
        }
        consumed = true;
    } else if(event->type == iButtonEvent::Type::EventTypeBack) {
        app->switch_to_next_scene(iButtonApp::Scene::SceneExitConfirm);
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
    dialog_ex_set_icon(dialog_ex, 0, 0, NULL);

    app->notify_green_off();
}
