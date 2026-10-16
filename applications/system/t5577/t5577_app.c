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

uint8_t t5577_app_blocks_read(const T5577Dump* dump) {
    uint8_t count = 0;

    for(uint8_t page = 0; page <= 1; page++) {
        for(uint8_t block = 0; block < t5577_page_blocks(page); block++) {
            if(t5577_dump_was_read(dump, page, block)) count++;
        }
    }

    return count;
}

void t5577_app_describe(T5577App* app) {
    const T5577Dump* dump = &app->dump;

    furi_string_reset(app->text);

    for(uint8_t page = 0; page <= 1; page++) {
        furi_string_cat_printf(app->text, "Page %u\n", page);

        for(uint8_t block = 0; block < t5577_page_blocks(page); block++) {
            if(t5577_dump_was_read(dump, page, block)) {
                furi_string_cat_printf(app->text, "%u: %08lX\n", block, dump->block[page][block]);
            } else {
                // Not "00000000": a block that was never read is not a block holding zero, and
                // the difference decides whether a restore would write it.
                furi_string_cat_printf(app->text, "%u: not read\n", block);
            }
        }
    }

    if(dump->has_password) {
        furi_string_cat_printf(app->text, "Password\n%08lX\n", dump->password);
    }
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

    app->text_box = text_box_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, T5577AppViewTextBox, text_box_get_view(app->text_box));

    app->text_input = text_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, T5577AppViewTextInput, text_input_get_view(app->text_input));

    app->byte_input = byte_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, T5577AppViewByteInput, byte_input_get_view(app->byte_input));

    app->widget = widget_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, T5577AppViewWidget, widget_get_view(app->widget));

    // One shot, re-armed by whichever scene wants another step. A periodic timer kept queueing
    // ticks behind steps that take most of a second, and the backlog outlived the scene.
    app->step_timer = furi_timer_alloc(t5577_app_step_timer_callback, FuriTimerTypeOnce, app);

    app->path = furi_string_alloc();
    app->text = furi_string_alloc();

    return app;
}

static void t5577_app_free(T5577App* app) {
    furi_timer_free(app->step_timer);

    view_dispatcher_remove_view(app->view_dispatcher, T5577AppViewPopup);
    view_dispatcher_remove_view(app->view_dispatcher, T5577AppViewSubmenu);
    view_dispatcher_remove_view(app->view_dispatcher, T5577AppViewTextBox);
    view_dispatcher_remove_view(app->view_dispatcher, T5577AppViewTextInput);
    view_dispatcher_remove_view(app->view_dispatcher, T5577AppViewByteInput);
    view_dispatcher_remove_view(app->view_dispatcher, T5577AppViewWidget);

    popup_free(app->popup);
    submenu_free(app->submenu);
    text_box_free(app->text_box);
    text_input_free(app->text_input);
    byte_input_free(app->byte_input);
    widget_free(app->widget);

    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);

    furi_string_free(app->path);
    furi_string_free(app->text);

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
