#include "app-loader.h"
#include "api-hal-delay.h"

#define APP_LOADER_TAG "app_loader"

typedef struct {
    FuriThread* thread;
    const FlipperApplication* current_app;
    string_t args;
    Cli* cli;
    size_t free_heap_size;
} AppLoaderState;

static AppLoaderState state;

// TODO add mutex for contex

static void app_loader_menu_callback(void* _ctx) {
    furi_assert(_ctx);
    const FlipperApplication* flipper_app = (FlipperApplication*)_ctx;
    furi_assert(flipper_app->app);
    furi_assert(flipper_app->name);

    if(furi_thread_get_state(state.thread) != FuriThreadStateStopped) {
        FURI_LOG_E(APP_LOADER_TAG, "Can't start app. %s is running", state.current_app->name);
        return;
    }
    api_hal_power_insomnia_enter();
    state.current_app = flipper_app;
    FURI_LOG_I(APP_LOADER_TAG, "Starting furi application: %s", state.current_app->name);
    furi_thread_set_name(state.thread, flipper_app->name);
    furi_thread_set_stack_size(state.thread, flipper_app->stack_size);
    furi_thread_set_context(state.thread, NULL);
    furi_thread_set_callback(state.thread, flipper_app->app);
    furi_thread_start(state.thread);
}

static void app_loader_cli_callback(Cli* cli, string_t args, void* _ctx) {
    furi_assert(_ctx);
    const FlipperApplication* flipper_app = (FlipperApplication*)_ctx;
    furi_assert(flipper_app->app);
    furi_assert(flipper_app->name);

    if(furi_thread_get_state(state.thread) != FuriThreadStateStopped) {
        printf("Can't start, furi application is running");
        return;
    }

    api_hal_power_insomnia_enter();
    state.current_app = flipper_app;
    printf("Starting furi application %s", state.current_app->name);
    furi_thread_set_name(state.thread, flipper_app->name);
    furi_thread_set_stack_size(state.thread, flipper_app->stack_size);
    furi_thread_set_callback(state.thread, flipper_app->app);
    furi_thread_start(state.thread);
}

bool app_loader_start(const char* name, const char* args) {
    furi_assert(name);

    const FlipperApplication* flipper_app = NULL;
    // Search for application
    for(size_t i = 0; i < FLIPPER_APPS_COUNT; i++) {
        if(!strcmp(FLIPPER_APPS[i].name, name)) {
            flipper_app = &FLIPPER_APPS[i];
            break;
        }
    }
    if(!flipper_app) {
        FURI_LOG_E(APP_LOADER_TAG, "Can't find application with name %s", name);
        return false;
    }
    if(furi_thread_get_state(state.thread) != FuriThreadStateStopped) {
        FURI_LOG_E(APP_LOADER_TAG, "Can't start app. %s is running", state.current_app->name);
        return false;
    }
    state.current_app = flipper_app;
    if(args) {
        string_set_str(state.args, args);
        string_strim(state.args);
        FURI_LOG_I(APP_LOADER_TAG, "Start %s app with args: %s", name, args);
    } else {
        string_clean(state.args);
        FURI_LOG_I(APP_LOADER_TAG, "Start %s app with no args", name);
    }
    furi_thread_set_name(state.thread, flipper_app->name);
    furi_thread_set_stack_size(state.thread, flipper_app->stack_size);
    furi_thread_set_context(state.thread, (void*)string_get_cstr(state.args));
    furi_thread_set_callback(state.thread, flipper_app->app);
    return furi_thread_start(state.thread);
}

void app_loader_thread_state_callback(FuriThreadState thread_state, void* context) {
    furi_assert(context);

    AppLoaderState* state = context;

    if(thread_state == FuriThreadStateRunning) {
        state->free_heap_size = xPortGetFreeHeapSize();
    } else if(thread_state == FuriThreadStateStopped) {
        /*
         * Current Leak Sanitizer assumes that memory is allocated and freed
         * inside one thread. Timers are allocated in one task, but freed in
         * Timer-Task thread, and xTimerDelete() just put command to queue.
         * To avoid some bad cases there are few fixes:
         * 1) delay for Timer to process commands
         * 2) there are 'heap diff' which shows difference in heap before task
         * started and after task completed. In process of leakage monitoring
         * both values should be taken into account.
         */
        delay(20);
        int heap_diff = state->free_heap_size - xPortGetFreeHeapSize();
        FURI_LOG_I(
            APP_LOADER_TAG,
            "Application thread stopped. Heap allocation balance: %d. Thread allocation balance: %d.",
            heap_diff,
            furi_thread_get_heap_size(state->thread));
        api_hal_power_insomnia_exit();
    }
}

int32_t app_loader(void* p) {
    FURI_LOG_I(APP_LOADER_TAG, "Starting");
    state.thread = furi_thread_alloc();
    furi_thread_enable_heap_trace(state.thread);
    furi_thread_set_state_context(state.thread, &state);
    furi_thread_set_state_callback(state.thread, app_loader_thread_state_callback);
    string_init(state.args);

    ValueMutex* menu_mutex = furi_record_open("menu");
    state.cli = furi_record_open("cli");

    // Main menu
    FURI_LOG_I(APP_LOADER_TAG, "Building main menu");
    with_value_mutex(
        menu_mutex, (Menu * menu) {
            for(size_t i = 0; i < FLIPPER_APPS_COUNT; i++) {
                // Add menu item
                menu_item_add(
                    menu,
                    menu_item_alloc_function(
                        FLIPPER_APPS[i].name,
                        icon_animation_alloc(FLIPPER_APPS[i].icon),
                        app_loader_menu_callback,
                        (void*)&FLIPPER_APPS[i]));

                // Add cli command
                string_t cli_name;
                string_init_set_str(cli_name, "app_");
                string_cat_str(cli_name, FLIPPER_APPS[i].name);
                cli_add_command(
                    state.cli,
                    string_get_cstr(cli_name),
                    app_loader_cli_callback,
                    (void*)&FLIPPER_APPS[i]);
                string_clear(cli_name);
            }
        });

    // Plugins
    FURI_LOG_I(APP_LOADER_TAG, "Building plugins menu");
    with_value_mutex(
        menu_mutex, (Menu * menu) {
            MenuItem* menu_plugins =
                menu_item_alloc_menu("Plugins", icon_animation_alloc(&A_Plugins_14));

            for(size_t i = 0; i < FLIPPER_PLUGINS_COUNT; i++) {
                // Add menu item
                menu_item_subitem_add(
                    menu_plugins,
                    menu_item_alloc_function(
                        FLIPPER_PLUGINS[i].name,
                        icon_animation_alloc(FLIPPER_PLUGINS[i].icon),
                        app_loader_menu_callback,
                        (void*)&FLIPPER_PLUGINS[i]));

                // Add cli command
                string_t cli_name;
                string_init_set_str(cli_name, "app_");
                string_cat_str(cli_name, FLIPPER_PLUGINS[i].name);
                cli_add_command(
                    state.cli,
                    string_get_cstr(cli_name),
                    app_loader_cli_callback,
                    (void*)&FLIPPER_PLUGINS[i]);
                string_clear(cli_name);
            }

            menu_item_add(menu, menu_plugins);
        });

    // Debug
    FURI_LOG_I(APP_LOADER_TAG, "Building debug menu");
    with_value_mutex(
        menu_mutex, (Menu * menu) {
            MenuItem* menu_debug =
                menu_item_alloc_menu("Debug tools", icon_animation_alloc(&A_Settings_14));

            for(size_t i = 0; i < FLIPPER_DEBUG_APPS_COUNT; i++) {
                // Add menu item
                menu_item_subitem_add(
                    menu_debug,
                    menu_item_alloc_function(
                        FLIPPER_DEBUG_APPS[i].name,
                        icon_animation_alloc(FLIPPER_DEBUG_APPS[i].icon),
                        app_loader_menu_callback,
                        (void*)&FLIPPER_DEBUG_APPS[i]));

                // Add cli command
                string_t cli_name;
                string_init_set_str(cli_name, "app_");
                string_cat_str(cli_name, FLIPPER_DEBUG_APPS[i].name);
                cli_add_command(
                    state.cli,
                    string_get_cstr(cli_name),
                    app_loader_cli_callback,
                    (void*)&FLIPPER_DEBUG_APPS[i]);
                string_clear(cli_name);
            }

            menu_item_add(menu, menu_debug);
        });

    // Settings
    FURI_LOG_I(APP_LOADER_TAG, "Building settings menu");
    with_value_mutex(
        menu_mutex, (Menu * menu) {
            MenuItem* menu_debug =
                menu_item_alloc_menu("Settings", icon_animation_alloc(&A_Settings_14));

            for(size_t i = 0; i < FLIPPER_SETTINGS_APPS_COUNT; i++) {
                // Add menu item
                menu_item_subitem_add(
                    menu_debug,
                    menu_item_alloc_function(
                        FLIPPER_SETTINGS_APPS[i].name,
                        icon_animation_alloc(FLIPPER_SETTINGS_APPS[i].icon),
                        app_loader_menu_callback,
                        (void*)&FLIPPER_SETTINGS_APPS[i]));
            }

            menu_item_add(menu, menu_debug);
        });

    // Call on start hooks
    for(size_t i = 0; i < FLIPPER_ON_SYSTEM_START_COUNT; i++) {
        (*FLIPPER_ON_SYSTEM_START[i])();
    }

    FURI_LOG_I(APP_LOADER_TAG, "Started");

    while(1) {
        osThreadSuspend(osThreadGetId());
    }

    return 0;
}
