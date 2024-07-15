#pragma once

#include "cli.h"

#include <furi.h>
#include <furi_hal.h>

#include <m-dict.h>
#include <m-bptree.h>
#include <m-array.h>

#include "cli_vcp.h"

#define CLI_LINE_SIZE_MAX
#define CLI_COMMANDS_TREE_RANK 4

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    CliCallback callback;
    void* context;
    uint32_t flags;
} CliCommand;

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
    FuriString*,
    FURI_STRING_OPLIST,
    CliCommand,
    M_POD_OPLIST)

#define M_OPL_CliCommandTree_t() BPTREE_OPLIST(CliCommandTree, M_POD_OPLIST)

struct Cli {
    CliCommandTree_t commands;
    FuriMutex* mutex;
    FuriSemaphore* idle_sem;
    FuriString* last_line;
    FuriString* line;
    CliSession* session;

    size_t cursor_position;
};

Cli* cli_alloc(void);

void cli_reset(Cli* cli);

void cli_putc(Cli* cli, char c);

void cli_stdout_callback(void* _cookie, const char* data, size_t size);

// Wraps CLI commands to load from plugin file
// Must call from CLI context, like dummy CLI command callback
// You need to setup the plugin to compile correctly separately
#define CLI_PLUGIN_APP_ID      "cli"
#define CLI_PLUGIN_API_VERSION 1
void cli_plugin_wrapper(const char* name, Cli* cli, FuriString* args, void* context);

#ifdef __cplusplus
}
#endif
