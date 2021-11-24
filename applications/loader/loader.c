#include "loader/loader.h"
#include "loader_i.h"

#define TAG "LoaderSrv"

#define LOADER_THREAD_FLAG_SHOW_MENU (1 << 0)
#define LOADER_THREAD_FLAG_ALL (LOADER_THREAD_FLAG_SHOW_MENU)

static Loader* loader_instance = NULL;

static void loader_menu_callback(void* _ctx, uint32_t index) {
    const FlipperApplication* flipper_app = _ctx;

    furi_assert(flipper_app->app);
    furi_assert(flipper_app->name);

    if(!loader_lock(loader_instance)) return;

    if(furi_thread_get_state(loader_instance->thread) != FuriThreadStateStopped) {
        FURI_LOG_E(TAG, "Can't start app. %s is running", loader_instance->current_app->name);
        return;
    }
    furi_hal_power_insomnia_enter();
    loader_instance->current_app = flipper_app;

    FURI_LOG_I(TAG, "Starting furi application: %s", loader_instance->current_app->name);
    furi_thread_set_name(loader_instance->thread, flipper_app->name);
    furi_thread_set_stack_size(loader_instance->thread, flipper_app->stack_size);
    furi_thread_set_context(loader_instance->thread, NULL);
    furi_thread_set_callback(loader_instance->thread, flipper_app->app);
    furi_thread_start(loader_instance->thread);
}

static void loader_submenu_callback(void* context, uint32_t index) {
    uint32_t view_id = (uint32_t)context;
    view_dispatcher_switch_to_view(loader_instance->view_dispatcher, view_id);
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
    furi_hal_power_insomnia_enter();
    loader_instance->current_app = flipper_app;
    printf("Starting furi application %s", loader_instance->current_app->name);
    furi_thread_set_name(loader_instance->thread, flipper_app->name);
    furi_thread_set_stack_size(loader_instance->thread, flipper_app->stack_size);
    furi_thread_set_callback(loader_instance->thread, flipper_app->app);
    furi_thread_start(loader_instance->thread);
}

LoaderStatus loader_start(Loader* instance, const char* name, const char* args) {
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
        for(size_t i = 0; i < FLIPPER_DEBUG_APPS_COUNT; i++) {
            if(strcmp(FLIPPER_DEBUG_APPS[i].name, name) == 0) {
                flipper_app = &FLIPPER_DEBUG_APPS[i];
                break;
            }
        }
    }

    if(!flipper_app) {
        FURI_LOG_E(TAG, "Can't find application with name %s", name);
        return LoaderStatusErrorUnknownApp;
    }

    bool locked = loader_lock(instance);

    if(!locked || (furi_thread_get_state(instance->thread) != FuriThreadStateStopped)) {
        FURI_LOG_E(TAG, "Can't start app. %s is running", instance->current_app->name);
        /* no need to call loader_unlock() - it is called as soon as application stops */
        return LoaderStatusErrorAppStarted;
    }

    instance->current_app = flipper_app;
    void* thread_args = NULL;
    if(args) {
        string_set_str(instance->args, args);
        string_strim(instance->args);
        thread_args = (void*)string_get_cstr(instance->args);
        FURI_LOG_I(TAG, "Start %s app with args: %s", name, args);
    } else {
        string_reset(instance->args);
        FURI_LOG_I(TAG, "Start %s app with no args", name);
    }

    furi_thread_set_name(instance->thread, flipper_app->name);
    furi_thread_set_stack_size(instance->thread, flipper_app->stack_size);
    furi_thread_set_context(instance->thread, thread_args);
    furi_thread_set_callback(instance->thread, flipper_app->app);

    bool thread_started = furi_thread_start(instance->thread);
    return thread_started ? LoaderStatusOk : LoaderStatusErrorInternal;
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

bool loader_is_locked(Loader* instance) {
    return (instance->lock_semaphore > 0);
}

static void loader_thread_state_callback(FuriThreadState thread_state, void* context) {
    furi_assert(context);

    Loader* instance = context;

    if(thread_state == FuriThreadStateRunning) {
        instance->free_heap_size = memmgr_get_free_heap();
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
        int heap_diff = instance->free_heap_size - memmgr_get_free_heap();
        FURI_LOG_I(
            TAG,
            "Application thread stopped. Heap allocation balance: %d. Thread allocation balance: %d.",
            heap_diff,
            furi_thread_get_heap_size(instance->thread));
        furi_hal_power_insomnia_exit();
        loader_unlock(instance);
    }
}

static uint32_t loader_hide_menu(void* context) {
    return VIEW_NONE;
}

static uint32_t loader_back_to_primary_menu(void* context) {
    furi_assert(context);
    Submenu* submenu = context;
    submenu_set_selected_item(submenu, 0);
    return LoaderMenuViewPrimary;
}

static Loader* loader_alloc() {
    Loader* instance = furi_alloc(sizeof(Loader));

    instance->thread = furi_thread_alloc();
    furi_thread_enable_heap_trace(instance->thread);
    furi_thread_set_state_context(instance->thread, instance);
    furi_thread_set_state_callback(instance->thread, loader_thread_state_callback);

    string_init(instance->args);

    instance->mutex = osMutexNew(NULL);

    instance->cli = furi_record_open("cli");

    instance->loader_thread = osThreadGetId();

    // Gui
    instance->gui = furi_record_open("gui");
    instance->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_attach_to_gui(
        instance->view_dispatcher, instance->gui, ViewDispatcherTypeFullscreen);
    // Primary menu
    instance->primary_menu = menu_alloc();
    view_set_previous_callback(menu_get_view(instance->primary_menu), loader_hide_menu);
    view_dispatcher_add_view(
        instance->view_dispatcher, LoaderMenuViewPrimary, menu_get_view(instance->primary_menu));
    // Plugins menu
    instance->plugins_menu = submenu_alloc();
    view_set_context(submenu_get_view(instance->plugins_menu), instance->plugins_menu);
    view_set_previous_callback(
        submenu_get_view(instance->plugins_menu), loader_back_to_primary_menu);
    view_dispatcher_add_view(
        instance->view_dispatcher,
        LoaderMenuViewPlugins,
        submenu_get_view(instance->plugins_menu));
    // Debug menu
    instance->debug_menu = submenu_alloc();
    view_set_context(submenu_get_view(instance->debug_menu), instance->debug_menu);
    view_set_previous_callback(
        submenu_get_view(instance->debug_menu), loader_back_to_primary_menu);
    view_dispatcher_add_view(
        instance->view_dispatcher, LoaderMenuViewDebug, submenu_get_view(instance->debug_menu));
    // Settings menu
    instance->settings_menu = submenu_alloc();
    view_set_context(submenu_get_view(instance->settings_menu), instance->settings_menu);
    view_set_previous_callback(
        submenu_get_view(instance->settings_menu), loader_back_to_primary_menu);
    view_dispatcher_add_view(
        instance->view_dispatcher,
        LoaderMenuViewSettings,
        submenu_get_view(instance->settings_menu));

    view_dispatcher_enable_queue(instance->view_dispatcher);

    return instance;
}

static void loader_free(Loader* instance) {
    furi_assert(instance);

    furi_record_close("cli");

    osMutexDelete(instance->mutex);

    string_clear(instance->args);

    furi_thread_free(instance->thread);

    menu_free(loader_instance->primary_menu);
    view_dispatcher_remove_view(loader_instance->view_dispatcher, LoaderMenuViewPrimary);
    submenu_free(loader_instance->plugins_menu);
    view_dispatcher_remove_view(loader_instance->view_dispatcher, LoaderMenuViewPlugins);
    submenu_free(loader_instance->debug_menu);
    view_dispatcher_remove_view(loader_instance->view_dispatcher, LoaderMenuViewDebug);
    submenu_free(loader_instance->settings_menu);
    view_dispatcher_remove_view(loader_instance->view_dispatcher, LoaderMenuViewSettings);
    view_dispatcher_free(loader_instance->view_dispatcher);

    furi_record_close("gui");

    free(instance);
    instance = NULL;
}

static void loader_add_cli_command(FlipperApplication* app) {
    string_t cli_name;
    string_init_printf(cli_name, "app_%s", app->name);
    cli_add_command(
        loader_instance->cli,
        string_get_cstr(cli_name),
        CliCommandFlagDefault,
        loader_cli_callback,
        app);
    string_clear(cli_name);
}

static void loader_build_menu() {
    FURI_LOG_I(TAG, "Building main menu");
    size_t i;
    for(i = 0; i < FLIPPER_APPS_COUNT; i++) {
        loader_add_cli_command((FlipperApplication*)&FLIPPER_APPS[i]);
        menu_add_item(
            loader_instance->primary_menu,
            FLIPPER_APPS[i].name,
            FLIPPER_APPS[i].icon ? icon_animation_alloc(FLIPPER_APPS[i].icon) : NULL,
            i,
            loader_menu_callback,
            (void*)&FLIPPER_APPS[i]);
    }
    menu_add_item(
        loader_instance->primary_menu,
        "Plugins",
        icon_animation_alloc(&A_Plugins_14),
        i++,
        loader_submenu_callback,
        (void*)LoaderMenuViewPlugins);
    menu_add_item(
        loader_instance->primary_menu,
        "Debug tools",
        icon_animation_alloc(&A_Debug_14),
        i++,
        loader_submenu_callback,
        (void*)LoaderMenuViewDebug);
    menu_add_item(
        loader_instance->primary_menu,
        "Settings",
        icon_animation_alloc(&A_Settings_14),
        i++,
        loader_submenu_callback,
        (void*)LoaderMenuViewSettings);

    FURI_LOG_I(TAG, "Building plugins menu");
    for(i = 0; i < FLIPPER_PLUGINS_COUNT; i++) {
        loader_add_cli_command((FlipperApplication*)&FLIPPER_PLUGINS[i]);
        submenu_add_item(
            loader_instance->plugins_menu,
            FLIPPER_PLUGINS[i].name,
            i,
            loader_menu_callback,
            (void*)&FLIPPER_PLUGINS[i]);
    }

    FURI_LOG_I(TAG, "Building debug menu");
    for(i = 0; i < FLIPPER_DEBUG_APPS_COUNT; i++) {
        loader_add_cli_command((FlipperApplication*)&FLIPPER_DEBUG_APPS[i]);
        submenu_add_item(
            loader_instance->debug_menu,
            FLIPPER_DEBUG_APPS[i].name,
            i,
            loader_menu_callback,
            (void*)&FLIPPER_DEBUG_APPS[i]);
    }

    FURI_LOG_I(TAG, "Building settings menu");
    for(i = 0; i < FLIPPER_SETTINGS_APPS_COUNT; i++) {
        submenu_add_item(
            loader_instance->settings_menu,
            FLIPPER_SETTINGS_APPS[i].name,
            i,
            loader_menu_callback,
            (void*)&FLIPPER_SETTINGS_APPS[i]);
    }
}

void loader_show_menu() {
    furi_assert(loader_instance);
    osThreadFlagsSet(loader_instance->loader_thread, LOADER_THREAD_FLAG_SHOW_MENU);
}

int32_t loader_srv(void* p) {
    FURI_LOG_I(TAG, "Starting");

    loader_instance = loader_alloc();

    loader_build_menu();

    // Call on start hooks
    for(size_t i = 0; i < FLIPPER_ON_SYSTEM_START_COUNT; i++) {
        FLIPPER_ON_SYSTEM_START[i]();
    }

    FURI_LOG_I(TAG, "Started");

    furi_record_create("loader", loader_instance);

#ifdef LOADER_AUTOSTART
    loader_start(loader_instance, LOADER_AUTOSTART, NULL);
#endif

    while(1) {
        uint32_t flags = osThreadFlagsWait(LOADER_THREAD_FLAG_ALL, osFlagsWaitAny, osWaitForever);
        if(flags & LOADER_THREAD_FLAG_SHOW_MENU) {
            menu_set_selected_item(loader_instance->primary_menu, 0);
            view_dispatcher_switch_to_view(
                loader_instance->view_dispatcher, LoaderMenuViewPrimary);
            view_dispatcher_run(loader_instance->view_dispatcher);
        }
    }

    loader_free(loader_instance);

    return 0;
}
