#pragma once

#include <cli/cli.h>
#include "../../../types/plugin_state.h"
#include "../../../config/app/config.h"

#define TOTP_CLI_COMMAND_DELETE "delete"
#define TOTP_CLI_COMMAND_DELETE_ALT "rm"

void totp_cli_command_delete_handle(PluginState* plugin_state, FuriString* args, Cli* cli);
#ifdef TOTP_CLI_RICH_HELP_ENABLED
void totp_cli_command_delete_docopt_commands();
void totp_cli_command_delete_docopt_usage();
void totp_cli_command_delete_docopt_arguments();
void totp_cli_command_delete_docopt_options();
#endif
