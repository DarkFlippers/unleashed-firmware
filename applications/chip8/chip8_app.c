#include "chip8_app_i.h"
#include <furi.h>

static bool chip8_app_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    Chip8App* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool chip8_app_back_event_callback(void* context) {
    furi_assert(context);
    Chip8App* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static void chip8_app_tick_event_callback(void* context) {
    furi_assert(context);
    Chip8App* app = context;
    scene_manager_handle_tick_event(app->scene_manager);
}

uint8_t** chip8_backup_screen_alloc() {
    FURI_LOG_I("chip8", "chip8_backup_screen_alloc start");

    uint8_t** backup_screen = malloc(SCREEN_HEIGHT * sizeof(size_t));
    for(int i = 0; i < SCREEN_HEIGHT; i++) {
        backup_screen[i] = malloc(SCREEN_WIDTH * sizeof(uint8_t));
        for(int j = 0; j < SCREEN_WIDTH; j++) {
            backup_screen[i][j] = 0;
        }
    }

    FURI_LOG_I("chip8", "chip8_backup_screen_alloc end");
    return backup_screen;
}

Chip8App* chip8_app_alloc() {
    Chip8App* app = malloc(sizeof(Chip8App));

    app->gui = furi_record_open("gui");
    app->dialogs = furi_record_open("dialogs");
    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&chip8_scene_handlers, app);

    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    view_dispatcher_set_tick_event_callback(
        app->view_dispatcher, chip8_app_tick_event_callback, 100);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, chip8_app_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, chip8_app_back_event_callback);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    app->chip8_view = chip8_alloc();
    app->backup_screen = chip8_backup_screen_alloc();
    view_dispatcher_add_view(app->view_dispatcher, Chip8WorkView, chip8_get_view(app->chip8_view));

    scene_manager_next_scene(app->scene_manager, Chip8FileSelectView);

    return app;
}

void chip8_app_free(Chip8App* app) {
    FURI_LOG_I("CHIP8", "chip8_app_free started");
    furi_assert(app);

    view_dispatcher_remove_view(app->view_dispatcher, Chip8FileSelectView);
    view_dispatcher_remove_view(app->view_dispatcher, Chip8WorkView);
    chip8_free(app->chip8_view);

    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);

    furi_record_close("gui");
    furi_record_close("dialogs");

    free(app);
}

int32_t chip8_app(void* p) {
    Chip8App* chip8_app = chip8_app_alloc();

    view_dispatcher_run(chip8_app->view_dispatcher);
    chip8_app_free(chip8_app);
    return 0;
}