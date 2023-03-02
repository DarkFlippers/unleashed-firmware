#include "../ibutton_i.h"
#include <toolbox/path.h>

void ibutton_scene_delete_confirm_on_enter(void* context) {
    iButton* ibutton = context;
    iButtonKey* key = ibutton->key;
    Widget* widget = ibutton->widget;

    FuriString* tmp = furi_string_alloc();

    widget_add_button_element(widget, GuiButtonTypeLeft, "Back", ibutton_widget_callback, context);
    widget_add_button_element(
        widget, GuiButtonTypeRight, "Delete", ibutton_widget_callback, context);

    furi_string_printf(tmp, "Delete %s?", ibutton->key_name);
    widget_add_string_element(
        widget, 128 / 2, 0, AlignCenter, AlignTop, FontPrimary, furi_string_get_cstr(tmp));

    furi_string_reset(tmp);
    ibutton_protocols_render_brief_data(ibutton->protocols, key, tmp);

    widget_add_string_multiline_element(
        widget, 128 / 2, 16, AlignCenter, AlignTop, FontSecondary, furi_string_get_cstr(tmp));

    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewWidget);
    furi_string_free(tmp);
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
            } else {
                dialog_message_show_storage_error(ibutton->dialogs, "Cannot delete\nkey file");
                scene_manager_previous_scene(scene_manager);
            }
        } else if(event.event == GuiButtonTypeLeft) {
            scene_manager_previous_scene(scene_manager);
        }
    }

    return consumed;
}

void ibutton_scene_delete_confirm_on_exit(void* context) {
    iButton* ibutton = context;
    widget_reset(ibutton->widget);
}
