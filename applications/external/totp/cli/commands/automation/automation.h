#pragma once

#include <cli/cli.h>
#include "../../../types/plugin_state.h"
#include "../../../config/app/config.h"

#define TOTP_CLI_COMMAND_AUTOMATION "automation"

void totp_cli_command_automation_handle(PluginState* plugin_state, FuriString* args, Cli* cli);
#ifdef TOTP_CLI_RICH_HELP_ENABLED
void totp_cli_command_automation_docopt_commands();
void totp_cli_command_automation_docopt_usage();
void totp_cli_command_automation_docopt_arguments();
void totp_cli_command_automation_docopt_options();
#endif