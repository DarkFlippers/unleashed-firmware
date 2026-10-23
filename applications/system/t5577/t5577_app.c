#include "t5577_app.h"
#include <toolbox/path.h>

#define TAG "T5577App"

static bool t5577_app_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    T5577App* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool t5577_app_back_event_callback(void* context) {
    furi_assert(context);
    T5577App* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static void t5577_app_step_timer_callback(void* context) {
    furi_assert(context);
    T5577App* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, T5577AppCustomEventStep);
}

void t5577_app_step_later(T5577App* app, uint32_t delay_ms) {
    furi_timer_start(app->step_timer, furi_ms_to_ticks(delay_ms));
}

void t5577_app_step_stop(T5577App* app) {
    furi_timer_stop(app->step_timer);
}

// One dialog shape for everything the app has to say, and one for everything it has to ask.
void t5577_app_forget_password(T5577App* app) {
    app->password_entered = false;
    app->password = 0;
}

void t5577_app_say(T5577App* app, const char* header, const char* text) {
    DialogMessage* message = dialog_message_alloc();
    dialog_message_set_header(message, header, 64, 10, AlignCenter, AlignTop);
    dialog_message_set_text(message, text, 64, 32, AlignCenter, AlignTop);
    dialog_message_set_buttons(message, NULL, "OK", NULL);
    dialog_message_show(app->dialogs, message);
    dialog_message_free(message);
}

bool t5577_app_ask(T5577App* app, const char* header, const char* text, const char* yes) {
    DialogMessage* message = dialog_message_alloc();
    dialog_message_set_header(message, header, 64, 4, AlignCenter, AlignTop);
    dialog_message_set_text(message, text, 64, 20, AlignCenter, AlignTop);
    dialog_message_set_buttons(message, "Cancel", NULL, yes);
    const DialogMessageButton pressed = dialog_message_show(app->dialogs, message);
    dialog_message_free(message);

    return pressed == DialogMessageButtonRight;
}

void t5577_app_short_name(const T5577App* app, FuriString* name) {
    path_extract_filename(app->path, name, true);

    // Neither a header nor a line of dialog scrolls, so a long one is cut rather than overrun.
    if(furi_string_size(name) > T5577_APP_HEADER_MAX) {
        furi_string_left(name, T5577_APP_HEADER_MAX - 1);
        furi_string_push_back(name, '~');
    }
}

bool t5577_app_open_dump(T5577App* app) {
    const bool loaded = t5577_file_load(&app->dump, furi_string_get_cstr(app->path));
    // What the editor compares against, set either way: a failed load zeroes the dump, and the
    // last file's copy left here would make the empty one look edited.
    app->saved = app->dump;

    if(loaded) return true;

    // A file that is not there and a file that is not a dump are different things to be told,
    // and the first is the likely one on this path: the dump was listed in another app, and may
    // have been deleted or renamed, or the card pulled, between that list and this load.
    const bool gone = !storage_file_exists(app->storage, furi_string_get_cstr(app->path));
    const char* header = gone ? "Not found" : "Cannot open";
    const char* reason = gone ? "\nis no longer\non the card" :
                                "\nis not a T5577 dump\nthis version can read";

    // Named, because the file may have been picked in another app: this screen is then the
    // first thing the user sees of a press they made somewhere else.
    FuriString* text = furi_string_alloc();
    t5577_app_short_name(app, text);
    furi_string_cat_str(text, reason);

    t5577_app_say(app, header, furi_string_get_cstr(text));
    furi_string_free(text);

    return false;
}

void t5577_app_exit(T5577App* app) {
    scene_manager_stop(app->scene_manager);
    view_dispatcher_stop(app->view_dispatcher);
}

void t5577_app_test_backup_path(FuriString* path) {
    furi_string_printf(
        path, "%s/%s%s", T5577_APP_OWN_FOLDER, T5577_APP_TEST_BACKUP, T5577_FILE_EXTENSION);
}

static T5577App* t5577_app_alloc(void) {
    T5577App* app = malloc(sizeof(T5577App));

    app->gui = furi_record_open(RECORD_GUI);
    app->storage = furi_record_open(RECORD_STORAGE);
    app->dialogs = furi_record_open(RECORD_DIALOGS);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);

    app->scene_manager = scene_manager_alloc(&t5577_app_scene_handlers, app);
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, t5577_app_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, t5577_app_back_event_callback);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    app->popup = popup_alloc();
    view_dispatcher_add_view(app->view_dispatcher, T5577AppViewPopup, popup_get_view(app->popup));

    app->submenu = submenu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, T5577AppViewSubmenu, submenu_get_view(app->submenu));

    app->text_input = text_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, T5577AppViewTextInput, text_input_get_view(app->text_input));

    app->config_list = variable_item_list_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, T5577AppViewConfig, variable_item_list_get_view(app->config_list));

    app->byte_input = byte_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, T5577AppViewByteInput, byte_input_get_view(app->byte_input));

    app->widget = widget_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, T5577AppViewWidget, widget_get_view(app->widget));

    app->dump_view = t5577_dump_view_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, T5577AppViewDump, t5577_dump_view_get_view(app->dump_view));

    app->test_view = t5577_test_view_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, T5577AppViewTest, t5577_test_view_get_view(app->test_view));

    app->work_view = t5577_work_view_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, T5577AppViewWork, t5577_work_view_get_view(app->work_view));

    // One shot, re-armed by whichever scene wants another step. A periodic timer kept queueing
    // ticks behind steps that take most of a second, and the backlog outlived the scene.
    app->step_timer = furi_timer_alloc(t5577_app_step_timer_callback, FuriTimerTypeOnce, app);

    app->path = furi_string_alloc();

    return app;
}

static void t5577_app_free(T5577App* app) {
    furi_timer_free(app->step_timer);

    view_dispatcher_remove_view(app->view_dispatcher, T5577AppViewPopup);
    view_dispatcher_remove_view(app->view_dispatcher, T5577AppViewSubmenu);
    view_dispatcher_remove_view(app->view_dispatcher, T5577AppViewTextInput);
    view_dispatcher_remove_view(app->view_dispatcher, T5577AppViewByteInput);
    view_dispatcher_remove_view(app->view_dispatcher, T5577AppViewWidget);
    view_dispatcher_remove_view(app->view_dispatcher, T5577AppViewConfig);
    view_dispatcher_remove_view(app->view_dispatcher, T5577AppViewDump);
    view_dispatcher_remove_view(app->view_dispatcher, T5577AppViewTest);
    view_dispatcher_remove_view(app->view_dispatcher, T5577AppViewWork);

    popup_free(app->popup);
    submenu_free(app->submenu);
    text_input_free(app->text_input);
    byte_input_free(app->byte_input);
    widget_free(app->widget);
    variable_item_list_free(app->config_list);
    t5577_dump_view_free(app->dump_view);
    t5577_test_view_free(app->test_view);
    t5577_work_view_free(app->work_view);

    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);

    furi_string_free(app->path);

    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_DIALOGS);
    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_GUI);

    free(app);
}

int32_t t5577_app(void* p) {
    const char* args = p;

    T5577App* app = t5577_app_alloc();

    if(!storage_simply_mkdir(app->storage, T5577_APP_FOLDER)) {
        FURI_LOG_W(TAG, "Could not make %s", T5577_APP_FOLDER);
    }

    scene_manager_next_scene(app->scene_manager, T5577AppSceneStart);

    // A dump handed over by another app - the lfrfid saved list, or the archive - opens as if it
    // had been picked here. The open is queued rather than done now so that it happens with the
    // loop running: a file that will not load says so in a dialog, and a dialog wants a loop.
    if(args && strlen(args)) {
        furi_string_set(app->path, args);
        app->file_only = true;
        view_dispatcher_send_custom_event(app->view_dispatcher, T5577AppCustomEventOpen);
    }

    view_dispatcher_run(app->view_dispatcher);

    t5577_app_free(app);

    return 0;
}
