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

    // Mapping the plugin is an SD read and the menu behind us is already gone.
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
        // Version match does not promise the page is there.
        if(settings_plugin && !settings_plugin->write_targets) {
            FURI_LOG_E(TAG, "%s loaded but carries no page", IBUTTON_SETTINGS_PLUGIN_PATH);
            settings_plugin = NULL;
        }
    }

    if(settings_plugin) {
        // Allocated once per session, freed in ibutton_free() - see iButton::variable_item_list.
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

    // error only separates loader failure / wrong app id / wrong API version - missing and
    // corrupt read the same, and zero means the .fal loaded but carried no page. So keep the
    // code in the log and give the user the one remedy that covers every case. Only editing
    // is lost - writes still honour whatever is in the settings file.
    // Reset first: the Popup is app-wide and the success scenes leave an icon, a callback and
    // a 1.5 s timeout on it.
    popup_reset(ibutton->popup);
    popup_set_icon(ibutton->popup, 83, 22, &I_WarningDolphinFlip_45x42);
    popup_set_header(ibutton->popup, "Error", 64, 3, AlignCenter, AlignTop);
    popup_set_text(
        ibutton->popup,
        "Settings page\nmissing.\nUpdate the\nSD resources",
        3,
        19,
        AlignLeft,
        AlignTop);
    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewPopup);
}

bool ibutton_scene_settings_write_targets_on_event(void* context, SceneManagerEvent event) {
    iButton* ibutton = context;

    // In on_event rather than on_exit: here the page is still the current scene and the plugin
    // is still mapped, whereas on_exit is mid-transition and also runs on teardown paths where
    // blocking on a modal would be wrong.
    if(event.type == SceneManagerEventTypeBack && settings_plugin) {
        if(!settings_plugin->write_targets->on_save()) {
            dialog_message_show_storage_error(ibutton->dialogs, "Cannot save\nsettings");
        }
        settings_plugin = NULL;
    }

    return false;
}

void ibutton_scene_settings_write_targets_on_exit(void* context) {
    iButton* ibutton = context;

    settings_plugin = NULL;

    if(ibutton->variable_item_list) {
        // Both kinds of callback on the list point into the plugin: reset() drops the per-item
        // ones with the items, the enter callback has to be replaced by hand.
        variable_item_list_reset(ibutton->variable_item_list);
        variable_item_list_set_enter_callback(
            ibutton->variable_item_list, ibutton_scene_settings_enter_callback_none, ibutton);
        // reset() leaves the cursor where it was, which would be out of range for a shorter
        // page later on.
        variable_item_list_set_selected_item(ibutton->variable_item_list, 0);
    }

    if(settings_plugin_manager) {
        plugin_manager_free(settings_plugin_manager);
        settings_plugin_manager = NULL;
    }

    popup_reset(ibutton->popup);
}
