/**
 * @file cli_registry.h
 * API for registering commands with a CLI shell
 */

#pragma once

#include <furi.h>
#include <m-array.h>
#include <toolbox/pipe.h>
#include "cli_command.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CliRegistry CliRegistry;

/**
 * @brief Allocates a `CliRegistry`.
 */
CliRegistry* cli_registry_alloc(void);

/**
 * @brief Frees a `CliRegistry`.
 */
void cli_registry_free(CliRegistry* registry);

/**
 * @brief Registers a command with the registry. Provides less options than the
 * `_ex` counterpart.
 *
 * @param [in] registry  Pointer to registry instance
 * @param [in] name      Command name
 * @param [in] flags     see CliCommandFlag
 * @param [in] callback  Callback function
 * @param [in] context   Custom context
 */
void cli_registry_add_command(
    CliRegistry* registry,
    const char* name,
    CliCommandFlag flags,
    CliCommandExecuteCallback callback,
    void* context);

/**
 * @brief Registers a command with the registry. Provides more options than the
 * non-`_ex` counterpart.
 *
 * @param [in] registry   Pointer to registry instance
 * @param [in] name       Command name
 * @param [in] flags      see CliCommandFlag
 * @param [in] callback   Callback function
 * @param [in] context    Custom context
 * @param [in] stack_size Thread stack size
 */
void cli_registry_add_command_ex(
    CliRegistry* registry,
    const char* name,
    CliCommandFlag flags,
    CliCommandExecuteCallback callback,
    void* context,
    size_t stack_size);

/**
 * @brief Deletes a cli command
 *
 * @param [in] registry Pointer to registry instance
 * @param [in] name     Command name
 */
void cli_registry_delete_command(CliRegistry* registry, const char* name);

/**
 * @brief Unregisters all external commands
 * 
 * @param [in] registry Pointer to registry instance
 */
void cli_registry_remove_external_commands(CliRegistry* registry);

/**
 * @brief Reloads the list of externally available commands
 * 
 * @param [in] registry Pointer to registry instance
 * @param [in] config   See `CliCommandExternalConfig`
 */
void cli_registry_reload_external_commands(
    CliRegistry* registry,
    const CliCommandExternalConfig* config);

#ifdef __cplusplus
}
#endif
