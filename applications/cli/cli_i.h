#pragma once

#include "cli.h"

#include <furi.h>
#include <furi-hal.h>

#include <m-dict.h>
#include <m-bptree.h>
#include <m-array.h>

#define CLI_LINE_SIZE_MAX
#define CLI_COMMANDS_TREE_RANK 4

typedef struct {
    CliCallback callback;
    void* context;
    uint32_t flags;
} CliCommand;

BPTREE_DEF2(
    CliCommandTree,
    CLI_COMMANDS_TREE_RANK,
    string_t,
    STRING_OPLIST,
    CliCommand,
    M_POD_OPLIST)

#define M_OPL_CliCommandTree_t() BPTREE_OPLIST(CliCommandTree, M_POD_OPLIST)

struct Cli {
    CliCommandTree_t commands;
    osMutexId_t mutex;
    string_t last_line;
    string_t line;

    size_t cursor_position;
};

Cli* cli_alloc();

void cli_free(Cli* cli);

void cli_reset(Cli* cli);

void cli_putc(char c);

void cli_stdout_callback(void* _cookie, const char* data, size_t size);
