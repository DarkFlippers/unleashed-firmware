#include "ibutton_scene_info.h"
#include "../ibutton_app.h"

void iButtonSceneInfo::on_enter(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    Widget* widget = view_manager->get_widget();
    iButtonKey* key = app->get_key();
    const uint8_t* key_data = ibutton_key_get_data_p(key);

    app->set_text_store("%s", ibutton_key_get_name_p(key));
    widget_add_text_box_element(
        widget, 0, 0, 128, 28, AlignCenter, AlignCenter, app->get_text_store());

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
            widget, 64, 51, AlignCenter, AlignBottom, FontSecondary, "Dallas");
        break;
    case iButtonKeyMetakom:
        app->set_text_store(
            "%02X %02X %02X %02X", key_data[0], key_data[1], key_data[2], key_data[3]);
        widget_add_string_element(
            widget, 64, 51, AlignCenter, AlignBottom, FontSecondary, "Metakom");
        break;
    case iButtonKeyCyfral:
        app->set_text_store("%02X %02X", key_data[0], key_data[1]);
        widget_add_string_element(
            widget, 64, 51, AlignCenter, AlignBottom, FontSecondary, "Cyfral");
        break;
    }
    widget_add_string_element(
        widget, 64, 35, AlignCenter, AlignBottom, FontPrimary, app->get_text_store());

    view_manager->switch_to(iButtonAppViewManager::Type::iButtonAppViewWidget);
}

bool iButtonSceneInfo::on_event(iButtonApp* app, iButtonEvent* event) {
    bool consumed = false;
    return consumed;
}

void iButtonSceneInfo::on_exit(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    Widget* widget = view_manager->get_widget();

    app->set_text_store("");

    widget_reset(widget);
}
