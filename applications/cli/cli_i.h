#pragma once

#include "cli.h"

#include <furi.h>
#include <api-hal.h>

#include <m-dict.h>
#include <m-bptree.h>

#define CLI_LINE_SIZE_MAX
#define CLI_COMMANDS_TREE_RANK 4

typedef struct {
    CliCallback callback;
    void* context;
} CliCommand;

BPTREE_DEF2(
    CliCommandTree,
    CLI_COMMANDS_TREE_RANK,
    string_t,
    STRING_OPLIST,
    CliCommand,
    M_POD_OPLIST)

struct Cli {
    CliCommandTree_t commands;
    osMutexId_t mutex;
    string_t line;
};

Cli* cli_alloc();
void cli_free(Cli* cli);
void cli_reset_state(Cli* cli);
void cli_print_version(const Version* version);
void cli_putc(char c);
void cli_stdout_callback(void* _cookie, const char* data, size_t size);
