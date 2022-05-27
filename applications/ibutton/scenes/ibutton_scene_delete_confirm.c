#include "../ibutton_i.h"
#include <toolbox/path.h>

static void ibutton_scene_delete_confirm_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    iButton* ibutton = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(ibutton->view_dispatcher, result);
    }
}

void ibutton_scene_delete_confirm_on_enter(void* context) {
    iButton* ibutton = context;
    Widget* widget = ibutton->widget;
    iButtonKey* key = ibutton->key;
    const uint8_t* key_data = ibutton_key_get_data_p(key);

    string_t key_name;
    string_init(key_name);
    path_extract_filename(ibutton->file_path, key_name, true);

    ibutton_text_store_set(ibutton, "\e#Delete %s?\e#", string_get_cstr(key_name));
    widget_add_text_box_element(
        widget, 0, 0, 128, 27, AlignCenter, AlignCenter, ibutton->text_store, false);
    widget_add_button_element(
        widget, GuiButtonTypeLeft, "Cancel", ibutton_scene_delete_confirm_widget_callback, ibutton);
    widget_add_button_element(
        widget,
        GuiButtonTypeRight,
        "Delete",
        ibutton_scene_delete_confirm_widget_callback,
        ibutton);

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
            widget, 64, 45, AlignCenter, AlignBottom, FontSecondary, "Dallas");
        break;

    case iButtonKeyCyfral:
        ibutton_text_store_set(ibutton, "%02X %02X", key_data[0], key_data[1]);
        widget_add_string_element(
            widget, 64, 45, AlignCenter, AlignBottom, FontSecondary, "Cyfral");
        break;

    case iButtonKeyMetakom:
        ibutton_text_store_set(
            ibutton, "%02X %02X %02X %02X", key_data[0], key_data[1], key_data[2], key_data[3]);
        widget_add_string_element(
            widget, 64, 45, AlignCenter, AlignBottom, FontSecondary, "Metakom");
        break;
    }
    widget_add_string_element(
        widget, 64, 33, AlignCenter, AlignBottom, FontSecondary, ibutton->text_store);

    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewWidget);

    string_clear(key_name);
}

bool ibutton_scene_delete_confirm_on_event(void* context, SceneManagerEvent event) {
    iButton* ibutton = context;
    SceneManager* scene_manager = ibutton->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == GuiButtonTypeRight) {
            if(ibutton_delete_key(ibutton)) {
                scene_manager_next_scene(scene_manager, iButtonSceneDeleteSuccess);
            }
            //TODO: What if the key could not be deleted?
        } else if(event.event == GuiButtonTypeLeft) {
            scene_manager_previous_scene(scene_manager);
        }
    }

    return consumed;
}

void ibutton_scene_delete_confirm_on_exit(void* context) {
    iButton* ibutton = context;
    ibutton_text_store_clear(ibutton);
    widget_reset(ibutton->widget);
}
