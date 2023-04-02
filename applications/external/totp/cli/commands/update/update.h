#pragma once

#include <cli/cli.h>
#include "../../../types/plugin_state.h"

#define TOTP_CLI_COMMAND_UPDATE "update"

void totp_cli_command_update_handle(PluginState* plugin_state, FuriString* args, Cli* cli);
void totp_cli_command_update_docopt_commands();
void totp_cli_command_update_docopt_usage();
void totp_cli_command_update_docopt_options();