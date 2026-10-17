#include "../t5577_app.h"
#include <toolbox/path.h>

typedef enum {
    FileMenuIndexInfo,
    FileMenuIndexRestore,
    FileMenuIndexRename,
    FileMenuIndexDelete,
} FileMenuIndex;

static void t5577_app_scene_file_menu_callback(void* context, uint32_t index) {
    T5577App* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void t5577_app_scene_file_menu_on_enter(void* context) {
    T5577App* app = context;

    submenu_reset(app->submenu);

    // Which dump is open is worth the header: the name is the only thing telling two of them
    // apart, and a header has no room to scroll, so a long one is cut rather than overrun.
    FuriString* name = furi_string_alloc();
    path_extract_filename(app->path, name, true);
    if(furi_string_size(name) > T5577_APP_HEADER_MAX) {
        furi_string_left(name, T5577_APP_HEADER_MAX - 1);
        furi_string_push_back(name, '~');
    }
    submenu_set_header(app->submenu, furi_string_get_cstr(name));
    furi_string_free(name);
    submenu_add_item(
        app->submenu, "Info", FileMenuIndexInfo, t5577_app_scene_file_menu_callback, app);
    submenu_add_item(
        app->submenu,
        "Write to Tag",
        FileMenuIndexRestore,
        t5577_app_scene_file_menu_callback,
        app);
    submenu_add_item(
        app->submenu, "Rename", FileMenuIndexRename, t5577_app_scene_file_menu_callback, app);
    submenu_add_item(
        app->submenu, "Delete", FileMenuIndexDelete, t5577_app_scene_file_menu_callback, app);
    submenu_set_selected_item(
        app->submenu, scene_manager_get_scene_state(app->scene_manager, T5577AppSceneFileMenu));

    view_dispatcher_switch_to_view(app->view_dispatcher, T5577AppViewSubmenu);
}

// Writing to a tag cannot be taken back, so it is asked for rather than assumed, and the count
// is in the question: a dump read from a protected tag holds fewer blocks than it looks like.
static bool t5577_app_scene_file_menu_confirm_restore(T5577App* app) {
    uint8_t page_0 = 0;
    uint8_t page_1 = 0;

    for(uint8_t i = 0; i < T5577_APP_RESTORE_BLOCKS; i++) {
        if(!t5577_app_restore_wanted(&app->dump, i)) continue;

        if(t5577_app_restore_order[i].page == 0) {
            page_0++;
        } else {
            page_1++;
        }
    }

    FuriString* text =
        furi_string_alloc_printf("%u blocks of page 0 and\n%u of page 1 (trace)", page_0, page_1);

    DialogMessage* message = dialog_message_alloc();
    dialog_message_set_header(message, "Write to tag", 64, 4, AlignCenter, AlignTop);
    dialog_message_set_text(message, furi_string_get_cstr(text), 64, 22, AlignCenter, AlignTop);
    dialog_message_set_buttons(message, "Cancel", NULL, "Write");
    const DialogMessageButton pressed = dialog_message_show(app->dialogs, message);
    dialog_message_free(message);

    furi_string_free(text);

    return pressed == DialogMessageButtonRight;
}

static bool t5577_app_scene_file_menu_confirm_delete(T5577App* app) {
    FuriString* name = furi_string_alloc();
    path_extract_filename(app->path, name, true);

    FuriString* text = furi_string_alloc_printf("Delete %s?", furi_string_get_cstr(name));

    DialogMessage* message = dialog_message_alloc();
    dialog_message_set_header(message, "Delete dump", 64, 10, AlignCenter, AlignTop);
    dialog_message_set_text(message, furi_string_get_cstr(text), 64, 32, AlignCenter, AlignTop);
    dialog_message_set_buttons(message, "Cancel", NULL, "Delete");
    const DialogMessageButton pressed = dialog_message_show(app->dialogs, message);
    dialog_message_free(message);

    furi_string_free(text);
    furi_string_free(name);

    return pressed == DialogMessageButtonRight;
}

bool t5577_app_scene_file_menu_on_event(void* context, SceneManagerEvent event) {
    T5577App* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;

    scene_manager_set_scene_state(app->scene_manager, T5577AppSceneFileMenu, event.event);

    switch(event.event) {
    case FileMenuIndexInfo:
        scene_manager_next_scene(app->scene_manager, T5577AppSceneInfo);
        return true;
    case FileMenuIndexRestore:
        if(t5577_app_scene_file_menu_confirm_restore(app)) {
            scene_manager_next_scene(app->scene_manager, T5577AppSceneRestore);
        }
        return true;
    case FileMenuIndexRename:
        app->renaming = true;
        scene_manager_next_scene(app->scene_manager, T5577AppSceneSaveName);
        return true;
    case FileMenuIndexDelete:
        if(t5577_app_scene_file_menu_confirm_delete(app)) {
            storage_simply_remove(app->storage, furi_string_get_cstr(app->path));
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, T5577AppSceneStart);
        }
        return true;
    default:
        return false;
    }
}

void t5577_app_scene_file_menu_on_exit(void* context) {
    T5577App* app = context;
    submenu_reset(app->submenu);
}
