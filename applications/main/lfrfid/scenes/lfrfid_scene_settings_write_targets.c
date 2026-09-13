#include "../lfrfid_i.h"
#include "../plugins/settings/lfrfid_settings_plugin.h"

#include <flipper_application/plugins/plugin_manager.h>
#include <loader/firmware_api/firmware_api.h>

#define TAG "LfRfidSettings"

#define LFRFID_SETTINGS_PLUGIN_PATH APP_ASSETS_PATH("plugins/lfrfid_settings.fal")

// The page itself lives in the plugin, so it is only in RAM while this scene is up.
static PluginManager* settings_plugin_manager;
static const LfRfidSettingsPlugin* settings_plugin;

static LfRfidSettingsPluginCtx lfrfid_scene_settings_plugin_ctx(LfRfid* app) {
    return (LfRfidSettingsPluginCtx){
        .list = app->variable_item_list,
        .view_dispatcher = app->view_dispatcher,
        .view_id = LfRfidViewVariableItemList,
    };
}

void lfrfid_scene_settings_write_targets_on_enter(void* context) {
    LfRfid* app = context;

    settings_plugin = NULL;
    settings_plugin_manager = plugin_manager_alloc(
        LFRFID_SETTINGS_PLUGIN_APP_ID, LFRFID_SETTINGS_PLUGIN_API_VERSION, firmware_api_interface);

    PluginManagerError error =
        plugin_manager_load_single(settings_plugin_manager, LFRFID_SETTINGS_PLUGIN_PATH);
    if(error == PluginManagerErrorNone) {
        settings_plugin = plugin_manager_get_ep(settings_plugin_manager, 0);
    }

    if(settings_plugin) {
        LfRfidSettingsPluginCtx ctx = lfrfid_scene_settings_plugin_ctx(app);
        settings_plugin->write_targets->on_enter(&ctx);
        return;
    }

    FURI_LOG_E(TAG, "Failed to load %s (error %d)", LFRFID_SETTINGS_PLUGIN_PATH, error);
    plugin_manager_free(settings_plugin_manager);
    settings_plugin_manager = NULL;

    // Only editing is lost - writes still honour whatever is in the settings file.
    popup_set_icon(app->popup, 83, 22, &I_WarningDolphinFlip_45x42);
    popup_set_header(app->popup, "Error", 64, 3, AlignCenter, AlignTop);
    popup_set_text(app->popup, "Settings plugin\nis missing", 3, 19, AlignLeft, AlignTop);
    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewPopup);
}

bool lfrfid_scene_settings_write_targets_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void lfrfid_scene_settings_write_targets_on_exit(void* context) {
    LfRfid* app = context;

    if(settings_plugin) {
        LfRfidSettingsPluginCtx ctx = lfrfid_scene_settings_plugin_ctx(app);
        settings_plugin->write_targets->on_exit(&ctx);
        settings_plugin = NULL;
    }

    // Before unloading: the list still holds callbacks that point into the plugin.
    variable_item_list_reset(app->variable_item_list);
    popup_reset(app->popup);

    if(settings_plugin_manager) {
        plugin_manager_free(settings_plugin_manager);
        settings_plugin_manager = NULL;
    }
}
