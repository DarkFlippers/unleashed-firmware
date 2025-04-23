#include "submenu_based.h"
#include <archive/helpers/archive_favorites.h>

struct SubmenuSettingsHelper {
    const SubmenuSettingsHelperDescriptor* descriptor;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    Submenu* submenu;
    uint32_t submenu_view_id;
    uint32_t main_scene_id;
};

SubmenuSettingsHelper*
    submenu_settings_helpers_alloc(const SubmenuSettingsHelperDescriptor* descriptor) {
    furi_check(descriptor);
    SubmenuSettingsHelper* helper = malloc(sizeof(SubmenuSettingsHelper));
    helper->descriptor = descriptor;
    return helper;
}

void submenu_settings_helpers_assign_objects(
    SubmenuSettingsHelper* helper,
    ViewDispatcher* view_dispatcher,
    SceneManager* scene_manager,
    Submenu* submenu,
    uint32_t submenu_view_id,
    uint32_t main_scene_id) {
    furi_check(helper);
    furi_check(view_dispatcher);
    furi_check(scene_manager);
    furi_check(submenu);
    helper->view_dispatcher = view_dispatcher;
    helper->scene_manager = scene_manager;
    helper->submenu = submenu;
    helper->submenu_view_id = submenu_view_id;
    helper->main_scene_id = main_scene_id;
}

void submenu_settings_helpers_free(SubmenuSettingsHelper* helper) {
    free(helper);
}

bool submenu_settings_helpers_app_start(SubmenuSettingsHelper* helper, void* arg) {
    furi_check(helper);
    if(!arg) return false;

    const char* option = arg;
    for(size_t i = 0; i < helper->descriptor->options_cnt; i++) {
        if(strcmp(helper->descriptor->options[i].name, option) == 0) {
            scene_manager_next_scene(
                helper->scene_manager, helper->descriptor->options[i].scene_id);
            return true;
        }
    }

    return false;
}

static void
    submenu_settings_helpers_callback(void* context, InputType input_type, uint32_t index) {
    SubmenuSettingsHelper* helper = context;
    if(input_type == InputTypeShort) {
        view_dispatcher_send_custom_event(helper->view_dispatcher, index);
    } else if(input_type == InputTypeLong) {
        archive_favorites_handle_setting_pin_unpin(
            helper->descriptor->app_name, helper->descriptor->options[index].name);
    }
}

void submenu_settings_helpers_scene_enter(SubmenuSettingsHelper* helper) {
    furi_check(helper);
    for(size_t i = 0; i < helper->descriptor->options_cnt; i++) {
        submenu_add_item_ex(
            helper->submenu,
            helper->descriptor->options[i].name,
            i,
            submenu_settings_helpers_callback,
            helper);
    }

    submenu_set_selected_item(
        helper->submenu,
        scene_manager_get_scene_state(helper->scene_manager, helper->main_scene_id));
    view_dispatcher_switch_to_view(helper->view_dispatcher, helper->submenu_view_id);
}

bool submenu_settings_helpers_scene_event(SubmenuSettingsHelper* helper, SceneManagerEvent event) {
    furi_check(helper);

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_next_scene(
            helper->scene_manager, helper->descriptor->options[event.event].scene_id);
        scene_manager_set_scene_state(helper->scene_manager, helper->main_scene_id, event.event);
        return true;
    }

    return false;
}

void submenu_settings_helpers_scene_exit(SubmenuSettingsHelper* helper) {
    furi_check(helper);
    submenu_reset(helper->submenu);
}
