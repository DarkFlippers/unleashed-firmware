#include "../lfrfid_i.h"

typedef enum {
    SubmenuIndexFields,
    SubmenuIndexHex,
} SubmenuIndex;

static void lfrfid_scene_save_method_submenu_callback(void* context, uint32_t index) {
    LfRfid* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void lfrfid_scene_save_method_on_enter(void* context) {
    LfRfid* app = context;
    Submenu* submenu = app->submenu;

    FuriString* label = furi_string_alloc();
    lfrfid_manual_format_get_label(app->manual_format, label);
    submenu_set_header(submenu, furi_string_get_cstr(label));
    furi_string_free(label);
    submenu_add_item(
        submenu, "Enter FC/ID", SubmenuIndexFields, lfrfid_scene_save_method_submenu_callback, app);
    submenu_add_item(
        submenu, "Enter Hex Data", SubmenuIndexHex, lfrfid_scene_save_method_submenu_callback, app);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(app->scene_manager, LfRfidSceneSaveMethod));

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewSubmenu);
}

bool lfrfid_scene_save_method_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(app->scene_manager, LfRfidSceneSaveMethod, event.event);
        if(event.event == SubmenuIndexFields) {
            // start from blank fields, coming back from the name entry keeps them
            memset(app->field_values, 0, sizeof(app->field_values));
            scene_manager_next_scene(app->scene_manager, LfRfidSceneSaveFields);
            consumed = true;
        } else if(event.event == SubmenuIndexHex) {
            scene_manager_next_scene(app->scene_manager, LfRfidSceneSaveData);
            consumed = true;
        }
    }

    return consumed;
}

void lfrfid_scene_save_method_on_exit(void* context) {
    LfRfid* app = context;

    submenu_reset(app->submenu);
}
