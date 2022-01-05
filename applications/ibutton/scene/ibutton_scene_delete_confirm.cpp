#include "ibutton_scene_delete_confirm.h"
#include "../ibutton_app.h"
#include "../ibutton_view_manager.h"
#include "../ibutton_event.h"
#include <callback-connector.h>

void iButtonSceneDeleteConfirm::on_enter(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    Widget* widget = view_manager->get_widget();
    auto callback = cbc::obtain_connector(this, &iButtonSceneDeleteConfirm::widget_callback);

    iButtonKey* key = app->get_key();
    uint8_t* key_data = key->get_data();

    app->set_text_store("\e#Delete %s?\e#", key->get_name());
    widget_add_text_box_element(
        widget, 0, 0, 128, 23, AlignCenter, AlignCenter, app->get_text_store());
    widget_add_button_element(widget, GuiButtonTypeLeft, "Back", callback, app);
    widget_add_button_element(widget, GuiButtonTypeRight, "Delete", callback, app);

    switch(key->get_key_type()) {
    case iButtonKeyType::KeyDallas:
        app->set_text_store(
            "%02X %02X %02X %02X %02X %02X %02X %02X\nDallas",
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
        app->set_text_store("%02X %02X\nCyfral", key_data[0], key_data[1]);
        break;
    case iButtonKeyType::KeyMetakom:
        app->set_text_store(
            "%02X %02X %02X %02X\nMetakom", key_data[0], key_data[1], key_data[2], key_data[3]);
        break;
    }
    widget_add_string_multiline_element(
        widget, 64, 23, AlignCenter, AlignTop, FontSecondary, app->get_text_store());

    view_manager->switch_to(iButtonAppViewManager::Type::iButtonAppViewWidget);
}

bool iButtonSceneDeleteConfirm::on_event(iButtonApp* app, iButtonEvent* event) {
    bool consumed = false;

    if(event->type == iButtonEvent::Type::EventTypeWidgetButtonResult) {
        if(event->payload.widget_button_result == GuiButtonTypeRight) {
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
    Widget* widget = view_manager->get_widget();

    app->set_text_store("");

    widget_clear(widget);
}

void iButtonSceneDeleteConfirm::widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    iButtonApp* app = static_cast<iButtonApp*>(context);
    iButtonEvent event;

    if(type == InputTypeShort) {
        event.type = iButtonEvent::Type::EventTypeWidgetButtonResult;
        event.payload.widget_button_result = result;
    }

    app->get_view_manager()->send_event(&event);
}