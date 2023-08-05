#pragma once

#include <cli/cli.h>
#include "../types/plugin_state.h"

typedef struct TotpCliContext TotpCliContext;

/**
 * @brief Registers TOTP CLI handler
 * @param plugin_state application state
 * @return TOTP CLI context
 */
TotpCliContext* totp_cli_register_command_handler(PluginState* plugin_state);

/**
 * @brief Unregisters TOTP CLI handler
 * @param context application state
 */
void totp_cli_unregister_command_handler(TotpCliContext* context);