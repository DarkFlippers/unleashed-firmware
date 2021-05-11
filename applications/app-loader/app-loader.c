#include <furi.h>
#include <cli/cli.h>
#include "menu/menu.h"
#include "menu/menu_item.h"
#include "applications.h"
#include <assets_icons.h>
#include <api-hal.h>

typedef struct {
    FuriThread* thread;
    const FlipperApplication* current_app;
    Cli* cli;
} AppLoaderState;

static AppLoaderState state;

// TODO add mutex for contex

static void app_loader_menu_callback(void* _ctx) {
    furi_assert(_ctx);
    const FlipperApplication* flipper_app = (FlipperApplication*)_ctx;
    furi_assert(flipper_app->app);
    furi_assert(flipper_app->name);
    api_hal_power_insomnia_enter();

    state.current_app = flipper_app;

    furi_thread_set_name(state.thread, flipper_app->name);
    furi_thread_set_stack_size(state.thread, flipper_app->stack_size);
    furi_thread_set_callback(state.thread, flipper_app->app);
    furi_thread_start(state.thread);
}

static void app_loader_cli_callback(string_t args, void* _ctx) {
    furi_assert(_ctx);
    const FlipperApplication* flipper_app = (FlipperApplication*)_ctx;
    furi_assert(flipper_app->app);
    furi_assert(flipper_app->name);

    if(!(furi_thread_get_state(state.thread) == FuriThreadStateStopped)) {
        printf("Can't start, furi application is running");
        return;
    }

    printf("Starting furi application %s", flipper_app->name);
    api_hal_power_insomnia_enter();
    furi_thread_set_name(state.thread, flipper_app->name);
    furi_thread_set_stack_size(state.thread, flipper_app->stack_size);
    furi_thread_set_callback(state.thread, flipper_app->app);
    furi_thread_start(state.thread);
}

void app_loader_thread_state_callback(FuriThreadState state, void* context) {
    furi_assert(context);
    if(state == FuriThreadStateStopped) {
        api_hal_power_insomnia_exit();
    }
}

int32_t app_loader(void* p) {
    FURI_LOG_I("app-loader", "Starting");
    state.thread = furi_thread_alloc();
    furi_thread_set_state_context(state.thread, &state);
    furi_thread_set_state_callback(state.thread, app_loader_thread_state_callback);

    ValueMutex* menu_mutex = furi_record_open("menu");
    state.cli = furi_record_open("cli");

    // Main menu
    FURI_LOG_I("app-loader", "Building main menu");
    with_value_mutex(
        menu_mutex, (Menu * menu) {
            for(size_t i = 0; i < FLIPPER_APPS_COUNT; i++) {
                // Add menu item
                menu_item_add(
                    menu,
                    menu_item_alloc_function(
                        FLIPPER_APPS[i].name,
                        assets_icons_get(FLIPPER_APPS[i].icon),
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
    FURI_LOG_I("app-loader", "Building plugins menu");
    with_value_mutex(
        menu_mutex, (Menu * menu) {
            MenuItem* menu_plugins =
                menu_item_alloc_menu("Plugins", assets_icons_get(A_Plugins_14));

            for(size_t i = 0; i < FLIPPER_PLUGINS_COUNT; i++) {
                // Add menu item
                menu_item_subitem_add(
                    menu_plugins,
                    menu_item_alloc_function(
                        FLIPPER_PLUGINS[i].name,
                        assets_icons_get(FLIPPER_PLUGINS[i].icon),
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
    FURI_LOG_I("app-loader", "Building debug menu");
    with_value_mutex(
        menu_mutex, (Menu * menu) {
            MenuItem* menu_debug =
                menu_item_alloc_menu("Debug tools", assets_icons_get(A_Settings_14));

            for(size_t i = 0; i < FLIPPER_DEBUG_APPS_COUNT; i++) {
                // Add menu item
                menu_item_subitem_add(
                    menu_debug,
                    menu_item_alloc_function(
                        FLIPPER_DEBUG_APPS[i].name,
                        assets_icons_get(FLIPPER_DEBUG_APPS[i].icon),
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

    // Call on start hooks
    for(size_t i = 0; i < FLIPPER_ON_SYSTEM_START_COUNT; i++) {
        (*FLIPPER_ON_SYSTEM_START[i])();
    }

    FURI_LOG_I("app-loader", "Started");

    while(1) {
        osThreadSuspend(osThreadGetId());
    }

    return 0;
}
