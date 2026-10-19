#include "../t5577_app.h"
#include <t5577_icons.h>
#include <string.h>
#include <dolphin/dolphin.h>

typedef enum {
    StartIndexRead,
    StartIndexSaved,
} StartIndex;

static void t5577_app_scene_start_callback(void* context, uint32_t index) {
    T5577App* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void t5577_app_scene_start_on_enter(void* context) {
    T5577App* app = context;

    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "T5577");
    submenu_add_item(
        app->submenu, "Read Tag", StartIndexRead, t5577_app_scene_start_callback, app);
    submenu_add_item(app->submenu, "Saved", StartIndexSaved, t5577_app_scene_start_callback, app);
    submenu_set_selected_item(
        app->submenu, scene_manager_get_scene_state(app->scene_manager, T5577AppSceneStart));

    view_dispatcher_switch_to_view(app->view_dispatcher, T5577AppViewSubmenu);
}

// The first saved dump, which the browser is then opened on.
//
// Two things come of looking first. Nothing to browse is worth saying rather than showing, since
// a browser opened on an empty folder is a screen that looks broken - there is nothing on it and
// nothing to press. And opening it on a file rather than on the folder means a file is selected
// when it appears, where a folder leaves the browser with nothing under the cursor.
static bool t5577_app_scene_start_first_saved(T5577App* app, FuriString* path, bool* readable) {
    File* dir = storage_file_alloc(app->storage);
    bool found = false;

    *readable = storage_dir_open(dir, T5577_APP_FOLDER);
    if(*readable) {
        FileInfo info;
        char name[64];
        const size_t suffix = strlen(T5577_FILE_EXTENSION);

        while(!found && storage_dir_read(dir, &info, name, sizeof(name))) {
            const size_t length = strlen(name);
            // A folder can be named like a dump, and is not one.
            if(file_info_is_dir(&info)) continue;
            if(length <= suffix) continue;
            if(strcasecmp(&name[length - suffix], T5577_FILE_EXTENSION) != 0) continue;

            furi_string_printf(path, "%s/%s", T5577_APP_FOLDER, name);
            found = true;
        }
    }

    storage_dir_close(dir);
    storage_file_free(dir);

    return found;
}

static void t5577_app_scene_start_open_saved(T5577App* app) {
    bool readable = false;
    if(!t5577_app_scene_start_first_saved(app, app->path, &readable)) {
        // A folder that cannot be read is not an empty folder, and telling a user who saved
        // dumps yesterday that they have none is the wrong thing to say about a card.
        if(readable) {
            t5577_app_say(app, "Nothing saved", "Read a tag and\nsave it first");
        } else {
            dialog_message_show_storage_error(app->dialogs, "Cannot read the\ndumps folder");
        }
        return;
    }

    DialogsFileBrowserOptions options;
    dialog_file_browser_set_basic_options(&options, T5577_FILE_EXTENSION, &I_125_10px);
    options.base_path = T5577_APP_FOLDER;

    if(!dialog_file_browser_show(app->dialogs, app->path, app->path, &options)) return;

    if(t5577_file_load(&app->dump, furi_string_get_cstr(app->path))) {
        scene_manager_next_scene(app->scene_manager, T5577AppSceneFileMenu);
        return;
    }

    t5577_app_say(app, "Cannot open", "Not a T5577 dump,\nor it holds nothing");
}

bool t5577_app_scene_start_on_event(void* context, SceneManagerEvent event) {
    T5577App* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;
    // Only the submenu's own indices get here as a choice; see the file menu for why.
    if(event.event > StartIndexSaved) return false;

    scene_manager_set_scene_state(app->scene_manager, T5577AppSceneStart, event.event);

    if(event.event == StartIndexRead) {
        // A password tried against the last tag says nothing about this one. The dump itself is
        // cleared by the read scene, which has to do it for a retry anyway.
        t5577_app_forget_password(app);
        scene_manager_next_scene(app->scene_manager, T5577AppSceneRead);
        dolphin_deed(DolphinDeedRfidRead);
    } else {
        t5577_app_scene_start_open_saved(app);
    }

    return true;
}

void t5577_app_scene_start_on_exit(void* context) {
    T5577App* app = context;
    submenu_reset(app->submenu);
}
