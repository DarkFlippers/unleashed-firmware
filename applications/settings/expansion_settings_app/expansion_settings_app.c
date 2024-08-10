#include "expansion_settings_app.h"

static const char* const expansion_uart_text[] = {
    "USART",
    "LPUART",
    "None",
};

static void expansion_settings_app_uart_changed(VariableItem* item) {
    ExpansionSettingsApp* app = variable_item_get_context(item);
    const uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, expansion_uart_text[index]);
    app->settings.uart_index = index;

    if(index < FuriHalSerialIdMax) {
        expansion_set_listen_serial(app->expansion, index);
    } else {
        expansion_disable(app->expansion);
    }
}

static uint32_t expansion_settings_app_exit(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

static ExpansionSettingsApp* expansion_settings_app_alloc(void) {
    ExpansionSettingsApp* app = malloc(sizeof(ExpansionSettingsApp));

    expansion_settings_load(&app->settings);

    app->gui = furi_record_open(RECORD_GUI);
    app->expansion = furi_record_open(RECORD_EXPANSION);

    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    app->var_item_list = variable_item_list_alloc();

    VariableItem* item;
    uint8_t value_index;

    item = variable_item_list_add(
        app->var_item_list,
        "Listen UART",
        COUNT_OF(expansion_uart_text),
        expansion_settings_app_uart_changed,
        app);
    value_index = app->settings.uart_index;
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, expansion_uart_text[value_index]);

    view_set_previous_callback(
        variable_item_list_get_view(app->var_item_list), expansion_settings_app_exit);
    view_dispatcher_add_view(
        app->view_dispatcher,
        ExpansionSettingsViewVarItemList,
        variable_item_list_get_view(app->var_item_list));

    view_dispatcher_switch_to_view(app->view_dispatcher, ExpansionSettingsViewVarItemList);

    return app;
}

static void expansion_settings_app_free(ExpansionSettingsApp* app) {
    furi_assert(app);

    expansion_settings_save(&app->settings);

    view_dispatcher_remove_view(app->view_dispatcher, ExpansionSettingsViewVarItemList);
    variable_item_list_free(app->var_item_list);
    view_dispatcher_free(app->view_dispatcher);

    furi_record_close(RECORD_EXPANSION);
    furi_record_close(RECORD_GUI);

    free(app);
}

int32_t expansion_settings_app(void* p) {
    UNUSED(p);
    ExpansionSettingsApp* app = expansion_settings_app_alloc();
    view_dispatcher_run(app->view_dispatcher);
    expansion_settings_app_free(app);
    return 0;
}
