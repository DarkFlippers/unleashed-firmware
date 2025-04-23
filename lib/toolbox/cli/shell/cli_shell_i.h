#pragma once

#include "../cli_ansi.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CliShell CliShell;

/**
 * @brief Key combo handler
 * @return true if the event was handled, false otherwise
 */
typedef bool (*CliShellKeyComboAction)(CliKeyCombo combo, void* context);

typedef struct {
    CliKeyCombo combo;
    CliShellKeyComboAction action;
} CliShellKeyComboRecord;

typedef struct {
    CliShellKeyComboAction fallback;
    size_t count;
    CliShellKeyComboRecord records[];
} CliShellKeyComboSet;

void cli_shell_execute_command(CliShell* cli_shell, FuriString* command);

const char* cli_shell_get_prompt(CliShell* cli_shell);

#ifdef __cplusplus
}
#endif
