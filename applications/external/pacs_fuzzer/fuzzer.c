#include "fuzzer_i.h"
#include "helpers/fuzzer_types.h"

static bool fuzzer_app_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    PacsFuzzerApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool fuzzer_app_back_event_callback(void* context) {
    furi_assert(context);
    PacsFuzzerApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static void fuzzer_app_tick_event_callback(void* context) {
    furi_assert(context);
    PacsFuzzerApp* app = context;
    scene_manager_handle_tick_event(app->scene_manager);
}

PacsFuzzerApp* fuzzer_app_alloc() {
    PacsFuzzerApp* app = malloc(sizeof(PacsFuzzerApp));

    app->fuzzer_state.menu_index = 0;
    app->fuzzer_state.proto_index = 0;

    // GUI
    app->gui = furi_record_open(RECORD_GUI);

    // View Dispatcher
    app->view_dispatcher = view_dispatcher_alloc();

    // Main view
    app->main_view = fuzzer_view_main_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, FuzzerViewIDMain, fuzzer_view_main_get_view(app->main_view));

    // Attack view
    app->attack_view = fuzzer_view_attack_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, FuzzerViewIDAttack, fuzzer_view_attack_get_view(app->attack_view));

    app->scene_manager = scene_manager_alloc(&fuzzer_scene_handlers, app);
    view_dispatcher_enable_queue(app->view_dispatcher);

    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, fuzzer_app_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, fuzzer_app_back_event_callback);
    view_dispatcher_set_tick_event_callback(
        app->view_dispatcher, fuzzer_app_tick_event_callback, 100);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    scene_manager_next_scene(app->scene_manager, FuzzerSceneMain);

    return app;
}

void fuzzer_app_free(PacsFuzzerApp* app) {
    furi_assert(app);

    // Remote view
    view_dispatcher_remove_view(app->view_dispatcher, FuzzerViewIDMain);
    fuzzer_view_main_free(app->main_view);

    // Attack view
    view_dispatcher_remove_view(app->view_dispatcher, FuzzerViewIDAttack);
    fuzzer_view_attack_free(app->attack_view);

    scene_manager_free(app->scene_manager);
    view_dispatcher_free(app->view_dispatcher);

    // Close records
    furi_record_close(RECORD_GUI);

    free(app);
}

int32_t fuzzer_start(void* p) {
    UNUSED(p);
    PacsFuzzerApp* fuzzer_app = fuzzer_app_alloc();

    view_dispatcher_run(fuzzer_app->view_dispatcher);

    fuzzer_app_free(fuzzer_app);
    return 0;
}