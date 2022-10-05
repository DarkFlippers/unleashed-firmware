#include "../ibutton_i.h"
#include <toolbox/path.h>

void ibutton_scene_info_on_enter(void* context) {
    iButton* ibutton = context;
    Widget* widget = ibutton->widget;
    iButtonKey* key = ibutton->key;

    const uint8_t* key_data = ibutton_key_get_data_p(key);

    FuriString* key_name;
    key_name = furi_string_alloc();
    path_extract_filename(ibutton->file_path, key_name, true);

    ibutton_text_store_set(ibutton, "%s", furi_string_get_cstr(key_name));
    widget_add_text_box_element(
        widget, 0, 0, 128, 23, AlignCenter, AlignCenter, ibutton->text_store, true);

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
        widget_add_string_element(widget, 64, 36, AlignCenter, AlignBottom, FontPrimary, "Dallas");
        break;

    case iButtonKeyMetakom:
        ibutton_text_store_set(
            ibutton, "%02X %02X %02X %02X", key_data[0], key_data[1], key_data[2], key_data[3]);
        widget_add_string_element(
            widget, 64, 36, AlignCenter, AlignBottom, FontPrimary, "Metakom");
        break;

    case iButtonKeyCyfral:
        ibutton_text_store_set(ibutton, "%02X %02X", key_data[0], key_data[1]);
        widget_add_string_element(widget, 64, 36, AlignCenter, AlignBottom, FontPrimary, "Cyfral");
        break;
    }

    widget_add_string_element(
        widget, 64, 50, AlignCenter, AlignBottom, FontSecondary, ibutton->text_store);

    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewWidget);

    furi_string_free(key_name);
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
