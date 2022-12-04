#include "wifi_deauther_app_i.h"

#include <furi_hal_power.h>
#include <furi.h>
#include <furi_hal.h>

static bool wifi_deauther_app_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    WifideautherApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool wifi_deauther_app_back_event_callback(void* context) {
    furi_assert(context);
    WifideautherApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static void wifi_deauther_app_tick_event_callback(void* context) {
    furi_assert(context);
    WifideautherApp* app = context;
    scene_manager_handle_tick_event(app->scene_manager);
}

WifideautherApp* wifi_deauther_app_alloc() {
    WifideautherApp* app = malloc(sizeof(WifideautherApp));

    app->gui = furi_record_open(RECORD_GUI);

    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&wifi_deauther_scene_handlers, app);
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, wifi_deauther_app_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, wifi_deauther_app_back_event_callback);
    view_dispatcher_set_tick_event_callback(
        app->view_dispatcher, wifi_deauther_app_tick_event_callback, 100);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    app->var_item_list = variable_item_list_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        WifideautherAppViewVarItemList,
        variable_item_list_get_view(app->var_item_list));

    for(int i = 0; i < NUM_MENU_ITEMS; ++i) {
        app->selected_option_index[i] = 0;
    }

    app->text_box = text_box_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, WifideautherAppViewConsoleOutput, text_box_get_view(app->text_box));
    app->text_box_store = furi_string_alloc();
    furi_string_reserve(app->text_box_store, WIFI_deauther_TEXT_BOX_STORE_SIZE);

    app->text_input = text_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, WifideautherAppViewTextInput, text_input_get_view(app->text_input));

    scene_manager_next_scene(app->scene_manager, WifideautherSceneStart);

    return app;
}

void wifi_deauther_app_free(WifideautherApp* app) {
    furi_assert(app);

    // Views
    view_dispatcher_remove_view(app->view_dispatcher, WifideautherAppViewVarItemList);
    view_dispatcher_remove_view(app->view_dispatcher, WifideautherAppViewConsoleOutput);
    view_dispatcher_remove_view(app->view_dispatcher, WifideautherAppViewTextInput);
    text_box_free(app->text_box);
    furi_string_free(app->text_box_store);
    text_input_free(app->text_input);

    // View dispatcher
    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);

    wifi_deauther_uart_free(app->uart);

    // Close records
    furi_record_close(RECORD_GUI);

    free(app);
}

int32_t wifi_deauther_app(void* p) {
    furi_hal_power_enable_otg();
    furi_delay_ms(600);
    UNUSED(p);
    WifideautherApp* wifi_deauther_app = wifi_deauther_app_alloc();

    wifi_deauther_app->uart = wifi_deauther_uart_init(wifi_deauther_app);

    view_dispatcher_run(wifi_deauther_app->view_dispatcher);

    wifi_deauther_app_free(wifi_deauther_app);
    furi_hal_power_disable_otg();

    return 0;
}
