#include "ibutton_scene_delete_confirm.h"
#include "../ibutton_app.h"

static void widget_callback(GuiButtonType result, InputType type, void* context) {
    furi_assert(context);
    iButtonApp* app = static_cast<iButtonApp*>(context);
    iButtonEvent event;

    if(type == InputTypeShort) {
        event.type = iButtonEvent::Type::EventTypeWidgetButtonResult;
        event.payload.widget_button_result = result;
        app->get_view_manager()->send_event(&event);
    }
}

void iButtonSceneDeleteConfirm::on_enter(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    Widget* widget = view_manager->get_widget();
    iButtonKey* key = app->get_key();
    const uint8_t* key_data = ibutton_key_get_data_p(key);

    app->set_text_store("\e#Delete %s?\e#", ibutton_key_get_name_p(key));
    widget_add_text_box_element(
        widget, 0, 0, 128, 27, AlignCenter, AlignCenter, app->get_text_store());
    widget_add_button_element(widget, GuiButtonTypeLeft, "Cancel", widget_callback, app);
    widget_add_button_element(widget, GuiButtonTypeRight, "Delete", widget_callback, app);

    switch(ibutton_key_get_type(key)) {
    case iButtonKeyDS1990:
        app->set_text_store(
            "%02X %02X %02X %02X %02X %02X %02X %02X",
            key_data[0],
            key_data[1],
            key_data[2],
            key_data[3],
            key_data[4],
            key_data[5],
            key_data[6],
            key_data[7]);
        widget_add_string_element(
            widget, 64, 45, AlignCenter, AlignBottom, FontSecondary, "Dallas");
        break;
    case iButtonKeyCyfral:
        app->set_text_store("%02X %02X", key_data[0], key_data[1]);
        widget_add_string_element(
            widget, 64, 45, AlignCenter, AlignBottom, FontSecondary, "Cyfral");
        break;
    case iButtonKeyMetakom:
        app->set_text_store(
            "%02X %02X %02X %02X", key_data[0], key_data[1], key_data[2], key_data[3]);
        widget_add_string_element(
            widget, 64, 45, AlignCenter, AlignBottom, FontSecondary, "Metakom");
        break;
    }
    widget_add_string_element(
        widget, 64, 33, AlignCenter, AlignBottom, FontSecondary, app->get_text_store());

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

    widget_reset(widget);
}
