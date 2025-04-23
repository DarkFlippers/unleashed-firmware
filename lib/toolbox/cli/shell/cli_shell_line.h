#pragma once

#include <furi.h>

#include "cli_shell_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CliShellLine CliShellLine;

CliShellLine* cli_shell_line_alloc(CliShell* shell);

void cli_shell_line_free(CliShellLine* line);

FuriString* cli_shell_line_get_selected(CliShellLine* line);

FuriString* cli_shell_line_get_editing(CliShellLine* line);

size_t cli_shell_line_prompt_length(CliShellLine* line);

void cli_shell_line_format_prompt(CliShellLine* line, char* buf, size_t length);

void cli_shell_line_prompt(CliShellLine* line);

size_t cli_shell_line_get_line_position(CliShellLine* line);

void cli_shell_line_set_line_position(CliShellLine* line, size_t position);

/**
 * @brief If a line from history has been selected, moves it into the active line
 */
void cli_shell_line_ensure_not_overwriting_history(CliShellLine* line);

void cli_shell_line_set_about_to_exit(CliShellLine* line);

extern CliShellKeyComboSet cli_shell_line_key_combo_set;

#ifdef __cplusplus
}
#endif
