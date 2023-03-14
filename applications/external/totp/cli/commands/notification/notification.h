#pragma once

#include <cli/cli.h>
#include "../../../types/plugin_state.h"

#define TOTP_CLI_COMMAND_NOTIFICATION "notify"

void totp_cli_command_notification_handle(PluginState* plugin_state, FuriString* args, Cli* cli);
void totp_cli_command_notification_docopt_commands();
void totp_cli_command_notification_docopt_usage();
void totp_cli_command_notification_docopt_arguments();