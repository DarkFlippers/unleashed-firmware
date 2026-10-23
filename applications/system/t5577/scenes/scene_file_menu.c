#include "../t5577_app.h"
#include <toolbox/path.h>

typedef enum {
    FileMenuIndexInfo,
    FileMenuIndexRestore,
    FileMenuIndexEdit,
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
    // apart.
    FuriString* name = furi_string_alloc();
    t5577_app_short_name(app, name);
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
        app->submenu, "Edit", FileMenuIndexEdit, t5577_app_scene_file_menu_callback, app);
    submenu_add_item(
        app->submenu, "Rename", FileMenuIndexRename, t5577_app_scene_file_menu_callback, app);
    submenu_add_item(
        app->submenu, "Delete", FileMenuIndexDelete, t5577_app_scene_file_menu_callback, app);
    submenu_set_selected_item(
        app->submenu, scene_manager_get_scene_state(app->scene_manager, T5577AppSceneFileMenu));

    view_dispatcher_switch_to_view(app->view_dispatcher, T5577AppViewSubmenu);
}

// Writing to a tag cannot be taken back, so it is asked for rather than assumed, and the count
// is in the question: a dump read off a tag that drifted holds fewer blocks than it looks like.
static bool t5577_app_scene_file_menu_confirm_restore(T5577App* app) {
    uint8_t page_0 = 0;
    uint8_t page_1 = 0;

    for(uint8_t i = 0; i < T5577_RESTORE_BLOCKS; i++) {
        if(!t5577_restore_wanted(&app->dump, i)) continue;

        const T5577Target target = t5577_restore_target(i);
        if(target.page == 0) {
            page_0++;
        } else {
            page_1++;
        }
    }

    // Nothing to write is not a write worth starting: it would put the tag through a read
    // back of no blocks at all and call that a success.
    if(page_0 + page_1 == 0) {
        t5577_app_say(app, "Nothing to write", "This dump holds no\nblocks from a tag");
        return false;
    }

    FuriString* text =
        furi_string_alloc_printf("%u blocks of page 0 and\n%u of page 1 (trace)", page_0, page_1);

    // A configuration this reader cannot follow is written all the same, and then nothing can be
    // read back from the tag - not block 0, not the data blocks, not on any later visit. Said
    // before the write, because afterwards the only honest thing left to report is that nothing
    // could be checked. Asked of the block the write will actually send, so a dump whose block 0
    // is only padding cannot raise it.
    if(t5577_restore_wanted(&app->dump, T5577_RESTORE_BLOCKS - 1) &&
       !t5577_config_readable(app->dump.block[0][0])) {
        furi_string_cat_str(text, "\nNothing will read back");
    }

    const bool write = t5577_app_ask(app, "Write to tag", furi_string_get_cstr(text), "Write");

    furi_string_free(text);

    return write;
}

static bool t5577_app_scene_file_menu_confirm_delete(T5577App* app) {
    FuriString* name = furi_string_alloc();
    path_extract_filename(app->path, name, true);

    FuriString* text = furi_string_alloc_printf("Delete %s?", furi_string_get_cstr(name));

    const bool remove = t5577_app_ask(app, "Delete dump", furi_string_get_cstr(text), "Delete");

    furi_string_free(text);
    furi_string_free(name);

    return remove;
}

bool t5577_app_scene_file_menu_on_event(void* context, SceneManagerEvent event) {
    T5577App* app = context;

    // A dump handed over by another app goes back to that app, not on to a start menu the user
    // never saw.
    if(event.type == SceneManagerEventTypeBack && app->file_only) {
        t5577_app_exit(app);
        return true;
    }

    if(event.type != SceneManagerEventTypeCustom) return false;
    // Only the submenu's own indices are a menu choice; anything else that got queued is not
    // this scene's, and remembering it as the selected item would move the cursor.
    if(event.event > FileMenuIndexDelete) return false;

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
    case FileMenuIndexEdit:
        scene_manager_next_scene(app->scene_manager, T5577AppSceneEdit);
        return true;
    case FileMenuIndexRename:
        app->renaming = true;
        scene_manager_next_scene(app->scene_manager, T5577AppSceneSaveName);
        return true;
    case FileMenuIndexDelete:
        if(t5577_app_scene_file_menu_confirm_delete(app)) {
            // A delete that did not happen has to be said, not least because the dump holds
            // the tag's password and the user is entitled to believe it is gone.
            if(storage_simply_remove(app->storage, furi_string_get_cstr(app->path))) {
                // Said out loud, because the screen that showed the file gone may not be there:
                // a dump another app sent here closes the app instead of returning to a list.
                notification_message(app->notifications, &sequence_success);
                // Back one scene, which is the browser when the dump was picked there - it
                // reopens on what is left - and out of the app when another app sent it here.
                if(app->file_only) {
                    t5577_app_exit(app);
                } else {
                    scene_manager_previous_scene(app->scene_manager);
                }
            } else {
                dialog_message_show_storage_error(app->dialogs, "The dump is still\non the card");
            }
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
