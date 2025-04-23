#pragma once

#include <furi.h>
#include <toolbox/pipe.h>
#include "../cli_registry.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CLI_SHELL_STACK_SIZE (4 * 1024U)

typedef struct CliShell CliShell;

/**
 * Called from the shell thread to print the Message of the Day when the shell
 * is started.
 */
typedef void (*CliShellMotd)(void* context);

/**
 * @brief Allocates a shell
 * 
 * @param [in] motd       Message of the Day callback
 * @param [in] context    Callback context
 * @param [in] pipe       Pipe side to be used by the shell
 * @param [in] registry   Command registry
 * @param [in] ext_config External command configuration. See
 *                        `CliCommandExternalConfig`. May be NULL if support for
 *                        external commands is not required.
 * 
 * @return Shell instance
 */
CliShell* cli_shell_alloc(
    CliShellMotd motd,
    void* context,
    PipeSide* pipe,
    CliRegistry* registry,
    const CliCommandExternalConfig* ext_config);

/**
 * @brief Frees a shell
 * 
 * @param [in] shell Shell instance
 */
void cli_shell_free(CliShell* shell);

/**
 * @brief Starts a shell
 * 
 * The shell runs in a separate thread. This call is non-blocking.
 * 
 * @param [in] shell Shell instance
 */
void cli_shell_start(CliShell* shell);

/**
 * @brief Joins the shell thread
 * 
 * @warning This call is blocking.
 * 
 * @param [in] shell Shell instance
 */
void cli_shell_join(CliShell* shell);

/**
 * @brief Sets optional text before prompt (`>:`)
 * 
 * @param [in] shell Shell instance
 */
void cli_shell_set_prompt(CliShell* shell, const char* prompt);

#ifdef __cplusplus
}
#endif
