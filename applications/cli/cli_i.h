#pragma once

#include "cli.h"

#include <furi.h>
#include <furi_hal.h>

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

typedef struct CliSession CliSession;

struct CliSession {
    void (*init)(void);
    void (*deinit)(void);
    size_t (*rx)(uint8_t* buffer, size_t size, uint32_t timeout);
    void (*tx)(const uint8_t* buffer, size_t size);
    void (*tx_stdout)(const char* data, size_t size);
    bool (*is_connected)(void);
};

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
    FuriMutex* mutex;
    FuriSemaphore* idle_sem;
    string_t last_line;
    string_t line;
    CliSession* session;

    size_t cursor_position;
};

Cli* cli_alloc();

void cli_reset(Cli* cli);

void cli_putc(Cli* cli, char c);

void cli_stdout_callback(void* _cookie, const char* data, size_t size);
