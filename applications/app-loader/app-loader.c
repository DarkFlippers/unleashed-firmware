#include <furi.h>
#include <cli/cli.h>
#include <gui/gui.h>
#include "menu/menu.h"
#include "menu/menu_item.h"
#include "applications.h"
#include <assets_icons.h>
#include <api-hal.h>

typedef struct {
    osThreadAttr_t app_thread_attr;
    osThreadId_t app_thread_id;
    Widget* widget;
    const FuriApplication* current_app;
} AppLoaderState;

typedef struct {
    AppLoaderState* state;
    const FuriApplication* app;
} AppLoaderContext;

// TODO add mutex for contex

static void render_callback(Canvas* canvas, void* _ctx) {
    AppLoaderState* ctx = (AppLoaderState*)_ctx;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 32, ctx->current_app->name);

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 44, "press back to exit");
}

static void input_callback(InputEvent* input_event, void* _ctx) {
    AppLoaderState* ctx = (AppLoaderState*)_ctx;

    if(input_event->state && input_event->input == InputBack) {
        osThreadTerminate(ctx->app_thread_id);
        widget_enabled_set(ctx->widget, false);
        api_hal_timebase_insomnia_exit();
    }
}

static void handle_menu(void* _ctx) {
    AppLoaderContext* ctx = (AppLoaderContext*)_ctx;

    if(ctx->app->app == NULL) return;

    widget_enabled_set(ctx->state->widget, true);

    // TODO how to call this?
    // furiac_wait_libs(&FLIPPER_STARTUP[i].libs);
    api_hal_timebase_insomnia_enter();

    ctx->state->current_app = ctx->app;
    ctx->state->app_thread_attr.name = ctx->app->name;
    ctx->state->app_thread_attr.attr_bits = osThreadDetached;
    ctx->state->app_thread_attr.cb_mem = NULL;
    ctx->state->app_thread_attr.cb_size = 0;
    ctx->state->app_thread_attr.stack_mem = NULL;
    ctx->state->app_thread_attr.stack_size = ctx->app->stack_size;
    ctx->state->app_thread_attr.priority = osPriorityNormal;
    ctx->state->app_thread_attr.tz_module = 0;
    ctx->state->app_thread_attr.reserved = 0;
    ctx->state->app_thread_id = osThreadNew(ctx->app->app, NULL, &ctx->state->app_thread_attr);
}

static void handle_cli(string_t args, void* _ctx) {
    AppLoaderContext* ctx = (AppLoaderContext*)_ctx;

    if(ctx->app->app == NULL) return;

    printf("Starting furi application\r\n");

    ctx->state->current_app = ctx->app;
    ctx->state->app_thread_attr.name = ctx->app->name;
    ctx->state->app_thread_attr.attr_bits = osThreadDetached;
    ctx->state->app_thread_attr.cb_mem = NULL;
    ctx->state->app_thread_attr.cb_size = 0;
    ctx->state->app_thread_attr.stack_mem = NULL;
    ctx->state->app_thread_attr.stack_size = ctx->app->stack_size;
    ctx->state->app_thread_attr.priority = osPriorityNormal;
    ctx->state->app_thread_attr.tz_module = 0;
    ctx->state->app_thread_attr.reserved = 0;
    ctx->state->app_thread_id = osThreadNew(ctx->app->app, NULL, &ctx->state->app_thread_attr);

    printf("Press any key to kill application");

    char c;
    cli_read(&c, 1);
    osThreadTerminate(ctx->state->app_thread_id);
}

void app_loader(void* p) {
    osThreadId_t self_id = osThreadGetId();
    furi_check(self_id);

    AppLoaderState state;
    state.app_thread_id = NULL;

    state.widget = widget_alloc();
    widget_enabled_set(state.widget, false);
    widget_draw_callback_set(state.widget, render_callback, &state);
    widget_input_callback_set(state.widget, input_callback, &state);

    ValueMutex* menu_mutex = furi_record_open("menu");
    Cli* cli = furi_record_open("cli");
    Gui* gui = furi_record_open("gui");

    gui_add_widget(gui, state.widget, GuiLayerFullscreen);

    // Main menu
    with_value_mutex(
        menu_mutex, (Menu * menu) {
            for(size_t i = 0; i < FLIPPER_APPS_size(); i++) {
                AppLoaderContext* ctx = furi_alloc(sizeof(AppLoaderContext));
                ctx->state = &state;
                ctx->app = &FLIPPER_APPS[i];

                menu_item_add(
                    menu,
                    menu_item_alloc_function(
                        FLIPPER_APPS[i].name,
                        assets_icons_get(FLIPPER_APPS[i].icon),
                        handle_menu,
                        ctx));

                // Add cli command
                string_t cli_name;
                string_init_set_str(cli_name, "app_");
                string_cat_str(cli_name, FLIPPER_APPS[i].name);
                cli_add_command(cli, string_get_cstr(cli_name), handle_cli, ctx);
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

            for(size_t i = 0; i < FLIPPER_PLUGINS_size(); i++) {
                AppLoaderContext* ctx = furi_alloc(sizeof(AppLoaderContext));
                ctx->state = &state;
                ctx->app = &FLIPPER_PLUGINS[i];

                menu_item_subitem_add(
                    menu_plugins,
                    menu_item_alloc_function(
                        FLIPPER_PLUGINS[i].name,
                        assets_icons_get(FLIPPER_PLUGINS[i].icon),
                        handle_menu,
                        ctx));

                // Add cli command
                string_t cli_name;
                string_init_set_str(cli_name, "app_");
                string_cat_str(cli_name, FLIPPER_PLUGINS[i].name);
                cli_add_command(cli, string_get_cstr(cli_name), handle_cli, ctx);
                string_clear(cli_name);
            }

            menu_item_add(menu, menu_plugins);
        });

    printf("[app loader] start\r\n");

    osThreadSuspend(self_id);
}
