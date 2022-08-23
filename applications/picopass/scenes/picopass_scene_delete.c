#include "../picopass_i.h"

void picopass_scene_delete_widget_callback(GuiButtonType result, InputType type, void* context) {
    Picopass* picopass = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(picopass->view_dispatcher, result);
    }
}

void picopass_scene_delete_on_enter(void* context) {
    Picopass* picopass = context;

    // Setup Custom Widget view
    char temp_str[64];
    snprintf(temp_str, sizeof(temp_str), "\e#Delete %s?\e#", picopass->dev->dev_name);
    widget_add_text_box_element(
        picopass->widget, 0, 0, 128, 23, AlignCenter, AlignCenter, temp_str, false);
    widget_add_button_element(
        picopass->widget,
        GuiButtonTypeLeft,
        "Back",
        picopass_scene_delete_widget_callback,
        picopass);
    widget_add_button_element(
        picopass->widget,
        GuiButtonTypeRight,
        "Delete",
        picopass_scene_delete_widget_callback,
        picopass);

    view_dispatcher_switch_to_view(picopass->view_dispatcher, PicopassViewWidget);
}

bool picopass_scene_delete_on_event(void* context, SceneManagerEvent event) {
    Picopass* picopass = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            return scene_manager_previous_scene(picopass->scene_manager);
        } else if(event.event == GuiButtonTypeRight) {
            if(picopass_device_delete(picopass->dev, true)) {
                scene_manager_next_scene(picopass->scene_manager, PicopassSceneDeleteSuccess);
            } else {
                scene_manager_search_and_switch_to_previous_scene(
                    picopass->scene_manager, PicopassSceneStart);
            }
            consumed = true;
        }
    }
    return consumed;
}

void picopass_scene_delete_on_exit(void* context) {
    Picopass* picopass = context;

    widget_reset(picopass->widget);
}
