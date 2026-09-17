#include "../ibutton_i.h"
#include "../plugins/settings/ibutton_settings_plugin.h"

#include <flipper_application/plugins/plugin_manager.h>
#include <loader/firmware_api/firmware_api.h>

#define TAG "IButtonSettings"

#define IBUTTON_SETTINGS_PLUGIN_PATH APP_ASSETS_PATH("plugins/ibutton_settings.fal")

// The page itself lives in the plugin, so it is only in RAM while this scene is up.
static PluginManager* settings_plugin_manager;
static const iButtonSettingsPlugin* settings_plugin;

// The list outlives the plugin and variable_item_list_set_enter_callback() rejects NULL, so the
// page's callback is replaced with this one - in the app image - before the plugin is unloaded.
static void ibutton_scene_settings_enter_callback_none(void* context, uint32_t index) {
    UNUSED(context);
    UNUSED(index);
}

void ibutton_scene_settings_write_targets_on_enter(void* context) {
    iButton* ibutton = context;

    // Mapping the plugin is an SD read and the menu behind us is already gone. Leaving the
    // loading view also resets the input queue, so a Back pressed during the read cannot pop
    // the scene a failure is about to be reported on.
    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewLoading);

    settings_plugin = NULL;
    settings_plugin_manager = plugin_manager_alloc(
        IBUTTON_SETTINGS_PLUGIN_APP_ID,
        IBUTTON_SETTINGS_PLUGIN_API_VERSION,
        firmware_api_interface);

    PluginManagerError error =
        plugin_manager_load_single(settings_plugin_manager, IBUTTON_SETTINGS_PLUGIN_PATH);
    if(error == PluginManagerErrorNone) {
        settings_plugin = plugin_manager_get_ep(settings_plugin_manager, 0);
    }

    if(settings_plugin) {
        // Only now, and only once per session: variable_item_list_alloc() starts a periodic
        // 333 ms timer it never stops, which would cap tickless idle for the rest of the
        // session. Freed in ibutton_free(), never here - removing a view while it is the
        // current one latches an event loop stop.
        if(!ibutton->variable_item_list) {
            ibutton->variable_item_list = variable_item_list_alloc();
            view_dispatcher_add_view(
                ibutton->view_dispatcher,
                iButtonViewVariableItemList,
                variable_item_list_get_view(ibutton->variable_item_list));
        }

        settings_plugin->write_targets->on_enter(ibutton->variable_item_list);
        view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewVariableItemList);
        return;
    }

    FURI_LOG_E(TAG, "Failed to load %s (error %d)", IBUTTON_SETTINGS_PLUGIN_PATH, error);
    plugin_manager_free(settings_plugin_manager);
    settings_plugin_manager = NULL;

    // The code is a breadcrumb for the log, not a diagnosis: plugin_manager collapses missing,
    // corrupt and API-mismatched into one value. Only editing is lost either way - writes still
    // honour whatever is in the settings file.
    snprintf(
        ibutton->text_store,
        IBUTTON_TEXT_STORE_SIZE,
        "Settings plugin\nfailed to load\nerror %d",
        error);
    popup_set_icon(ibutton->popup, 83, 22, &I_WarningDolphinFlip_45x42);
    popup_set_header(ibutton->popup, "Error", 64, 3, AlignCenter, AlignTop);
    popup_set_text(ibutton->popup, ibutton->text_store, 3, 19, AlignLeft, AlignTop);
    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewPopup);
}

bool ibutton_scene_settings_write_targets_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);

    return false;
}

void ibutton_scene_settings_write_targets_on_exit(void* context) {
    iButton* ibutton = context;

    if(settings_plugin) {
        if(!settings_plugin->write_targets->on_save()) {
            FURI_LOG_E(TAG, "Failed to save write targets");
        }
        settings_plugin = NULL;
    }

    if(ibutton->variable_item_list) {
        variable_item_list_set_enter_callback(
            ibutton->variable_item_list, ibutton_scene_settings_enter_callback_none, ibutton);
        variable_item_list_reset(ibutton->variable_item_list);
    }

    if(settings_plugin_manager) {
        plugin_manager_free(settings_plugin_manager);
        settings_plugin_manager = NULL;
    }

    popup_reset(ibutton->popup);
}
