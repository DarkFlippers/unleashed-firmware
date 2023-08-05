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

extern const char* TOTP_CLI_COLOR_ERROR;
extern const char* TOTP_CLI_COLOR_WARNING;
extern const char* TOTP_CLI_COLOR_SUCCESS;
extern const char* TOTP_CLI_COLOR_INFO;

#define TOTP_CLI_PRINTF(format, ...) printf(format, ##__VA_ARGS__)

#define TOTP_CLI_PRINTF_COLORFUL(color, format, ...) \
    TOTP_CLI_PRINTF("\e[%s" format "\e[0m", color, ##__VA_ARGS__)

#define TOTP_CLI_PRINTF_ERROR(format, ...) \
    TOTP_CLI_PRINTF_COLORFUL(TOTP_CLI_COLOR_ERROR, format, ##__VA_ARGS__)
#define TOTP_CLI_PRINTF_WARNING(format, ...) \
    TOTP_CLI_PRINTF_COLORFUL(TOTP_CLI_COLOR_WARNING, format, ##__VA_ARGS__)
#define TOTP_CLI_PRINTF_SUCCESS(format, ...) \
    TOTP_CLI_PRINTF_COLORFUL(TOTP_CLI_COLOR_SUCCESS, format, ##__VA_ARGS__)
#define TOTP_CLI_PRINTF_INFO(format, ...) \
    TOTP_CLI_PRINTF_COLORFUL(TOTP_CLI_COLOR_INFO, format, ##__VA_ARGS__)

#define TOTP_CLI_LOCK_UI(plugin_state)                                  \
    Scene __previous_scene = plugin_state->current_scene;               \
    totp_scene_director_activate_scene(plugin_state, TotpSceneStandby); \
    totp_scene_director_force_redraw(plugin_state)

#define TOTP_CLI_UNLOCK_UI(plugin_state)                                \
    totp_scene_director_activate_scene(plugin_state, __previous_scene); \
    totp_scene_director_force_redraw(plugin_state)

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

/**
 * @brief Extracts \c uint8_t value and trims arguments string
 * @param args arguments string
 * @param[out] value parsed value
 * @return \c true if value successfully read and parsed as \c uint8_t ; \c false otherwise
 */
bool args_read_uint8_and_trim(FuriString* args, uint8_t* value);

/**
 * @brief Free \c FuriString instance in a secure manner by clearing it first
 * @param str instance to free
 */
void furi_string_secure_free(FuriString* str);

/**
 * @brief Deletes last printed line in console
 */
void totp_cli_delete_last_line();

/**
 * @brief Deletes current printed line in console
 */
void totp_cli_delete_current_line();

/**
 * @brief Deletes last printed char in console
 */
void totp_cli_delete_last_char();

/**
 * @brief Prints error message about invalid command arguments
 */
void totp_cli_print_invalid_arguments();

/**
 * @brief Prints error message about config file update error
 */
void totp_cli_print_error_updating_config_file();

/**
 * @brief Prints error message about config file loading error
 */
void totp_cli_print_error_loading_token_info();

/**
 * @brief Prints message to let user know that command is processing now
 */
void totp_cli_print_processing();