#pragma once

#include <m-string.h>

/* Cli type
 * Anonymous structure. Use cli_i.h if you need to go deeper.
 */
typedef struct Cli Cli;

/* Cli callback function pointer.
 * Implement this interface and use add_cli_command
 * @param args - string with what was passed after command
 * @param context - pointer to whatever you gave us on cli_add_command
 */
typedef void (*CliCallback)(string_t args, void* context);

/* Add cli command
 * Registers you command callback
 * @param cli - pointer to cli instance
 * @param name - command name
 * @param callback - callback function
 * @param context - pointer to whatever we need to pass to callback
 */
void cli_add_command(Cli* cli, const char* name, CliCallback callback, void* context);

/* Read terminal input.
 * Do it only from inside of callback.
 * @param buffer - buffer pointer to char buffer
 * @param size - size of buffer in bytes
 */
void cli_read(char* buffer, size_t size);

/* Print to terminal
 * Do it only from inside of callback.
 * @param buffer - pointer to null terminated string to print.
 */
void cli_print(const char* buffer);

/* New line 
 * Send new ine sequence
 */
void cli_nl();
