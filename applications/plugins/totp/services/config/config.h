#pragma once

#include <flipper_format/flipper_format.h>
#include <furi.h>
#include "../../types/plugin_state.h"
#include "../../types/token_info.h"
#include "constants.h"

typedef uint8_t TokenLoadingResult;
typedef uint8_t TotpConfigFileOpenResult;
typedef uint8_t TotpConfigFileUpdateResult;

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
 * @brief Config file opening result
 */
enum TotpConfigFileOpenResults {
    /**
     * @brief Config file opened successfully
     */
    TotpConfigFileOpenSuccess = 0,

    /**
     * @brief An error has occurred during opening config file
     */
    TotpConfigFileOpenError = 1
};

/**
 * @brief Config file updating result
 */
enum TotpConfigFileUpdateResults {
    /**
     * @brief Config file updated successfully
     */
    TotpConfigFileUpdateSuccess,

    /**
     * @brief An error has occurred during updating config file
     */
    TotpConfigFileUpdateError
};

/**
 * @brief Saves all the settings and tokens to an application config file
 * @param plugin_state application state
 * @return Config file update result
 */
TotpConfigFileUpdateResult totp_full_save_config_file(const PluginState* const plugin_state);

/**
 * @brief Loads basic information from an application config file into application state without loading all the tokens
 * @param plugin_state application state
 * @return Config file open result
 */
TotpConfigFileOpenResult totp_config_file_load_base(PluginState* const plugin_state);

/**
 * @brief Loads tokens from an application config file into application state
 * @param plugin_state application state
 * @return Results of the loading
 */
TokenLoadingResult totp_config_file_load_tokens(PluginState* const plugin_state);

/**
 * @brief Add new token to the end of the application config file
 * @param token_info token information to be saved
 * @return Config file update result
 */
TotpConfigFileUpdateResult totp_config_file_save_new_token(const TokenInfo* token_info);

/**
 * @brief Updates timezone offset in an application config file
 * @param new_timezone_offset new timezone offset to be set
 * @return Config file update result
 */
TotpConfigFileUpdateResult totp_config_file_update_timezone_offset(float new_timezone_offset);

/**
 * @brief Updates notification method in an application config file
 * @param new_notification_method new notification method to be set
 * @return Config file update result
 */
TotpConfigFileUpdateResult
    totp_config_file_update_notification_method(NotificationMethod new_notification_method);

/**
 * @brief Updates application user settings
 * @param plugin_state application state
 * @return Config file update result
 */
TotpConfigFileUpdateResult totp_config_file_update_user_settings(const PluginState* plugin_state);

/**
 * @brief Updates crypto signatures information
 * @param plugin_state application state
 * @return Config file update result
 */
TotpConfigFileUpdateResult
    totp_config_file_update_crypto_signatures(const PluginState* plugin_state);

/**
 * @brief Reset all the settings to default
 */
void totp_config_file_reset();