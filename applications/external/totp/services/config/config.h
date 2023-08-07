#pragma once

#include "../../types/plugin_state.h"
#include "../../types/token_info.h"
#include "config_file_context.h"
#include "constants.h"
#include "token_info_iterator.h"

typedef uint8_t TotpConfigFileOpenResult;
typedef uint8_t TotpConfigFileUpdateResult;

/**
 * @brief Tries to take a config file backup
 * @param plugin_state application state
 * @return backup path if backup successfully taken; \c NULL otherwise
 */
char* totp_config_file_backup(const PluginState* plugin_state);

/**
 * @brief Loads basic information from an application config file into application state without loading all the tokens
 * @param plugin_state application state
 * @return Config file open result
 */
bool totp_config_file_load(PluginState* const plugin_state);

/**
 * @brief Updates timezone offset in an application config file
 * @param plugin_state application state
 * @return Config file update result
 */
bool totp_config_file_update_timezone_offset(const PluginState* plugin_state);

/**
 * @brief Updates notification method in an application config file
 * @param plugin_state application state
 * @return Config file update result
 */
bool totp_config_file_update_notification_method(const PluginState* plugin_state);

/**
 * @brief Updates automation method in an application config file
 * @param plugin_state application state
 * @return Config file update result
 */
bool totp_config_file_update_automation_method(const PluginState* plugin_state);

/**
 * @brief Updates application user settings
 * @param plugin_state application state
 * @return Config file update result
 */
bool totp_config_file_update_user_settings(const PluginState* plugin_state);

/**
 * @brief Updates crypto signatures information
 * @param plugin_state application state
 * @return Config file update result
 */
bool totp_config_file_update_crypto_signatures(const PluginState* plugin_state);

/**
 * @brief Reset all the settings to default
 * @param plugin_state application state
 */
void totp_config_file_reset(PluginState* const plugin_state);

/**
 * @brief Closes config file and releases all the resources
 * @param plugin_state application state
 */
void totp_config_file_close(PluginState* const plugin_state);

/**
 * @brief Updates config file encryption by re-encrypting it using new user's PIN and new randomly generated IV
 * @param plugin_state application state
 * @param new_crypto_key_slot new crypto key slot to be used
 * @param new_pin new user's PIN
 * @param new_pin_length new user's PIN length
 * @return \c true if config file encryption successfully updated; \c false otherwise
 */
bool totp_config_file_update_encryption(
    PluginState* plugin_state,
    uint8_t new_crypto_key_slot,
    const uint8_t* new_pin,
    uint8_t new_pin_length);

/**
 * @brief Ensures application config file uses latest encryption and upgrades encryption if needed
 * @param plugin_state application state
 * @param pin user's PIN
 * @param pin_length user's PIN length
 * @return \c true if operation succeeded; \c false otherwise
 */
bool totp_config_file_ensure_latest_encryption(
    PluginState* plugin_state,
    const uint8_t* pin,
    uint8_t pin_length);

/**
 * @brief Gets token info iterator context
 * @param plugin_state application state
 * @return token info iterator context
 */
TokenInfoIteratorContext* totp_config_get_token_iterator_context(const PluginState* plugin_state);