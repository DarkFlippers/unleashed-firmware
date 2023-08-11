#pragma once

#include <cli/cli.h>
#include "../../../types/plugin_state.h"
#include "../../../config/app/config.h"

#define TOTP_CLI_COMMAND_TIMEZONE "timezone"
#define TOTP_CLI_COMMAND_TIMEZONE_ALT "tz"

void totp_cli_command_timezone_handle(PluginState* plugin_state, FuriString* args, Cli* cli);
#ifdef TOTP_CLI_RICH_HELP_ENABLED
void totp_cli_command_timezone_docopt_commands();
void totp_cli_command_timezone_docopt_usage();
void totp_cli_command_timezone_docopt_arguments();
#endif
