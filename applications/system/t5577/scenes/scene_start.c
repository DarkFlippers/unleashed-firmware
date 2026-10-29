#include "../t5577_app.h"
#include <t5577_icons.h>
#include <dolphin/dolphin.h>

typedef enum {
    StartIndexRead,
    StartIndexTest,
    StartIndexSaved,
    StartIndexWipe,
    StartIndexClear,
    StartIndexSniff,
    StartIndexDict,
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
    submenu_add_item(
        app->submenu, "Sniff Password", StartIndexSniff, t5577_app_scene_start_callback, app);
    submenu_add_item(
        app->submenu, "Passwords", StartIndexDict, t5577_app_scene_start_callback, app);
    submenu_set_selected_item(
        app->submenu, scene_manager_get_scene_state(app->scene_manager, T5577AppSceneStart));

    view_dispatcher_switch_to_view(app->view_dispatcher, T5577AppViewSubmenu);
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

    // The dump another app handed over, opened now that the loop is running. It does not go
    // through the browser scene: the file was already chosen, and Back belongs on this menu
    // rather than in a list the user never opened. The submenu is moved as well as the scene
    // state, since it was drawn before this arrived.
    if(event.event == T5577AppCustomEventOpen) {
        scene_manager_set_scene_state(app->scene_manager, T5577AppSceneStart, StartIndexSaved);
        submenu_set_selected_item(app->submenu, StartIndexSaved);
        if(t5577_app_open_dump(app)) {
            scene_manager_next_scene(app->scene_manager, T5577AppSceneFileMenu);
        } else {
            // Nothing was opened, so there is nothing here the user came for.
            t5577_app_exit(app);
        }
        return true;
    }

    // Only the submenu's own indices get here as a choice; see the file menu for why.
    if(event.event > StartIndexDict) return false;

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
        scene_manager_next_scene(app->scene_manager, T5577AppSceneSelectDump);
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
    } else if(event.event == StartIndexDict) {
        // Nothing here touches a tag: it is the list Clear Password sweeps from, and the place
        // a word recovered by Sniff Password ends up.
        scene_manager_next_scene(app->scene_manager, T5577AppSceneDict);
    } else if(event.event == StartIndexSniff) {
        // Nothing is sent and nothing is energised here, so there is no warning to give and no
        // tag to put at risk - the one screen in this app that is safe to point at hardware
        // somebody else owns.
        scene_manager_next_scene(app->scene_manager, T5577AppSceneSniff);
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
