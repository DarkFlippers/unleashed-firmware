#pragma once

#include "cli.h"

#include <furi.h>

#include <m-dict.h>

#define CLI_LINE_SIZE_MAX

typedef struct {
    CliCallback callback;
    void* context;
} CliCommand;

DICT_DEF2(CliCommandDict, string_t, STRING_OPLIST, CliCommand, M_POD_OPLIST)

typedef enum {
    CliSymbolAsciiSOH = 0x01,
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
    CliCommandDict_t commands;
    osMutexId_t mutex;
    string_t line;
};

Cli* cli_alloc();
void cli_free(Cli* cli);
void cli_reset_state(Cli* cli);
void cli_print_version();
void cli_putc(char c);
void cli_stdout_callback(void* _cookie, const char* data, size_t size);
