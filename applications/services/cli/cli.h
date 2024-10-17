/**
 * @file cli.h
 * Cli API
 */

#pragma once
#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CliSymbolAsciiSOH = 0x01,
    CliSymbolAsciiETX = 0x03,
    CliSymbolAsciiEOT = 0x04,
    CliSymbolAsciiBell = 0x07,
    CliSymbolAsciiBackspace = 0x08,
    CliSymbolAsciiTab = 0x09,
    CliSymbolAsciiLF = 0x0A,
    CliSymbolAsciiCR = 0x0D,
    CliSymbolAsciiEsc = 0x1B,
    CliSymbolAsciiUS = 0x1F,
    CliSymbolAsciiSpace = 0x20,
    CliSymbolAsciiDel = 0x7F,
} CliSymbols;

typedef enum {
    CliCommandFlagDefault = 0, /**< Default, loader lock is used */
    CliCommandFlagParallelSafe =
        (1 << 0), /**< Safe to run in parallel with other apps, loader lock is not used */
    CliCommandFlagInsomniaSafe = (1 << 1), /**< Safe to run with insomnia mode on */
} CliCommandFlag;

#define RECORD_CLI "cli"

/** Cli type anonymous structure */
typedef struct Cli Cli;

/** Cli callback function pointer. Implement this interface and use
 * add_cli_command
 * @param      args     string with what was passed after command
 * @param      context  pointer to whatever you gave us on cli_add_command
 */
typedef void (*CliCallback)(Cli* cli, FuriString* args, void* context);

/** Add cli command Registers you command callback
 *
 * @param      cli       pointer to cli instance
 * @param      name      command name
 * @param      flags     CliCommandFlag
 * @param      callback  callback function
 * @param      context   pointer to whatever we need to pass to callback
 */
void cli_add_command(
    Cli* cli,
    const char* name,
    CliCommandFlag flags,
    CliCallback callback,
    void* context);

/** Print unified cmd usage tip
 *
 * @param      cmd    cmd name
 * @param      usage  usage tip
 * @param      arg    arg passed by user
 */
void cli_print_usage(const char* cmd, const char* usage, const char* arg);

/** Delete cli command
 *
 * @param      cli   pointer to cli instance
 * @param      name  command name
 */
void cli_delete_command(Cli* cli, const char* name);

/** Read from terminal
 *
 * @param      cli     Cli instance
 * @param      buffer  pointer to buffer
 * @param      size    size of buffer in bytes
 *
 * @return     bytes read
 */
size_t cli_read(Cli* cli, uint8_t* buffer, size_t size);

/** Non-blocking read from terminal
 *
 * @param      cli     Cli instance
 * @param      buffer  pointer to buffer
 * @param      size    size of buffer in bytes
 * @param      timeout timeout value in ms
 *
 * @return     bytes read
 */
size_t cli_read_timeout(Cli* cli, uint8_t* buffer, size_t size, uint32_t timeout);

/** Non-blocking check for interrupt command received
 *
 * @param      cli   Cli instance
 *
 * @return     true if received
 */
bool cli_cmd_interrupt_received(Cli* cli);

/** Write to terminal Do it only from inside of cli call.
 *
 * @param      cli     Cli instance
 * @param      buffer  pointer to buffer
 * @param      size    size of buffer in bytes
 */
void cli_write(Cli* cli, const uint8_t* buffer, size_t size);

/** Read character
 *
 * @param      cli   Cli instance
 *
 * @return     char
 */
char cli_getc(Cli* cli);

/** New line Send new ine sequence
 */
void cli_nl(Cli* cli);

void cli_session_open(Cli* cli, void* session);

void cli_session_close(Cli* cli);

bool cli_is_connected(Cli* cli);

#ifdef __cplusplus
}
#endif
