#pragma once

#include "cli.h"
#include <toolbox/cli/cli_command.h>
#include <toolbox/cli/cli_registry.h>

#define CLI_APPID "cli"

void cli_main_commands_init(CliRegistry* registry);
