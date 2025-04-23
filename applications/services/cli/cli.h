#pragma once

/* 
 * Compatibility header for ease of porting existing apps.
 *  In short: 
 *   Cli* is replaced with with CliRegistry*
 *   cli_* functions are replaced with cli_registry_* functions
 *   (i.e., cli_add_command() is now cli_registry_add_command())
*/

#include <toolbox/cli/cli_registry.h>

#define RECORD_CLI "cli"
