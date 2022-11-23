#pragma once

#include <cli/cli.h>
#include "../types/plugin_state.h"

void totp_cli_register_command_handler(PluginState* plugin_state);
void totp_cli_unregister_command_handler();