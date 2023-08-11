#pragma once

#include <cli/cli.h>
#include "../../../types/plugin_state.h"
#include "../../../config/app/config.h"

#define TOTP_CLI_COMMAND_LIST "list"
#define TOTP_CLI_COMMAND_LIST_ALT "ls"

void totp_cli_command_list_handle(PluginState* plugin_state, Cli* cli);
#ifdef TOTP_CLI_RICH_HELP_ENABLED
void totp_cli_command_list_docopt_commands();
void totp_cli_command_list_docopt_usage();
#endif
