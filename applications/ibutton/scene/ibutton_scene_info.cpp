#include "ibutton_scene_info.h"
#include "../ibutton_app.h"
#include "../ibutton_view_manager.h"
#include "../ibutton_event.h"
#include <callback-connector.h>

void iButtonSceneInfo::on_enter(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    Widget* widget = view_manager->get_widget();
    auto callback = cbc::obtain_connector(this, &iButtonSceneInfo::widget_callback);

    iButtonKey* key = app->get_key();
    uint8_t* key_data = key->get_data();

    app->set_text_store("%s", key->get_name());
    widget_add_text_box_element(
        widget, 0, 0, 128, 27, AlignCenter, AlignCenter, app->get_text_store());
    widget_add_button_element(widget, GuiButtonTypeLeft, "Back", callback, app);

    switch(key->get_key_type()) {
    case iButtonKeyType::KeyDallas:
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
    case iButtonKeyType::KeyMetakom:
        app->set_text_store(
            "%02X %02X %02X %02X", key_data[0], key_data[1], key_data[2], key_data[3]);
        widget_add_string_element(
            widget, 64, 45, AlignCenter, AlignBottom, FontSecondary, "Metakom");
        break;
    case iButtonKeyType::KeyCyfral:
        app->set_text_store("%02X %02X", key_data[0], key_data[1]);
        widget_add_string_element(
            widget, 64, 45, AlignCenter, AlignBottom, FontSecondary, "Cyfral");
        break;
    }
    widget_add_string_element(
        widget, 64, 33, AlignCenter, AlignBottom, FontPrimary, app->get_text_store());

    view_manager->switch_to(iButtonAppViewManager::Type::iButtonAppViewWidget);
}

bool iButtonSceneInfo::on_event(iButtonApp* app, iButtonEvent* event) {
    bool consumed = false;

    if(event->type == iButtonEvent::Type::EventTypeWidgetButtonResult) {
        if(event->payload.widget_button_result == GuiButtonTypeLeft) {
            app->switch_to_previous_scene();
            consumed = true;
        }
    }

    return consumed;
}

void iButtonSceneInfo::on_exit(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    Widget* widget = view_manager->get_widget();

    app->set_text_store("");

    widget_reset(widget);
}

void iButtonSceneInfo::widget_callback(GuiButtonType result, InputType type, void* context) {
    iButtonApp* app = static_cast<iButtonApp*>(context);
    iButtonEvent event;

    if(type == InputTypeShort) {
        event.type = iButtonEvent::Type::EventTypeWidgetButtonResult;
        event.payload.widget_button_result = result;
    }

    app->get_view_manager()->send_event(&event);
}
