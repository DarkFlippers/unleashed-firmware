#pragma once

#include <furi.h>
#include <m-array.h>
#include "cli_shell_i.h"
#include "cli_shell_line.h"
#include "../cli_registry.h"
#include "../cli_registry_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CliShellCompletions CliShellCompletions;

CliShellCompletions*
    cli_shell_completions_alloc(CliRegistry* registry, CliShell* shell, CliShellLine* line);

void cli_shell_completions_free(CliShellCompletions* completions);

extern CliShellKeyComboSet cli_shell_completions_key_combo_set;

#ifdef __cplusplus
}
#endif
