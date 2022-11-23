#pragma once

#include <cli/cli.h>

#define TOTP_CLI_COMMAND_HELP "help"
#define TOTP_CLI_COMMAND_HELP_ALT "h"
#define TOTP_CLI_COMMAND_HELP_ALT2 "?"

void totp_cli_command_help_handle();
void totp_cli_command_help_docopt_commands();
void totp_cli_command_help_docopt_usage();