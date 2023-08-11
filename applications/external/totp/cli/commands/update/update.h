#pragma once

#include <cli/cli.h>
#include "../../../types/plugin_state.h"
#include "../../../config/app/config.h"

#define TOTP_CLI_COMMAND_UPDATE "update"

void totp_cli_command_update_handle(PluginState* plugin_state, FuriString* args, Cli* cli);
#ifdef TOTP_CLI_RICH_HELP_ENABLED
void totp_cli_command_update_docopt_commands();
void totp_cli_command_update_docopt_usage();
void totp_cli_command_update_docopt_options();
#endif