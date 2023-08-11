#pragma once

#include <cli/cli.h>
#include "../../../types/plugin_state.h"
#include "../../../config/app/config.h"

#define TOTP_CLI_COMMAND_RESET "reset"

void totp_cli_command_reset_handle(PluginState* plugin_state, Cli* cli);
#ifdef TOTP_CLI_RICH_HELP_ENABLED
void totp_cli_command_reset_docopt_commands();
void totp_cli_command_reset_docopt_usage();
#endif
