#include "flipper_v2.h"
#include <gui/gui.h>
#include "menu/menu.h"
#include "applications.h"
#include <assets_icons.h>

typedef struct {
    FuriApp* handler;
    Widget* widget;
    const FlipperStartupApp* current_app;
} AppLoaderState;

typedef struct {
    AppLoaderState* state;
    const FlipperStartupApp* app;
} AppLoaderContext;

// TODO add mutex for contex

static void render_callback(CanvasApi* canvas, void* _ctx) {
    AppLoaderState* ctx = (AppLoaderState*)_ctx;

    canvas->clear(canvas);
    canvas->set_color(canvas, ColorBlack);
    canvas->set_font(canvas, FontPrimary);
    canvas->draw_str(canvas, 2, 32, ctx->current_app->name);

    canvas->set_font(canvas, FontSecondary);
    canvas->draw_str(canvas, 2, 44, "press back to exit");
}

static void input_callback(InputEvent* input_event, void* _ctx) {
    AppLoaderState* ctx = (AppLoaderState*)_ctx;

    if(input_event->state && input_event->input == InputBack) {
        furiac_kill(ctx->handler);
        widget_enabled_set(ctx->widget, false);
    }
}

static void handle_menu(void* _ctx) {
    AppLoaderContext* ctx = (AppLoaderContext*)_ctx;

    widget_enabled_set(ctx->state->widget, true);

    // TODO how to call this?
    // furiac_wait_libs(&FLIPPER_STARTUP[i].libs);

    ctx->state->current_app = ctx->app;
    ctx->state->handler = furiac_start(ctx->app->app, ctx->app->name, NULL);
}

void app_loader(void* p) {
    osThreadId_t self_id = osThreadGetId();
    furi_check(self_id);

    AppLoaderState state;
    state.handler = NULL;

    state.widget = widget_alloc();
    widget_enabled_set(state.widget, false);
    widget_draw_callback_set(state.widget, render_callback, &state);
    widget_input_callback_set(state.widget, input_callback, &state);

    ValueMutex* menu_mutex = furi_open("menu");
    if(menu_mutex == NULL) {
        printf("menu is not available\n");
        furiac_exit(NULL);
    }

    // Open GUI and register widget
    GuiApi* gui = furi_open("gui");
    if(gui == NULL) {
        printf("gui is not available\n");
        furiac_exit(NULL);
    }
    gui->add_widget(gui, state.widget, GuiLayerFullscreen);

    {
        Menu* menu = acquire_mutex_block(menu_mutex);

        // FURI startup
        const size_t flipper_app_count = sizeof(FLIPPER_APPS) / sizeof(FLIPPER_APPS[0]);

        for(size_t i = 0; i < flipper_app_count; i++) {
            AppLoaderContext* ctx = furi_alloc(sizeof(AppLoaderContext));
            ctx->state = &state;
            ctx->app = &FLIPPER_APPS[i];

            menu_item_add(
                menu,
                menu_item_alloc_function(
                    FLIPPER_APPS[i].name, assets_icons_get(A_Infrared_14), handle_menu, ctx));
        }

        /*
        menu_item_add(menu, menu_item_alloc_function("Sub 1 gHz", NULL, NULL, NULL));
        menu_item_add(menu, menu_item_alloc_function("125 kHz RFID", NULL, NULL, NULL));
        menu_item_add(menu, menu_item_alloc_function("Infrared", NULL, NULL, NULL));
        menu_item_add(menu, menu_item_alloc_function("I-Button", NULL, NULL, NULL));
        menu_item_add(menu, menu_item_alloc_function("USB", NULL, NULL, NULL));
        menu_item_add(menu, menu_item_alloc_function("Bluetooth", NULL, NULL, NULL));
        menu_item_add(menu, menu_item_alloc_function("GPIO / HW", NULL, NULL, NULL));
        menu_item_add(menu, menu_item_alloc_function("U2F", NULL, NULL, NULL));
        menu_item_add(menu, menu_item_alloc_function("Tamagotchi", NULL, NULL, NULL));
        menu_item_add(menu, menu_item_alloc_function("Plugins", NULL, NULL, NULL));
        */

        release_mutex(menu_mutex, menu);
    }

    printf("[app loader] start\n");

    osThreadSuspend(self_id);
}