#pragma once

#include <cli/cli.h>
#include "../types/plugin_state.h"

#define TOTP_CLI_COMMAND_NAME "totp"

#define DOCOPT_ARGUMENT(arg) "<" arg ">"
#define DOCOPT_MULTIPLE(arg) arg "..."
#define DOCOPT_OPTIONAL(param) "[" param "]"
#define DOCOPT_REQUIRED(param) "(" param ")"
#define DOCOPT_OPTION(option, value) option " " value
#define DOCOPT_SWITCH(option) option
#define DOCOPT_OPTIONS "[options]"
#define DOCOPT_DEFAULT(val) "[default: " val "]"

#define TOTP_CLI_PRINTF(format, ...)                                        \
    do {                                                                    \
        _Pragma(STRINGIFY(GCC diagnostic push))                             \
            _Pragma(STRINGIFY(GCC diagnostic ignored "-Wdouble-promotion")) \
                printf(format, ##__VA_ARGS__);                              \
        _Pragma(STRINGIFY(GCC diagnostic pop))                              \
    } while(false)

#define TOTP_CLI_PRINTF_COLORFUL(color, format, ...)                        \
    do {                                                                    \
        _Pragma(STRINGIFY(GCC diagnostic push))                             \
            _Pragma(STRINGIFY(GCC diagnostic ignored "-Wdouble-promotion")) \
                printf("\e[%s", color);                                     \
        printf(format, ##__VA_ARGS__);                                      \
        printf("\e[0m");                                                    \
        fflush(stdout);                                                     \
        _Pragma(STRINGIFY(GCC diagnostic pop))                              \
    } while(false)

#define TOTP_CLI_COLOR_ERROR "91m"
#define TOTP_CLI_COLOR_WARNING "93m"
#define TOTP_CLI_COLOR_SUCCESS "92m"
#define TOTP_CLI_COLOR_INFO "96m"

#define TOTP_CLI_PRINTF_ERROR(format, ...) \
    TOTP_CLI_PRINTF_COLORFUL(TOTP_CLI_COLOR_ERROR, format, ##__VA_ARGS__)
#define TOTP_CLI_PRINTF_WARNING(format, ...) \
    TOTP_CLI_PRINTF_COLORFUL(TOTP_CLI_COLOR_WARNING, format, ##__VA_ARGS__)
#define TOTP_CLI_PRINTF_SUCCESS(format, ...) \
    TOTP_CLI_PRINTF_COLORFUL(TOTP_CLI_COLOR_SUCCESS, format, ##__VA_ARGS__)
#define TOTP_CLI_PRINTF_INFO(format, ...) \
    TOTP_CLI_PRINTF_COLORFUL(TOTP_CLI_COLOR_INFO, format, ##__VA_ARGS__)

#define TOTP_CLI_DELETE_LAST_LINE()    \
    TOTP_CLI_PRINTF("\033[A\33[2K\r"); \
    fflush(stdout)

#define TOTP_CLI_DELETE_CURRENT_LINE() \
    TOTP_CLI_PRINTF("\33[2K\r");       \
    fflush(stdout)

#define TOTP_CLI_DELETE_LAST_CHAR() \
    TOTP_CLI_PRINTF("\b \b");       \
    fflush(stdout)

#define TOTP_CLI_PRINT_INVALID_ARGUMENTS() \
    TOTP_CLI_PRINTF_ERROR(                 \
        "Invalid command arguments. use \"help\" command to get list of available commands")

#define TOTP_CLI_PRINT_ERROR_UPDATING_CONFIG_FILE() \
    TOTP_CLI_PRINTF_ERROR("An error has occurred during updating config file\r\n")

/**
 * @brief Checks whether user is authenticated and entered correct PIN.
 *        If user is not authenticated it prompts user to enter correct PIN to authenticate.
 * @param plugin_state application state
 * @param cli pointer to the firmware CLI subsystem 
 * @return \c true if user is already authenticated or successfully authenticated; \c false otherwise
 */
bool totp_cli_ensure_authenticated(const PluginState* plugin_state, Cli* cli);

/**
 * @brief Forces application to be instantly closed
 * @param event_queue main app queue
 */
void totp_cli_force_close_app(FuriMessageQueue* event_queue);

/**
 * @brief Reads line of characters from console
 * @param cli pointer to the firmware CLI subsystem 
 * @param out_str pointer to an output string to put read line to
 * @param mask_user_input whether to mask input characters in console or not
 * @return \c true if line successfully read and confirmed; \c false otherwise
 */
bool totp_cli_read_line(Cli* cli, FuriString* out_str, bool mask_user_input);
