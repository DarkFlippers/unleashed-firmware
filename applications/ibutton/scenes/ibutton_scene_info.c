#include "../ibutton_i.h"
#include <toolbox/path.h>

void ibutton_scene_info_on_enter(void* context) {
    iButton* ibutton = context;
    Widget* widget = ibutton->widget;
    iButtonKey* key = ibutton->key;

    const uint8_t* key_data = ibutton_key_get_data_p(key);

    string_t key_name;
    string_init(key_name);
    path_extract_filename(ibutton->file_path, key_name, true);

    ibutton_text_store_set(ibutton, "%s", string_get_cstr(key_name));
    widget_add_text_box_element(
        widget, 0, 0, 128, 28, AlignCenter, AlignCenter, ibutton->text_store, false);

    switch(ibutton_key_get_type(key)) {
    case iButtonKeyDS1990:
        ibutton_text_store_set(
            ibutton,
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
        ibutton_text_store_set(
            ibutton, "%02X %02X %02X %02X", key_data[0], key_data[1], key_data[2], key_data[3]);
        widget_add_string_element(
            widget, 64, 51, AlignCenter, AlignBottom, FontSecondary, "Metakom");
        break;

    case iButtonKeyCyfral:
        ibutton_text_store_set(ibutton, "%02X %02X", key_data[0], key_data[1]);
        widget_add_string_element(
            widget, 64, 51, AlignCenter, AlignBottom, FontSecondary, "Cyfral");
        break;
    }

    widget_add_string_element(
        widget, 64, 35, AlignCenter, AlignBottom, FontPrimary, ibutton->text_store);

    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewWidget);

    string_clear(key_name);
}

bool ibutton_scene_info_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void ibutton_scene_info_on_exit(void* context) {
    iButton* ibutton = context;
    ibutton_text_store_clear(ibutton);
    widget_reset(ibutton->widget);
}
