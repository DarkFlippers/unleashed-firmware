/**
 * @file cli_registry_i.h
 * Internal API for getting commands registered with the CLI
 */

#pragma once

#include <furi.h>
#include <m-dict.h>
#include "cli_registry.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CLI_BUILTIN_COMMAND_STACK_SIZE (4 * 1024U)

typedef struct {
    void* context; //<! Context passed to callbacks
    CliCommandExecuteCallback execute_callback; //<! Callback for command execution
    CliCommandFlag flags;
    size_t stack_depth;
} CliRegistryCommand;

DICT_DEF2(CliCommandDict, FuriString*, FURI_STRING_OPLIST, CliRegistryCommand, M_POD_OPLIST);

#define M_OPL_CliCommandDict_t() DICT_OPLIST(CliCommandDict, FURI_STRING_OPLIST, M_POD_OPLIST)

bool cli_registry_get_command(
    CliRegistry* registry,
    FuriString* command,
    CliRegistryCommand* result);

void cli_registry_lock(CliRegistry* registry);

void cli_registry_unlock(CliRegistry* registry);

/**
 * @warning Surround calls to this function with `cli_registry_[un]lock`
 */
CliCommandDict_t* cli_registry_get_commands(CliRegistry* registry);

#ifdef __cplusplus
}
#endif
