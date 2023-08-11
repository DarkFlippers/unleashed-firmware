#pragma once

#include <cli/cli.h>
#include "../../../types/plugin_state.h"
#include "../../../config/app/config.h"

#define TOTP_CLI_COMMAND_DETAILS "lsattr"
#define TOTP_CLI_COMMAND_DETAILS_ALT "cat"

void totp_cli_command_details_handle(PluginState* plugin_state, FuriString* args, Cli* cli);
#ifdef TOTP_CLI_RICH_HELP_ENABLED
void totp_cli_command_details_docopt_commands();
void totp_cli_command_details_docopt_usage();
#endif
