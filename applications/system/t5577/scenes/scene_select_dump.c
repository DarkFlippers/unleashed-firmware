#include "../t5577_app.h"
#include <t5577_icons.h>
#include <string.h>

// The first dump in a folder, which the browser is then opened on.
//
// Two things come of looking first. Nothing to browse is worth saying rather than showing, since
// a browser opened on an empty folder is a screen that looks broken - there is nothing on it and
// nothing to press. And opening it on a file rather than on the folder means a file is selected
// when it appears, where a folder leaves the browser with nothing under the cursor.
static bool t5577_app_scene_select_dump_first_in(
    T5577App* app,
    const char* folder,
    FuriString* path,
    bool* readable) {
    File* dir = storage_file_alloc(app->storage);
    bool found = false;

    *readable = storage_dir_open(dir, folder);
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

            furi_string_printf(path, "%s/%s", folder, name);
            found = true;
        }
    }

    storage_dir_close(dir);
    storage_file_free(dir);

    return found;
}

// Where the browser opens: the app folder, and failing that this app's own folder, which still
// holds the dumps earlier versions saved there. Only a folder with a dump in it answers - the
// app folder is the lfrfid one now, and the subfolders of keys people keep there would otherwise
// be offered as somewhere to find dumps that are not in them.
static bool t5577_app_scene_select_dump_first(T5577App* app, FuriString* path, bool* readable) {
    if(t5577_app_scene_select_dump_first_in(app, T5577_APP_FOLDER, path, readable)) return true;
    if(!*readable) return false;

    bool own_readable = false;
    return t5577_app_scene_select_dump_first_in(app, T5577_APP_OWN_FOLDER, path, &own_readable);
}

// True when a dump was picked and opened, with the file menu already pushed.
static bool t5577_app_scene_select_dump_show(T5577App* app) {
    // The dump this run last opened or saved is where the browser goes back to, which is what
    // makes leaving the file menu land on the file that was in it. Checked rather than trusted:
    // it may have been renamed - the new name is here - or deleted, and it costs one stat where
    // the scan below reads past every key in the lfrfid folder.
    bool readable = true;
    if(!storage_file_exists(app->storage, furi_string_get_cstr(app->path)) &&
       !t5577_app_scene_select_dump_first(app, app->path, &readable)) {
        // A folder that cannot be read is not an empty folder, and telling a user who saved
        // dumps yesterday that they have none is the wrong thing to say about a card.
        if(readable) {
            t5577_app_say(app, "Nothing saved", "Read a tag and\nsave it first");
        } else {
            dialog_message_show_storage_error(app->dialogs, "Cannot read the\ndumps folder");
        }
        return false;
    }

    DialogsFileBrowserOptions options;
    dialog_file_browser_set_basic_options(&options, T5577_FILE_EXTENSION, &I_t5577_10px);
    options.base_path = T5577_APP_FOLDER;

    if(!dialog_file_browser_show(app->dialogs, app->path, app->path, &options)) return false;

    return t5577_app_open_dump(app);
}

void t5577_app_scene_select_dump_on_enter(void* context) {
    T5577App* app = context;

    if(t5577_app_scene_select_dump_show(app)) {
        scene_manager_next_scene(app->scene_manager, T5577AppSceneFileMenu);
        return;
    }

    // Nothing was opened, so there is nothing for Back to come back to: this scene is the
    // browser, and re-entering it would open it again on the file the user just declined.
    scene_manager_search_and_switch_to_previous_scene(app->scene_manager, T5577AppSceneStart);
}

bool t5577_app_scene_select_dump_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);

    return false;
}

void t5577_app_scene_select_dump_on_exit(void* context) {
    UNUSED(context);
}
