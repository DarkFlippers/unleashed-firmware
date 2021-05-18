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

typedef enum {
    CliSymbolAsciiSOH = 0x01,
    CliSymbolAsciiETX = 0x03,
    CliSymbolAsciiEOT = 0x04,
    CliSymbolAsciiBell = 0x07,
    CliSymbolAsciiBackspace = 0x08,
    CliSymbolAsciiTab = 0x09,
    CliSymbolAsciiCR = 0x0D,
    CliSymbolAsciiEsc = 0x1B,
    CliSymbolAsciiUS = 0x1F,
    CliSymbolAsciiSpace = 0x20,
    CliSymbolAsciiDel = 0x7F,
} CliSymbols;

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
