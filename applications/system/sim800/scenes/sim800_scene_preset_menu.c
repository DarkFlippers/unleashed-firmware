#include "../sim800_app_i.h"

typedef enum {
    SubmenuIndexSend,
    SubmenuIndexView,
    SubmenuIndexEdit,
    SubmenuIndexRename,
    SubmenuIndexDelete,
} SubmenuIndex;

#define SCENE_STATE_MENU 0
#define SCENE_STATE_TEXT 1

static void sim800_preset_menu_show_menu(Sim800App* app) {
    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, furi_string_get_cstr(app->entry_name));
    submenu_add_item(
        app->submenu, "Send to number", SubmenuIndexSend, sim800_submenu_callback, app);
    submenu_add_item(app->submenu, "View text", SubmenuIndexView, sim800_submenu_callback, app);
    submenu_add_item(app->submenu, "Edit text", SubmenuIndexEdit, sim800_submenu_callback, app);
    submenu_add_item(app->submenu, "Rename", SubmenuIndexRename, sim800_submenu_callback, app);
    submenu_add_item(app->submenu, "Delete", SubmenuIndexDelete, sim800_submenu_callback, app);

    scene_manager_set_scene_state(app->scene_manager, Sim800ScenePresetMenu, SCENE_STATE_MENU);
    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewSubmenu);
}

void sim800_scene_preset_menu_on_enter(void* context) {
    sim800_preset_menu_show_menu(context);
}

bool sim800_scene_preset_menu_on_event(void* context, SceneManagerEvent event) {
    Sim800App* app = context;

    if(event.type == SceneManagerEventTypeBack) {
        if(scene_manager_get_scene_state(app->scene_manager, Sim800ScenePresetMenu) ==
           SCENE_STATE_TEXT) {
            sim800_preset_menu_show_menu(app);
            return true;
        }
        return false;
    }

    if(event.type != SceneManagerEventTypeCustom) return false;

    switch(event.event) {
    case SubmenuIndexSend:
        if(sim800_preset_load(furi_string_get_cstr(app->file_path), app->sms_text)) {
            app->mode = Sim800ModeSmsReady;
            sim800_number_ask(app);
        }
        return true;

    case SubmenuIndexView:
        if(sim800_preset_load(furi_string_get_cstr(app->file_path), app->text_box_store)) {
            text_box_reset(app->text_box);
            text_box_set_font(app->text_box, TextBoxFontText);
            text_box_set_text(app->text_box, furi_string_get_cstr(app->text_box_store));
            scene_manager_set_scene_state(
                app->scene_manager, Sim800ScenePresetMenu, SCENE_STATE_TEXT);
            view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewTextBox);
        }
        return true;

    case SubmenuIndexEdit:
        if(sim800_preset_load(furi_string_get_cstr(app->file_path), app->sms_text)) {
            app->preset_is_new = false;
            scene_manager_next_scene(app->scene_manager, Sim800ScenePresetText);
        }
        return true;

    case SubmenuIndexRename:
        if(sim800_preset_load(furi_string_get_cstr(app->file_path), app->sms_text)) {
            app->save_target = Sim800SaveTargetRenamePreset;
            scene_manager_next_scene(app->scene_manager, Sim800SceneSaveName);
        }
        return true;

    case SubmenuIndexDelete:
        sim800_notify_result(app, sim800_preset_delete(furi_string_get_cstr(app->file_path)));
        scene_manager_previous_scene(app->scene_manager);
        return true;

    default:
        return false;
    }
}

void sim800_scene_preset_menu_on_exit(void* context) {
    Sim800App* app = context;
    submenu_reset(app->submenu);
    text_box_reset(app->text_box);
}
