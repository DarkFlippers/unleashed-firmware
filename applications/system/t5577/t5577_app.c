#include "t5577_app.h"

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

// Data blocks first and block 0 last, because block 0 decides how the chip talks: write it
// first and the rest of the sequence is verified against a configuration the chip is no longer
// using. Block 7 is the password store, and it has to be in place before block 0 turns
// protection on. Page 1 carries the traceability data the chip was made with and goes back as
// well, since a copy that leaves it behind is not the same tag; its block 0 repeats the
// configuration and is left alone. This is the order Proxmark3 restores in.
const T5577AppTarget t5577_app_restore_order[T5577_APP_RESTORE_BLOCKS] = {
    {0, 1},
    {0, 2},
    {0, 3},
    {0, 4},
    {0, 5},
    {0, 6},
    {0, 7},
    {1, 1},
    {1, 2},
    {1, 3},
    {0, 0},
};

bool t5577_app_restore_wanted(const T5577Dump* dump, uint8_t index) {
    const T5577AppTarget target = t5577_app_restore_order[index];

    if(!t5577_dump_was_read(dump, target.page, target.block)) return false;

    // A configuration of zero is not a configuration: a chip holding it answers nothing at all,
    // and there is no way back in without knowing that is what happened. It is what a dump taken
    // from a failed read holds, so a write skips it, the way a Proxmark3 restore does.
    if(target.page == 0 && target.block == 0 && dump->block[0][0] == 0) return false;

    return true;
}

uint8_t t5577_app_blocks_read(const T5577Dump* dump) {
    uint8_t count = 0;

    for(uint8_t page = 0; page <= 1; page++) {
        for(uint8_t block = 0; block < t5577_page_blocks(page); block++) {
            if(t5577_dump_was_read(dump, page, block)) count++;
        }
    }

    return count;
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

    app->byte_input = byte_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, T5577AppViewByteInput, byte_input_get_view(app->byte_input));

    app->widget = widget_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, T5577AppViewWidget, widget_get_view(app->widget));

    app->dump_view = t5577_dump_view_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, T5577AppViewDump, t5577_dump_view_get_view(app->dump_view));

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
    view_dispatcher_remove_view(app->view_dispatcher, T5577AppViewDump);
    view_dispatcher_remove_view(app->view_dispatcher, T5577AppViewWork);

    popup_free(app->popup);
    submenu_free(app->submenu);
    text_input_free(app->text_input);
    byte_input_free(app->byte_input);
    widget_free(app->widget);
    t5577_dump_view_free(app->dump_view);
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
    UNUSED(p);

    T5577App* app = t5577_app_alloc();

    if(!storage_simply_mkdir(app->storage, T5577_APP_FOLDER)) {
        FURI_LOG_W(TAG, "Could not make %s", T5577_APP_FOLDER);
    }

    scene_manager_next_scene(app->scene_manager, T5577AppSceneStart);
    view_dispatcher_run(app->view_dispatcher);

    t5577_app_free(app);

    return 0;
}
