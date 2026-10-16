#include "../t5577_app.h"
#include <t5577_icons.h>
#include <string.h>

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
    submenu_add_item(
        app->submenu, "Saved Dumps", StartIndexSaved, t5577_app_scene_start_callback, app);
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
static bool t5577_app_scene_start_first_saved(T5577App* app, FuriString* path) {
    File* dir = storage_file_alloc(app->storage);
    bool found = false;

    if(storage_dir_open(dir, T5577_APP_FOLDER)) {
        char name[64];
        const size_t suffix = strlen(T5577_FILE_EXTENSION);

        while(!found && storage_dir_read(dir, NULL, name, sizeof(name))) {
            const size_t length = strlen(name);
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

static void t5577_app_scene_start_say(T5577App* app, const char* header, const char* text) {
    DialogMessage* message = dialog_message_alloc();
    dialog_message_set_header(message, header, 64, 10, AlignCenter, AlignTop);
    dialog_message_set_text(message, text, 64, 32, AlignCenter, AlignTop);
    dialog_message_set_buttons(message, NULL, "OK", NULL);
    dialog_message_show(app->dialogs, message);
    dialog_message_free(message);
}

static void t5577_app_scene_start_open_saved(T5577App* app) {
    if(!t5577_app_scene_start_first_saved(app, app->path)) {
        t5577_app_scene_start_say(app, "Nothing saved", "Read a tag and\nsave it first");
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

    t5577_app_scene_start_say(app, "Cannot open", "Not a T5577 dump\nthis version reads");
}

bool t5577_app_scene_start_on_event(void* context, SceneManagerEvent event) {
    T5577App* app = context;

    if(event.type != SceneManagerEventTypeCustom) return false;

    scene_manager_set_scene_state(app->scene_manager, T5577AppSceneStart, event.event);

    if(event.event == StartIndexRead) {
        // A read starts from nothing: the dump a previous read or a loaded file left behind is
        // not evidence about the tag now on the reader.
        memset(&app->dump, 0, sizeof(app->dump));
        app->password_entered = false;
        app->password = 0;
        scene_manager_next_scene(app->scene_manager, T5577AppSceneRead);
    } else {
        t5577_app_scene_start_open_saved(app);
    }

    return true;
}

void t5577_app_scene_start_on_exit(void* context) {
    T5577App* app = context;
    submenu_reset(app->submenu);
}
