#pragma once

#include <dialogs/dialogs.h>
#include "../types/plugin_state.h"

/**
 * @brief Shows standard dialog about the fact that error occurred when loading config file
 * @param plugin_state application state
 * @return dialog button which user pressed to close the dialog
 */
DialogMessageButton totp_dialogs_config_loading_error(PluginState* plugin_state);

/**
 * @brief Shows standard dialog about the fact that error occurred when updating config file
 * @param plugin_state application state
 * @return dialog button which user pressed to close the dialog
 */
DialogMessageButton totp_dialogs_config_updating_error(PluginState* plugin_state);