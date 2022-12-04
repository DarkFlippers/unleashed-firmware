#include "../power_settings_app.h"
#include <lib/toolbox/value_index.h>

#define SHUTDOWN_IDLE_DELAY_COUNT 9
#define SCENE_EVENT_SELECT_SHUTDOWN_IDLE_DELAY 0

const char* const shutdown_idle_delay_text[SHUTDOWN_IDLE_DELAY_COUNT] =
    {"OFF", "15min", "30min", "1h", "2h", "6h", "12h", "24h", "48h"};

const uint32_t shutdown_idle_delay_value[SHUTDOWN_IDLE_DELAY_COUNT] =
    {0, 900000, 1800000, 3600000, 7200000, 21600000, 43200000, 86400000, 172800000};

static void power_settings_scene_shutodwn_idle_callback(void* context, uint32_t index) {
    PowerSettingsApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

static void power_settings_scene_start_shutdown_idle_delay_changed(VariableItem* item) {
    PowerSettingsApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, shutdown_idle_delay_text[index]);
    app->shutdown_idle_delay_ms = shutdown_idle_delay_value[index];
}

void power_settings_scene_shutdown_idle_on_enter(void* context) {
    PowerSettingsApp* app = context;
    LOAD_POWER_SETTINGS(&app->shutdown_idle_delay_ms);
    VariableItemList* variable_item_list = app->variable_item_list;
    VariableItem* item;
    uint8_t value_index;

    item = variable_item_list_add(
        variable_item_list,
        "Set Time",
        SHUTDOWN_IDLE_DELAY_COUNT,
        power_settings_scene_start_shutdown_idle_delay_changed,
        app);

    variable_item_list_set_enter_callback(
        variable_item_list, power_settings_scene_shutodwn_idle_callback, app);

    value_index = value_index_uint32(
        app->shutdown_idle_delay_ms, shutdown_idle_delay_value, SHUTDOWN_IDLE_DELAY_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, shutdown_idle_delay_text[value_index]);

    view_dispatcher_switch_to_view(app->view_dispatcher, PowerSettingsAppViewVariableItemList);
}

bool power_settings_scene_shutdown_idle_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SCENE_EVENT_SELECT_SHUTDOWN_IDLE_DELAY) {
            consumed = true;
        }
    }
    return consumed;
}

void power_settings_scene_shutdown_idle_on_exit(void* context) {
    PowerSettingsApp* app = context;
    SAVE_POWER_SETTINGS(&app->shutdown_idle_delay_ms);
    furi_pubsub_publish(app->settings_events, &app->shutdown_idle_delay_ms);
    variable_item_list_reset(app->variable_item_list);
}
