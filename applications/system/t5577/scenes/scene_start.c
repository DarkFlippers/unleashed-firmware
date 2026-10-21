#include "../t5577_app.h"
#include <t5577_icons.h>
#include <string.h>
#include <dolphin/dolphin.h>

typedef enum {
    StartIndexRead,
    StartIndexTest,
    StartIndexSaved,
    StartIndexWipe,
    StartIndexClear,
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
    submenu_add_item(
        app->submenu, "Test Tag", StartIndexTest, t5577_app_scene_start_callback, app);
    submenu_add_item(app->submenu, "Saved", StartIndexSaved, t5577_app_scene_start_callback, app);
    submenu_add_item(
        app->submenu, "Wipe Tag", StartIndexWipe, t5577_app_scene_start_callback, app);
    submenu_add_item(
        app->submenu, "Clear Password", StartIndexClear, t5577_app_scene_start_callback, app);
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
        // What the editor compares against: at this instant the file and the dump are the same.
        app->saved = app->dump;
        scene_manager_next_scene(app->scene_manager, T5577AppSceneFileMenu);
        return;
    }

    t5577_app_say(app, "Cannot open", "Not a T5577 dump,\nor it holds nothing");
}

// A backup outlives its run only when that run left a block changed, so one still here is a
// tag that was never put back - and this run would overwrite its way home under the same name.
static bool t5577_app_scene_start_backup_spent(T5577App* app) {
    FuriString* path = furi_string_alloc();
    t5577_app_test_backup_path(path);
    const bool exists = storage_file_exists(app->storage, furi_string_get_cstr(path));
    furi_string_free(path);

    if(!exists) return true;

    return t5577_app_ask(
        app,
        "Unfinished test",
        "A tag was left changed\nby the last test. This\noverwrites its backup",
        "Overwrite");
}

bool t5577_app_scene_start_on_event(void* context, SceneManagerEvent event) {
    T5577App* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;
    // Only the submenu's own indices get here as a choice; see the file menu for why.
    if(event.event > StartIndexClear) return false;

    scene_manager_set_scene_state(app->scene_manager, T5577AppSceneStart, event.event);

    if(event.event == StartIndexRead) {
        // A password tried against the last tag says nothing about this one. The dump itself is
        // cleared by the read scene, which has to do it for a retry anyway.
        t5577_app_forget_password(app);
        scene_manager_next_scene(app->scene_manager, T5577AppSceneRead);
        dolphin_deed(DolphinDeedRfidRead);
    } else if(event.event == StartIndexTest) {
        // Asked before the first write rather than explained after it, and the backup is named
        // in the question: the way out has to exist before the risk is taken.
        if(t5577_app_scene_start_backup_spent(app) &&
           t5577_app_ask(
               app,
               "Do not move the tag",
               "A backup is saved, then\nevery block is written\nand put back",
               "Test")) {
            scene_manager_next_scene(app->scene_manager, T5577AppSceneTest);
        }
    } else if(event.event == StartIndexSaved) {
        t5577_app_scene_start_open_saved(app);
    } else if(event.event == StartIndexWipe) {
        // Nothing a tag was carrying survives this and nothing is saved first, so it is the one
        // thing the app does that the user has to mean. What it loses is said in this app's own
        // terms: the lfrfid wording was "won't read until re-written", true of a reader looking
        // for a key and false here, since the wipe leaves a configuration the app can still read
        // - seven zeroed data blocks, and page 1 as the chip was made.
        if(t5577_app_ask(
               app,
               "Wipe T5577",
               "Erases all data. The tag\nstops working as a key.\nHold still while running",
               "Wipe")) {
            scene_manager_next_scene(app->scene_manager, T5577AppSceneWipe);
        }
    } else {
        // A password of the user's own goes first and is asked for first, as it was in the
        // lfrfid app: the sweep tries it before the dictionary, and a tag whose password is
        // known should not wait through a hundred guesses to be opened by it.
        scene_manager_set_scene_state(
            app->scene_manager, T5577AppScenePassword, T5577AppSceneClear);
        scene_manager_next_scene(app->scene_manager, T5577AppScenePassword);
    }

    return true;
}

void t5577_app_scene_start_on_exit(void* context) {
    T5577App* app = context;
    submenu_reset(app->submenu);
}
