#include "../lfrfid_i.h"
#include "../plugins/settings/lfrfid_settings_plugin.h"

#include <flipper_application/plugins/plugin_manager.h>
#include <loader/firmware_api/firmware_api.h>

#define TAG "LfRfidSettings"

#define LFRFID_SETTINGS_PLUGIN_PATH APP_ASSETS_PATH("plugins/lfrfid_settings.fal")

// The page itself lives in the plugin, so it is only in RAM while this scene is up.
static PluginManager* settings_plugin_manager;
static const LfRfidSettingsPlugin* settings_plugin;

// The list outlives the plugin and variable_item_list_set_enter_callback() rejects NULL, so the
// page's callback is replaced with this one - in the app image - before the plugin is unloaded.
static void lfrfid_scene_settings_enter_callback_none(void* context, uint32_t index) {
    UNUSED(context);
    UNUSED(index);
}

void lfrfid_scene_settings_write_targets_on_enter(void* context) {
    LfRfid* app = context;

    // Allocated on first use, not at startup: variable_item_list_alloc() starts a periodic 333 ms
    // timer it never stops, which would cap tickless idle for the whole session over a screen
    // most users never open. Freed in lfrfid_free(), never here - removing a view while it is
    // the current one latches an event loop stop.
    if(!app->variable_item_list) {
        app->variable_item_list = variable_item_list_alloc();
        view_dispatcher_add_view(
            app->view_dispatcher,
            LfRfidViewVariableItemList,
            variable_item_list_get_view(app->variable_item_list));
    }

    // Mapping the plugin is an SD read and the menu behind us is already gone. The loading view
    // also swallows input, so Back cannot pop the scene a failure is about to be reported on.
    view_dispatcher_show_loading(app->view_dispatcher);

    settings_plugin = NULL;
    settings_plugin_manager = plugin_manager_alloc(
        LFRFID_SETTINGS_PLUGIN_APP_ID, LFRFID_SETTINGS_PLUGIN_API_VERSION, firmware_api_interface);

    PluginManagerError error =
        plugin_manager_load_single(settings_plugin_manager, LFRFID_SETTINGS_PLUGIN_PATH);
    if(error == PluginManagerErrorNone) {
        settings_plugin = plugin_manager_get_ep(settings_plugin_manager, 0);
    }

    if(settings_plugin) {
        settings_plugin->write_targets->on_enter(app->variable_item_list);
        view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewVariableItemList);
        return;
    }

    FURI_LOG_E(TAG, "Failed to load %s (error %d)", LFRFID_SETTINGS_PLUGIN_PATH, error);
    plugin_manager_free(settings_plugin_manager);
    settings_plugin_manager = NULL;

    // The error code separates a missing file from a stale or foreign one, which need different
    // fixes. Only editing is lost either way - writes still honour whatever is in the file.
    lfrfid_text_store_set(app, "Settings plugin\nfailed to load\n(error %d)", error);
    popup_set_icon(app->popup, 83, 22, &I_WarningDolphinFlip_45x42);
    popup_set_header(app->popup, "Error", 64, 3, AlignCenter, AlignTop);
    popup_set_text(app->popup, app->text_store, 3, 19, AlignLeft, AlignTop);
    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewPopup);
}

bool lfrfid_scene_settings_write_targets_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;

    // Saving has to happen here rather than in on_exit: by the time a scene exits, the screen
    // already belongs to the next one, so a failure could not be shown to anyone.
    if(event.type == SceneManagerEventTypeBack && settings_plugin) {
        if(!settings_plugin->write_targets->on_save()) {
            dialog_message_show_storage_error(app->dialogs, "Cannot save\nsettings");
        }
    }

    return false;
}

void lfrfid_scene_settings_write_targets_on_exit(void* context) {
    LfRfid* app = context;

    if(app->variable_item_list) {
        // Both kinds of callback on the list point into the plugin: reset() drops the per-item
        // ones with the items, the enter callback has to be replaced by hand.
        variable_item_list_reset(app->variable_item_list);
        variable_item_list_set_enter_callback(
            app->variable_item_list, lfrfid_scene_settings_enter_callback_none, NULL);
    }
    popup_reset(app->popup);

    settings_plugin = NULL;
    if(settings_plugin_manager) {
        plugin_manager_free(settings_plugin_manager);
        settings_plugin_manager = NULL;
    }
}
