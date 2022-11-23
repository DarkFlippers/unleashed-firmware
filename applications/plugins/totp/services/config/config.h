#pragma once

#include <flipper_format/flipper_format.h>
#include <furi.h>
#include "../../types/plugin_state.h"
#include "../../types/token_info.h"
#include "constants.h"

typedef uint8_t TokenLoadingResult;

/**
 * @brief Token loading results
 */
enum TokenLoadingResults {
    /**
     * @brief All the tokens loaded successfully 
     */
    TokenLoadingResultSuccess,

    /**
     * @brief All the tokens loaded, but there are some warnings
     */
    TokenLoadingResultWarning,

    /**
     * @brief Tokens not loaded because of error(s) 
     */
    TokenLoadingResultError
};

/**
 * @brief Opens storage record
 * @return Storage record
 */
Storage* totp_open_storage();

/**
 * @brief Closes storage record
 */
void totp_close_storage();

/**
 * @brief Opens or creates TOTP application standard config file
 * @param storage storage record to use
 * @return Config file reference
 */
FlipperFormat* totp_open_config_file(Storage* storage);

/**
 * @brief Closes config file
 * @param file config file reference
 */
void totp_close_config_file(FlipperFormat* file);

/**
 * @brief Saves all the settings and tokens to an application config file
 * @param plugin_state application state
 */
void totp_full_save_config_file(const PluginState* const plugin_state);

/**
 * @brief Loads basic information from an application config file into application state without loading all the tokens
 * @param plugin_state application state
 */
void totp_config_file_load_base(PluginState* const plugin_state);

/**
 * @brief Loads tokens from an application config file into application state
 * @param plugin_state application state
 * @return Results of the loading
 */
TokenLoadingResult totp_config_file_load_tokens(PluginState* const plugin_state);

/**
 * @brief Add new token to the end of the application config file
 * @param token_info token information to be saved
 */
void totp_config_file_save_new_token(const TokenInfo* token_info);

/**
 * @brief Updates timezone offset in an application config file
 * @param new_timezone_offset new timezone offset to be set
 */
void totp_config_file_update_timezone_offset(float new_timezone_offset);

/**
 * @brief Updates notification method in an application config file
 * @param new_notification_method new notification method to be set
 */
void totp_config_file_update_notification_method(NotificationMethod new_notification_method);

/**
 * @brief Updates application user settings
 * @param plugin_state application state
 */
void totp_config_file_update_user_settings(const PluginState* plugin_state);