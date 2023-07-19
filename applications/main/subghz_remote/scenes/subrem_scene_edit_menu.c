#include "../subghz_remote_app_i.h"

void subrem_scene_edit_menu_callback(SubRemCustomEvent event, void* context) {
    furi_assert(context);
    SubGhzRemoteApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, event);
}

void subrem_scene_edit_menu_widget_callback(GuiButtonType result, InputType type, void* context) {
    furi_assert(context);
    SubGhzRemoteApp* app = context;
    if((result == GuiButtonTypeRight) && (type == InputTypeShort)) {
        app->map_not_saved = false;
        view_dispatcher_send_custom_event(app->view_dispatcher, SubRemCustomEventViewEditMenuBack);
    } else if((result == GuiButtonTypeLeft) && (type == InputTypeShort)) {
        view_dispatcher_switch_to_view(app->view_dispatcher, SubRemViewIDEditMenu);
    }
}

static uint8_t subrem_scene_edit_menu_state_to_index(SubRemEditMenuState event_id) {
    uint8_t ret = 0;

    if(event_id == SubRemEditMenuStateUP) {
        ret = SubRemSubKeyNameUp;
    } else if(event_id == SubRemEditMenuStateDOWN) {
        ret = SubRemSubKeyNameDown;
    } else if(event_id == SubRemEditMenuStateLEFT) {
        ret = SubRemSubKeyNameLeft;
    } else if(event_id == SubRemEditMenuStateRIGHT) {
        ret = SubRemSubKeyNameRight;
    } else if(event_id == SubRemEditMenuStateOK) {
        ret = SubRemSubKeyNameOk;
    }

    return ret;
}

static void subrem_scene_edit_menu_update_data(SubGhzRemoteApp* app) {
    furi_assert(app);
    uint8_t index = subrem_scene_edit_menu_state_to_index(
        scene_manager_get_scene_state(app->scene_manager, SubRemSceneEditMenu));

    subrem_view_edit_menu_add_data_to_show(
        app->subrem_edit_menu,
        index,
        app->map_preset->subs_preset[index]->label,
        app->map_preset->subs_preset[index]->file_path,
        app->map_preset->subs_preset[index]->load_state);
}

void subrem_scene_edit_menu_on_enter(void* context) {
    SubGhzRemoteApp* app = context;

    subrem_view_edit_menu_set_callback(
        app->subrem_edit_menu, subrem_scene_edit_menu_callback, app);

    subrem_scene_edit_menu_update_data(app);

    view_dispatcher_switch_to_view(app->view_dispatcher, SubRemViewIDEditMenu);

    Widget* widget = app->widget;

    widget_add_string_element(
        widget, 63, 12, AlignCenter, AlignBottom, FontPrimary, "Changes are not saved");
    widget_add_string_element(
        widget, 63, 32, AlignCenter, AlignBottom, FontPrimary, "do you want to exit?");

    widget_add_button_element(
        widget, GuiButtonTypeRight, "Yes", subrem_scene_edit_menu_widget_callback, app);
    widget_add_button_element(
        widget, GuiButtonTypeLeft, "No", subrem_scene_edit_menu_widget_callback, app);
}

bool subrem_scene_edit_menu_on_event(void* context, SceneManagerEvent event) {
    SubGhzRemoteApp* app = context;

    if(event.type == SceneManagerEventTypeBack) {
        // Catch widget backEvent
        return true;
    }

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubRemCustomEventViewEditMenuBack) {
            if(app->map_not_saved) {
                view_dispatcher_switch_to_view(app->view_dispatcher, SubRemViewIDWidget);
            } else if(!scene_manager_search_and_switch_to_previous_scene(
                          app->scene_manager, SubRemSceneStart)) {
                scene_manager_stop(app->scene_manager);
                view_dispatcher_stop(app->view_dispatcher);
            }

            return true;
        } else if(
            event.event == SubRemCustomEventViewEditMenuUP ||
            event.event == SubRemCustomEventViewEditMenuDOWN) {
            scene_manager_set_scene_state(
                app->scene_manager,
                SubRemSceneEditMenu,
                subrem_view_edit_menu_get_index(app->subrem_edit_menu));
            subrem_scene_edit_menu_update_data(app);

            return true;
        } else if(event.event == SubRemCustomEventViewEditMenuEdit) {
            app->chusen_sub = subrem_view_edit_menu_get_index(app->subrem_edit_menu);
            scene_manager_set_scene_state(
                app->scene_manager, SubRemSceneEditSubMenu, EditSubmenuIndexEditLabel);
            scene_manager_next_scene(app->scene_manager, SubRemSceneEditSubMenu);

            return true;
        } else if(event.event == SubRemCustomEventViewEditMenuSave) {
            scene_manager_next_scene(app->scene_manager, SubRemSceneEditPreview);

            return true;
        }
    }

    return false;
}

void subrem_scene_edit_menu_on_exit(void* context) {
    SubGhzRemoteApp* app = context;
    widget_reset(app->widget);
}
