#include "loader_i.h"

static Loader* loader_instance = NULL;

static void loader_menu_callback(void* _ctx) {
    const FlipperApplication* flipper_app = _ctx;

    furi_assert(flipper_app->app);
    furi_assert(flipper_app->name);

    if(!loader_lock(loader_instance)) return;

    if(furi_thread_get_state(loader_instance->thread) != FuriThreadStateStopped) {
        FURI_LOG_E(
            LOADER_LOG_TAG, "Can't start app. %s is running", loader_instance->current_app->name);
        return;
    }
    api_hal_power_insomnia_enter();
    loader_instance->current_app = flipper_app;

    FURI_LOG_I(
        LOADER_LOG_TAG, "Starting furi application: %s", loader_instance->current_app->name);
    furi_thread_set_name(loader_instance->thread, flipper_app->name);
    furi_thread_set_stack_size(loader_instance->thread, flipper_app->stack_size);
    furi_thread_set_context(loader_instance->thread, NULL);
    furi_thread_set_callback(loader_instance->thread, flipper_app->app);
    furi_thread_start(loader_instance->thread);
}

static void loader_cli_callback(Cli* cli, string_t args, void* _ctx) {
    furi_assert(_ctx);
    const FlipperApplication* flipper_app = (FlipperApplication*)_ctx;
    furi_assert(flipper_app->app);
    furi_assert(flipper_app->name);

    if(furi_thread_get_state(loader_instance->thread) != FuriThreadStateStopped) {
        printf("Can't start, furi application is running");
        return;
    }

    loader_instance->lock_semaphore++;
    api_hal_power_insomnia_enter();
    loader_instance->current_app = flipper_app;
    printf("Starting furi application %s", loader_instance->current_app->name);
    furi_thread_set_name(loader_instance->thread, flipper_app->name);
    furi_thread_set_stack_size(loader_instance->thread, flipper_app->stack_size);
    furi_thread_set_callback(loader_instance->thread, flipper_app->app);
    furi_thread_start(loader_instance->thread);
}

bool loader_start(Loader* instance, const char* name, const char* args) {
    furi_assert(name);

    const FlipperApplication* flipper_app = NULL;
    // Search for application
    for(size_t i = 0; i < FLIPPER_APPS_COUNT; i++) {
        if(strcmp(FLIPPER_APPS[i].name, name) == 0) {
            flipper_app = &FLIPPER_APPS[i];
            break;
        }
    }

    if(!flipper_app) {
        FURI_LOG_E(LOADER_LOG_TAG, "Can't find application with name %s", name);
        return false;
    }

    loader_lock(instance);

    if(furi_thread_get_state(instance->thread) != FuriThreadStateStopped) {
        FURI_LOG_E(LOADER_LOG_TAG, "Can't start app. %s is running", instance->current_app->name);
        return false;
    }

    instance->current_app = flipper_app;
    if(args) {
        string_set_str(instance->args, args);
        string_strim(instance->args);
        FURI_LOG_I(LOADER_LOG_TAG, "Start %s app with args: %s", name, args);
    } else {
        string_clean(instance->args);
        FURI_LOG_I(LOADER_LOG_TAG, "Start %s app with no args", name);
    }

    furi_thread_set_name(instance->thread, flipper_app->name);
    furi_thread_set_stack_size(instance->thread, flipper_app->stack_size);
    furi_thread_set_context(instance->thread, (void*)string_get_cstr(instance->args));
    furi_thread_set_callback(instance->thread, flipper_app->app);

    return furi_thread_start(instance->thread);
}

bool loader_lock(Loader* instance) {
    bool ret = false;
    furi_check(osMutexAcquire(instance->mutex, osWaitForever) == osOK);
    if(instance->lock_semaphore == 0) {
        instance->lock_semaphore++;
        ret = true;
    }
    furi_check(osMutexRelease(instance->mutex) == osOK);
    return ret;
}

void loader_unlock(Loader* instance) {
    furi_check(osMutexAcquire(instance->mutex, osWaitForever) == osOK);
    furi_check(instance->lock_semaphore > 0);
    instance->lock_semaphore--;
    furi_check(osMutexRelease(instance->mutex) == osOK);
}

static void loader_thread_state_callback(FuriThreadState thread_state, void* context) {
    furi_assert(context);

    Loader* instance = context;

    if(thread_state == FuriThreadStateRunning) {
        instance->free_heap_size = xPortGetFreeHeapSize();
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
        int heap_diff = instance->free_heap_size - xPortGetFreeHeapSize();
        FURI_LOG_I(
            LOADER_LOG_TAG,
            "Application thread stopped. Heap allocation balance: %d. Thread allocation balance: %d.",
            heap_diff,
            furi_thread_get_heap_size(instance->thread));
        api_hal_power_insomnia_exit();
        loader_unlock(instance);
    }
}

static Loader* loader_alloc() {
    Loader* instance = furi_alloc(sizeof(Loader));

    instance->thread = furi_thread_alloc();
    furi_thread_enable_heap_trace(instance->thread);
    furi_thread_set_state_context(instance->thread, instance);
    furi_thread_set_state_callback(instance->thread, loader_thread_state_callback);

    string_init(instance->args);

    instance->mutex = osMutexNew(NULL);

    instance->menu_vm = furi_record_open("menu");

    instance->cli = furi_record_open("cli");

    return instance;
}

static void loader_free(Loader* instance) {
    furi_assert(instance);

    furi_record_close("cli");

    furi_record_close("menu");

    osMutexDelete(instance->mutex);

    string_clear(instance->args);

    furi_thread_free(instance->thread);

    free(instance);
}

static void loader_build_menu() {
    FURI_LOG_I(LOADER_LOG_TAG, "Building main menu");
    with_value_mutex(
        loader_instance->menu_vm, (Menu * menu) {
            for(size_t i = 0; i < FLIPPER_APPS_COUNT; i++) {
                // Add menu item
                menu_item_add(
                    menu,
                    menu_item_alloc_function(
                        FLIPPER_APPS[i].name,
                        FLIPPER_APPS[i].icon ? icon_animation_alloc(FLIPPER_APPS[i].icon) : NULL,
                        loader_menu_callback,
                        (void*)&FLIPPER_APPS[i]));

                // Add cli command
                string_t cli_name;
                string_init_set_str(cli_name, "app_");
                string_cat_str(cli_name, FLIPPER_APPS[i].name);
                cli_add_command(
                    loader_instance->cli,
                    string_get_cstr(cli_name),
                    CliCommandFlagDefault,
                    loader_cli_callback,
                    (void*)&FLIPPER_APPS[i]);
                string_clear(cli_name);
            }
        });

    FURI_LOG_I(LOADER_LOG_TAG, "Building plugins menu");
    with_value_mutex(
        loader_instance->menu_vm, (Menu * menu) {
            MenuItem* menu_plugins =
                menu_item_alloc_menu("Plugins", icon_animation_alloc(&A_Plugins_14));

            for(size_t i = 0; i < FLIPPER_PLUGINS_COUNT; i++) {
                // Add menu item
                menu_item_subitem_add(
                    menu_plugins,
                    menu_item_alloc_function(
                        FLIPPER_PLUGINS[i].name,
                        FLIPPER_PLUGINS[i].icon ? icon_animation_alloc(FLIPPER_PLUGINS[i].icon) :
                                                  NULL,
                        loader_menu_callback,
                        (void*)&FLIPPER_PLUGINS[i]));

                // Add cli command
                string_t cli_name;
                string_init_set_str(cli_name, "app_");
                string_cat_str(cli_name, FLIPPER_PLUGINS[i].name);
                cli_add_command(
                    loader_instance->cli,
                    string_get_cstr(cli_name),
                    CliCommandFlagDefault,
                    loader_cli_callback,
                    (void*)&FLIPPER_PLUGINS[i]);
                string_clear(cli_name);
            }

            menu_item_add(menu, menu_plugins);
        });

    FURI_LOG_I(LOADER_LOG_TAG, "Building debug menu");
    with_value_mutex(
        loader_instance->menu_vm, (Menu * menu) {
            MenuItem* menu_debug =
                menu_item_alloc_menu("Debug tools", icon_animation_alloc(&A_Settings_14));

            for(size_t i = 0; i < FLIPPER_DEBUG_APPS_COUNT; i++) {
                // Add menu item
                menu_item_subitem_add(
                    menu_debug,
                    menu_item_alloc_function(
                        FLIPPER_DEBUG_APPS[i].name,
                        FLIPPER_DEBUG_APPS[i].icon ?
                            icon_animation_alloc(FLIPPER_DEBUG_APPS[i].icon) :
                            NULL,
                        loader_menu_callback,
                        (void*)&FLIPPER_DEBUG_APPS[i]));

                // Add cli command
                string_t cli_name;
                string_init_set_str(cli_name, "app_");
                string_cat_str(cli_name, FLIPPER_DEBUG_APPS[i].name);
                cli_add_command(
                    loader_instance->cli,
                    string_get_cstr(cli_name),
                    CliCommandFlagDefault,
                    loader_cli_callback,
                    (void*)&FLIPPER_DEBUG_APPS[i]);
                string_clear(cli_name);
            }

            menu_item_add(menu, menu_debug);
        });

    FURI_LOG_I(LOADER_LOG_TAG, "Building settings menu");
    with_value_mutex(
        loader_instance->menu_vm, (Menu * menu) {
            MenuItem* menu_debug =
                menu_item_alloc_menu("Settings", icon_animation_alloc(&A_Settings_14));

            for(size_t i = 0; i < FLIPPER_SETTINGS_APPS_COUNT; i++) {
                // Add menu item
                menu_item_subitem_add(
                    menu_debug,
                    menu_item_alloc_function(
                        FLIPPER_SETTINGS_APPS[i].name,
                        FLIPPER_SETTINGS_APPS[i].icon ?
                            icon_animation_alloc(FLIPPER_SETTINGS_APPS[i].icon) :
                            NULL,
                        loader_menu_callback,
                        (void*)&FLIPPER_SETTINGS_APPS[i]));
            }

            menu_item_add(menu, menu_debug);
        });
}

int32_t loader_srv(void* p) {
    FURI_LOG_I(LOADER_LOG_TAG, "Starting");

    loader_instance = loader_alloc();

    loader_build_menu();

    // Call on start hooks
    for(size_t i = 0; i < FLIPPER_ON_SYSTEM_START_COUNT; i++) {
        (*FLIPPER_ON_SYSTEM_START[i])();
    }

    FURI_LOG_I(LOADER_LOG_TAG, "Started");

    furi_record_create("loader", loader_instance);

    while(1) {
        osThreadSuspend(osThreadGetId());
    }

    loader_free(loader_instance);

    return 0;
}
