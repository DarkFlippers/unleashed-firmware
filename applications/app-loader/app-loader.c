#include <furi.h>
#include <cli/cli.h>
#include <gui/gui.h>
#include "menu/menu.h"
#include "menu/menu_item.h"
#include "applications.h"
#include <assets_icons.h>
#include <api-hal.h>

typedef struct {
    FuriThread* thread;
    ViewPort* view_port;
    const FlipperApplication* current_app;
    Cli* cli;
    Gui* gui;
} AppLoaderState;

typedef struct {
    AppLoaderState* state;
    const FlipperApplication* app;
} AppLoaderContext;

// TODO add mutex for contex

static void app_loader_render_callback(Canvas* canvas, void* _ctx) {
    AppLoaderState* ctx = (AppLoaderState*)_ctx;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 32, ctx->current_app->name);

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 44, "press back to exit");
}

static void app_loader_input_callback(InputEvent* input_event, void* _ctx) {
    AppLoaderState* ctx = (AppLoaderState*)_ctx;

    if(input_event->type == InputTypeShort && input_event->key == InputKeyBack) {
        furi_thread_terminate(ctx->thread);
    }
}

static void app_loader_menu_callback(void* _ctx) {
    AppLoaderContext* ctx = (AppLoaderContext*)_ctx;

    if(ctx->app->app == NULL) return;

    view_port_enabled_set(ctx->state->view_port, true);

    api_hal_power_insomnia_enter();

    ctx->state->current_app = ctx->app;

    furi_thread_set_name(ctx->state->thread, ctx->app->name);
    furi_thread_set_stack_size(ctx->state->thread, ctx->app->stack_size);
    furi_thread_set_callback(ctx->state->thread, ctx->app->app);
    furi_thread_start(ctx->state->thread);
}

static void app_loader_cli_callback(string_t args, void* _ctx) {
    AppLoaderContext* ctx = (AppLoaderContext*)_ctx;

    if(ctx->app->app == NULL) return;

    printf("Starting furi application\r\n");

    api_hal_power_insomnia_enter();

    furi_thread_set_name(ctx->state->thread, ctx->app->name);
    furi_thread_set_stack_size(ctx->state->thread, ctx->app->stack_size);
    furi_thread_set_callback(ctx->state->thread, ctx->app->app);
    furi_thread_start(ctx->state->thread);

    printf("Press any key to kill application");

    cli_getc(ctx->state->cli);

    furi_thread_terminate(ctx->state->thread);
}

void app_loader_thread_state_callback(FuriThreadState state, void* context) {
    furi_assert(context);
    AppLoaderState* app_loader_state = context;
    if(state == FuriThreadStateStopped) {
        view_port_enabled_set(app_loader_state->view_port, false);
        api_hal_power_insomnia_exit();
    }
}

int32_t app_loader(void* p) {
    AppLoaderState state;
    state.thread = furi_thread_alloc();
    furi_thread_set_state_context(state.thread, &state);
    furi_thread_set_state_callback(state.thread, app_loader_thread_state_callback);

    state.view_port = view_port_alloc();
    view_port_enabled_set(state.view_port, false);
    view_port_draw_callback_set(state.view_port, app_loader_render_callback, &state);
    view_port_input_callback_set(state.view_port, app_loader_input_callback, &state);

    ValueMutex* menu_mutex = furi_record_open("menu");
    state.cli = furi_record_open("cli");
    state.gui = furi_record_open("gui");

    gui_add_view_port(state.gui, state.view_port, GuiLayerFullscreen);

    // Main menu
    with_value_mutex(
        menu_mutex, (Menu * menu) {
            for(size_t i = 0; i < FLIPPER_APPS_COUNT; i++) {
                AppLoaderContext* ctx = furi_alloc(sizeof(AppLoaderContext));
                ctx->state = &state;
                ctx->app = &FLIPPER_APPS[i];

                menu_item_add(
                    menu,
                    menu_item_alloc_function(
                        FLIPPER_APPS[i].name,
                        assets_icons_get(FLIPPER_APPS[i].icon),
                        app_loader_menu_callback,
                        ctx));

                // Add cli command
                string_t cli_name;
                string_init_set_str(cli_name, "app_");
                string_cat_str(cli_name, FLIPPER_APPS[i].name);
                cli_add_command(
                    state.cli, string_get_cstr(cli_name), app_loader_cli_callback, ctx);
                string_clear(cli_name);
            }
        });

    with_value_mutex(
        menu_mutex, (Menu * menu) {
            menu_item_add(
                menu, menu_item_alloc_function("U2F", assets_icons_get(A_U2F_14), NULL, NULL));
            menu_item_add(
                menu,
                menu_item_alloc_function(
                    "File Manager", assets_icons_get(A_FileManager_14), NULL, NULL));
            menu_item_add(
                menu, menu_item_alloc_function("Games", assets_icons_get(A_Games_14), NULL, NULL));
            menu_item_add(
                menu,
                menu_item_alloc_function("Passport", assets_icons_get(A_Passport_14), NULL, NULL));
            menu_item_add(
                menu,
                menu_item_alloc_function("Settings", assets_icons_get(A_Settings_14), NULL, NULL));
        });

    // plugins
    with_value_mutex(
        menu_mutex, (Menu * menu) {
            MenuItem* menu_plugins =
                menu_item_alloc_menu("Plugins", assets_icons_get(A_Plugins_14));

            for(size_t i = 0; i < FLIPPER_PLUGINS_COUNT; i++) {
                AppLoaderContext* ctx = furi_alloc(sizeof(AppLoaderContext));
                ctx->state = &state;
                ctx->app = &FLIPPER_PLUGINS[i];

                menu_item_subitem_add(
                    menu_plugins,
                    menu_item_alloc_function(
                        FLIPPER_PLUGINS[i].name,
                        assets_icons_get(FLIPPER_PLUGINS[i].icon),
                        app_loader_menu_callback,
                        ctx));

                // Add cli command
                string_t cli_name;
                string_init_set_str(cli_name, "app_");
                string_cat_str(cli_name, FLIPPER_PLUGINS[i].name);
                cli_add_command(
                    state.cli, string_get_cstr(cli_name), app_loader_cli_callback, ctx);
                string_clear(cli_name);
            }

            menu_item_add(menu, menu_plugins);
        });

    printf("[app loader] start\r\n");

    while(1) {
        osThreadSuspend(osThreadGetId());
    }

    return 0;
}
