#include "config.h"
#include <stdlib.h>
#include <string.h>
#include "../list/list.h"
#include "../../types/common.h"
#include "../../types/token_info.h"
#include "migrations/config_migration_v1_to_v2.h"

#define CONFIG_FILE_DIRECTORY_PATH EXT_PATH("apps_data/authenticator")
#define CONFIG_FILE_PATH CONFIG_FILE_DIRECTORY_PATH "/totp.conf"
#define CONFIG_FILE_BACKUP_PATH CONFIG_FILE_PATH ".backup"
#define CONFIG_FILE_TEMP_PATH CONFIG_FILE_PATH ".tmp"
#define CONFIG_FILE_ORIG_PATH CONFIG_FILE_PATH ".orig"
#define CONFIG_FILE_PATH_PREVIOUS EXT_PATH("apps/Misc") "/totp.conf"

static char* token_info_get_algo_as_cstr(const TokenInfo* token_info) {
    switch(token_info->algo) {
    case SHA1:
        return TOTP_CONFIG_TOKEN_ALGO_SHA1_NAME;
    case SHA256:
        return TOTP_CONFIG_TOKEN_ALGO_SHA256_NAME;
    case SHA512:
        return TOTP_CONFIG_TOKEN_ALGO_SHA512_NAME;
    default:
        break;
    }

    return NULL;
}

static void token_info_set_algo_from_str(TokenInfo* token_info, const FuriString* str) {
    if(furi_string_cmpi_str(str, TOTP_CONFIG_TOKEN_ALGO_SHA1_NAME) == 0) {
        token_info->algo = SHA1;
    } else if(furi_string_cmpi_str(str, TOTP_CONFIG_TOKEN_ALGO_SHA256_NAME) == 0) {
        token_info->algo = SHA256;
    } else if(furi_string_cmpi_str(str, TOTP_CONFIG_TOKEN_ALGO_SHA512_NAME) == 0) {
        token_info->algo = SHA512;
    }
}

/**
 * @brief Opens storage record
 * @return Storage record
 */
static Storage* totp_open_storage() {
    return furi_record_open(RECORD_STORAGE);
}

/**
 * @brief Closes storage record
 */
static void totp_close_storage() {
    furi_record_close(RECORD_STORAGE);
}

/**
 * @brief Closes config file
 * @param file config file reference
 */
static void totp_close_config_file(FlipperFormat* file) {
    if(file == NULL) return;
    flipper_format_file_close(file);
    flipper_format_free(file);
}

/**
 * @brief Opens or creates TOTP application standard config file
 * @param storage storage record to use
 * @param[out] file opened config file
 * @return Config file open result
 */
static TotpConfigFileOpenResult totp_open_config_file(Storage* storage, FlipperFormat** file) {
    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);

    if(storage_common_stat(storage, CONFIG_FILE_PATH, NULL) == FSE_OK) {
        FURI_LOG_D(LOGGING_TAG, "Config file %s found", CONFIG_FILE_PATH);
        if(!flipper_format_file_open_existing(fff_data_file, CONFIG_FILE_PATH)) {
            FURI_LOG_E(LOGGING_TAG, "Error opening existing file %s", CONFIG_FILE_PATH);
            totp_close_config_file(fff_data_file);
            return TotpConfigFileOpenError;
        }
    } else if(storage_common_stat(storage, CONFIG_FILE_PATH_PREVIOUS, NULL) == FSE_OK) {
        FURI_LOG_D(LOGGING_TAG, "Old config file %s found", CONFIG_FILE_PATH_PREVIOUS);
        if(storage_common_stat(storage, CONFIG_FILE_DIRECTORY_PATH, NULL) == FSE_NOT_EXIST) {
            FURI_LOG_D(
                LOGGING_TAG,
                "Directory %s doesn't exist. Will create new.",
                CONFIG_FILE_DIRECTORY_PATH);
            if(!storage_simply_mkdir(storage, CONFIG_FILE_DIRECTORY_PATH)) {
                FURI_LOG_E(LOGGING_TAG, "Error creating directory %s", CONFIG_FILE_DIRECTORY_PATH);
                totp_close_config_file(fff_data_file);
                return TotpConfigFileOpenError;
            }
        }
        if(storage_common_rename(storage, CONFIG_FILE_PATH_PREVIOUS, CONFIG_FILE_PATH) != FSE_OK) {
            FURI_LOG_E(LOGGING_TAG, "Error moving config to %s", CONFIG_FILE_PATH);
            totp_close_config_file(fff_data_file);
            return TotpConfigFileOpenError;
        }
        FURI_LOG_I(LOGGING_TAG, "Applied config file path migration");
        return totp_open_config_file(storage, file);
    } else {
        FURI_LOG_D(LOGGING_TAG, "Config file %s is not found. Will create new.", CONFIG_FILE_PATH);
        if(storage_common_stat(storage, CONFIG_FILE_DIRECTORY_PATH, NULL) == FSE_NOT_EXIST) {
            FURI_LOG_D(
                LOGGING_TAG,
                "Directory %s doesn't exist. Will create new.",
                CONFIG_FILE_DIRECTORY_PATH);
            if(!storage_simply_mkdir(storage, CONFIG_FILE_DIRECTORY_PATH)) {
                FURI_LOG_E(LOGGING_TAG, "Error creating directory %s", CONFIG_FILE_DIRECTORY_PATH);
                return TotpConfigFileOpenError;
            }
        }

        if(!flipper_format_file_open_new(fff_data_file, CONFIG_FILE_PATH)) {
            totp_close_config_file(fff_data_file);
            FURI_LOG_E(LOGGING_TAG, "Error creating new file %s", CONFIG_FILE_PATH);
            return TotpConfigFileOpenError;
        }

        flipper_format_write_header_cstr(
            fff_data_file, CONFIG_FILE_HEADER, CONFIG_FILE_ACTUAL_VERSION);
        float tmp_tz = 0;
        flipper_format_write_comment_cstr(fff_data_file, " ");
        flipper_format_write_comment_cstr(
            fff_data_file,
            "Timezone offset in hours. Important note: do not put '+' sign for positive values");
        flipper_format_write_float(fff_data_file, TOTP_CONFIG_KEY_TIMEZONE, &tmp_tz, 1);

        uint32_t tmp_uint32 = NotificationMethodSound | NotificationMethodVibro;
        flipper_format_write_comment_cstr(fff_data_file, " ");
        flipper_format_write_comment_cstr(
            fff_data_file,
            "How to notify user when new token is generated or badusb mode is activated (possible values: 0 - do not notify, 1 - sound, 2 - vibro, 3 sound and vibro)");
        flipper_format_write_uint32(
            fff_data_file, TOTP_CONFIG_KEY_NOTIFICATION_METHOD, &tmp_uint32, 1);

        FuriString* temp_str = furi_string_alloc();

        flipper_format_write_comment_cstr(fff_data_file, " ");
        flipper_format_write_comment_cstr(fff_data_file, "=== TOKEN SAMPLE BEGIN ===");
        flipper_format_write_comment_cstr(fff_data_file, " ");
        flipper_format_write_comment_cstr(
            fff_data_file, "# Token name which will be visible in the UI.");
        furi_string_printf(temp_str, "%s: Sample token name", TOTP_CONFIG_KEY_TOKEN_NAME);
        flipper_format_write_comment(fff_data_file, temp_str);
        flipper_format_write_comment_cstr(fff_data_file, " ");

        flipper_format_write_comment_cstr(
            fff_data_file,
            "# Plain token secret without spaces, dashes and etc, just pure alpha-numeric characters. Important note: plain token will be encrypted and replaced by TOTP app");
        furi_string_printf(temp_str, "%s: plaintokensecret", TOTP_CONFIG_KEY_TOKEN_SECRET);
        flipper_format_write_comment(fff_data_file, temp_str);
        flipper_format_write_comment_cstr(fff_data_file, " ");

        furi_string_printf(
            temp_str,
            " # Token hashing algorithm to use during code generation. Supported options are %s, %s and %s. If you are not use which one to use - use %s",
            TOTP_CONFIG_TOKEN_ALGO_SHA1_NAME,
            TOTP_CONFIG_TOKEN_ALGO_SHA256_NAME,
            TOTP_CONFIG_TOKEN_ALGO_SHA512_NAME,
            TOTP_CONFIG_TOKEN_ALGO_SHA1_NAME);
        flipper_format_write_comment(fff_data_file, temp_str);
        furi_string_printf(
            temp_str, "%s: %s", TOTP_CONFIG_KEY_TOKEN_ALGO, TOTP_CONFIG_TOKEN_ALGO_SHA1_NAME);
        flipper_format_write_comment(fff_data_file, temp_str);
        flipper_format_write_comment_cstr(fff_data_file, " ");

        flipper_format_write_comment_cstr(
            fff_data_file,
            "# How many digits there should be in generated code. Available options are 6 and 8. Majority websites requires 6 digits code, however some rare websites wants to get 8 digits code. If you are not sure which one to use - use 6");
        furi_string_printf(temp_str, "%s: 6", TOTP_CONFIG_KEY_TOKEN_DIGITS);
        flipper_format_write_comment(fff_data_file, temp_str);
        flipper_format_write_comment_cstr(fff_data_file, " ");

        flipper_format_write_comment_cstr(fff_data_file, "=== TOKEN SAMPLE END ===");
        flipper_format_write_comment_cstr(fff_data_file, " ");

        furi_string_free(temp_str);
        if(!flipper_format_rewind(fff_data_file)) {
            totp_close_config_file(fff_data_file);
            FURI_LOG_E(LOGGING_TAG, "Rewind error");
            return TotpConfigFileOpenError;
        }
    }

    *file = fff_data_file;
    return TotpConfigFileOpenSuccess;
}

TotpConfigFileUpdateResult
    totp_config_file_save_new_token_i(FlipperFormat* file, const TokenInfo* token_info) {
    TotpConfigFileUpdateResult update_result;
    do {
        if(!flipper_format_seek_to_end(file)) {
            update_result = TotpConfigFileUpdateError;
            break;
        }

        if(!flipper_format_write_string_cstr(file, TOTP_CONFIG_KEY_TOKEN_NAME, token_info->name)) {
            update_result = TotpConfigFileUpdateError;
            break;
        }

        bool token_is_valid = token_info->token != NULL && token_info->token_length > 0;
        if(!token_is_valid &&
           !flipper_format_write_comment_cstr(file, "!!! WARNING BEGIN: INVALID TOKEN !!!")) {
            update_result = TotpConfigFileUpdateError;
            break;
        }

        if(!flipper_format_write_hex(
               file, TOTP_CONFIG_KEY_TOKEN_SECRET, token_info->token, token_info->token_length)) {
            update_result = TotpConfigFileUpdateError;
            break;
        }

        if(!token_is_valid && !flipper_format_write_comment_cstr(file, "!!! WARNING END !!!")) {
            update_result = TotpConfigFileUpdateError;
            break;
        }

        if(!flipper_format_write_string_cstr(
               file, TOTP_CONFIG_KEY_TOKEN_ALGO, token_info_get_algo_as_cstr(token_info))) {
            update_result = TotpConfigFileUpdateError;
            break;
        }

        uint32_t tmp_uint32 = token_info->digits;
        if(!flipper_format_write_uint32(file, TOTP_CONFIG_KEY_TOKEN_DIGITS, &tmp_uint32, 1)) {
            update_result = TotpConfigFileUpdateError;
            break;
        }

        update_result = TotpConfigFileUpdateSuccess;
    } while(false);

    return update_result;
}

TotpConfigFileUpdateResult totp_config_file_save_new_token(const TokenInfo* token_info) {
    Storage* cfg_storage = totp_open_storage();
    FlipperFormat* file;
    TotpConfigFileUpdateResult update_result;

    if(totp_open_config_file(cfg_storage, &file) == TotpConfigFileOpenSuccess) {
        do {
            if(totp_config_file_save_new_token_i(file, token_info) !=
               TotpConfigFileUpdateSuccess) {
                update_result = TotpConfigFileUpdateError;
                break;
            }

            update_result = TotpConfigFileUpdateSuccess;
        } while(false);

        totp_close_config_file(file);
    } else {
        update_result = TotpConfigFileUpdateError;
    }

    totp_close_storage();
    return update_result;
}

TotpConfigFileUpdateResult totp_config_file_update_timezone_offset(float new_timezone_offset) {
    Storage* cfg_storage = totp_open_storage();
    FlipperFormat* file;
    TotpConfigFileUpdateResult update_result;

    if(totp_open_config_file(cfg_storage, &file) == TotpConfigFileOpenSuccess) {
        do {
            if(!flipper_format_insert_or_update_float(
                   file, TOTP_CONFIG_KEY_TIMEZONE, &new_timezone_offset, 1)) {
                update_result = TotpConfigFileUpdateError;
                break;
            }

            update_result = TotpConfigFileUpdateSuccess;
        } while(false);

        totp_close_config_file(file);
    } else {
        update_result = TotpConfigFileUpdateError;
    }

    totp_close_storage();
    return update_result;
}

TotpConfigFileUpdateResult
    totp_config_file_update_notification_method(NotificationMethod new_notification_method) {
    Storage* cfg_storage = totp_open_storage();
    FlipperFormat* file;
    TotpConfigFileUpdateResult update_result;

    if(totp_open_config_file(cfg_storage, &file) == TotpConfigFileOpenSuccess) {
        do {
            uint32_t tmp_uint32 = new_notification_method;
            if(!flipper_format_insert_or_update_uint32(
                   file, TOTP_CONFIG_KEY_NOTIFICATION_METHOD, &tmp_uint32, 1)) {
                update_result = TotpConfigFileUpdateError;
                break;
            }

            update_result = TotpConfigFileUpdateSuccess;
        } while(false);

        totp_close_config_file(file);
    } else {
        update_result = TotpConfigFileUpdateError;
    }

    totp_close_storage();
    return update_result;
}

TotpConfigFileUpdateResult totp_config_file_update_user_settings(const PluginState* plugin_state) {
    Storage* cfg_storage = totp_open_storage();
    FlipperFormat* file;
    TotpConfigFileUpdateResult update_result;
    if(totp_open_config_file(cfg_storage, &file) == TotpConfigFileOpenSuccess) {
        do {
            if(!flipper_format_insert_or_update_float(
                   file, TOTP_CONFIG_KEY_TIMEZONE, &plugin_state->timezone_offset, 1)) {
                update_result = TotpConfigFileUpdateError;
                break;
            }
            uint32_t tmp_uint32 = plugin_state->notification_method;
            if(!flipper_format_insert_or_update_uint32(
                   file, TOTP_CONFIG_KEY_NOTIFICATION_METHOD, &tmp_uint32, 1)) {
                update_result = TotpConfigFileUpdateError;
                break;
            }

            update_result = TotpConfigFileUpdateSuccess;
        } while(false);

        totp_close_config_file(file);
    } else {
        update_result = TotpConfigFileUpdateError;
    }

    totp_close_storage();
    return update_result;
}

TotpConfigFileUpdateResult totp_full_save_config_file(const PluginState* const plugin_state) {
    Storage* storage = totp_open_storage();
    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);
    TotpConfigFileUpdateResult result = TotpConfigFileUpdateSuccess;

    do {
        if(!flipper_format_file_open_always(fff_data_file, CONFIG_FILE_TEMP_PATH)) {
            result = TotpConfigFileUpdateError;
            break;
        }

        if(!flipper_format_write_header_cstr(
               fff_data_file, CONFIG_FILE_HEADER, CONFIG_FILE_ACTUAL_VERSION)) {
            result = TotpConfigFileUpdateError;
            break;
        }

        if(!flipper_format_write_hex(
               fff_data_file, TOTP_CONFIG_KEY_BASE_IV, &plugin_state->base_iv[0], TOTP_IV_SIZE)) {
            result = TotpConfigFileUpdateError;
            break;
        }

        if(!flipper_format_write_hex(
               fff_data_file,
               TOTP_CONFIG_KEY_CRYPTO_VERIFY,
               plugin_state->crypto_verify_data,
               plugin_state->crypto_verify_data_length)) {
            result = TotpConfigFileUpdateError;
            break;
        }

        if(!flipper_format_write_float(
               fff_data_file, TOTP_CONFIG_KEY_TIMEZONE, &plugin_state->timezone_offset, 1)) {
            result = TotpConfigFileUpdateError;
            break;
        }

        if(!flipper_format_write_bool(
               fff_data_file, TOTP_CONFIG_KEY_PINSET, &plugin_state->pin_set, 1)) {
            result = TotpConfigFileUpdateError;
            break;
        }
        uint32_t tmp_uint32 = plugin_state->notification_method;
        if(!flipper_format_write_uint32(
               fff_data_file, TOTP_CONFIG_KEY_NOTIFICATION_METHOD, &tmp_uint32, 1)) {
            result = TotpConfigFileUpdateError;
            break;
        }

        bool tokens_written = true;
        TOTP_LIST_FOREACH(plugin_state->tokens_list, node, {
            const TokenInfo* token_info = node->data;
            tokens_written = tokens_written &&
                             totp_config_file_save_new_token_i(fff_data_file, token_info) ==
                                 TotpConfigFileUpdateSuccess;
        });

        if(!tokens_written) {
            result = TotpConfigFileUpdateError;
            break;
        }
    } while(false);

    totp_close_config_file(fff_data_file);

    if(result == TotpConfigFileUpdateSuccess) {
        if(storage_file_exists(storage, CONFIG_FILE_ORIG_PATH)) {
            storage_simply_remove(storage, CONFIG_FILE_ORIG_PATH);
        }

        if(storage_common_rename(storage, CONFIG_FILE_PATH, CONFIG_FILE_ORIG_PATH) != FSE_OK) {
            result = TotpConfigFileUpdateError;
        } else if(storage_common_rename(storage, CONFIG_FILE_TEMP_PATH, CONFIG_FILE_PATH) != FSE_OK) {
            result = TotpConfigFileUpdateError;
        } else if(!storage_simply_remove(storage, CONFIG_FILE_ORIG_PATH)) {
            result = TotpConfigFileUpdateError;
        }
    }

    totp_close_storage();
    return result;
}

TotpConfigFileOpenResult totp_config_file_load_base(PluginState* const plugin_state) {
    Storage* storage = totp_open_storage();
    FlipperFormat* fff_data_file;

    TotpConfigFileOpenResult result;
    if((result = totp_open_config_file(storage, &fff_data_file)) != TotpConfigFileOpenSuccess) {
        totp_close_storage();
        return result;
    }

    plugin_state->timezone_offset = 0;

    FuriString* temp_str = furi_string_alloc();

    do {
        uint32_t file_version;
        if(!flipper_format_read_header(fff_data_file, temp_str, &file_version)) {
            FURI_LOG_E(LOGGING_TAG, "Missing or incorrect header");
            result = TotpConfigFileOpenError;
            break;
        }

        if(file_version < CONFIG_FILE_ACTUAL_VERSION) {
            FURI_LOG_I(
                LOGGING_TAG,
                "Obsolete config file version detected. Current version: %" PRIu32
                "; Actual version: %" PRId16,
                file_version,
                CONFIG_FILE_ACTUAL_VERSION);
            totp_close_config_file(fff_data_file);

            if(storage_common_stat(storage, CONFIG_FILE_BACKUP_PATH, NULL) == FSE_OK) {
                storage_simply_remove(storage, CONFIG_FILE_BACKUP_PATH);
            }

            if(storage_common_copy(storage, CONFIG_FILE_PATH, CONFIG_FILE_BACKUP_PATH) == FSE_OK) {
                FURI_LOG_I(LOGGING_TAG, "Took config file backup to %s", CONFIG_FILE_BACKUP_PATH);
                if(totp_open_config_file(storage, &fff_data_file) != TotpConfigFileOpenSuccess) {
                    result = TotpConfigFileOpenError;
                    break;
                }

                FlipperFormat* fff_backup_data_file = flipper_format_file_alloc(storage);
                if(!flipper_format_file_open_existing(
                       fff_backup_data_file, CONFIG_FILE_BACKUP_PATH)) {
                    flipper_format_file_close(fff_backup_data_file);
                    flipper_format_free(fff_backup_data_file);
                    result = TotpConfigFileOpenError;
                    break;
                }

                if(file_version == 1) {
                    if(totp_config_migrate_v1_to_v2(fff_data_file, fff_backup_data_file)) {
                        FURI_LOG_I(LOGGING_TAG, "Applied migration from v1 to v2");
                    } else {
                        FURI_LOG_W(
                            LOGGING_TAG, "An error occurred during migration from v1 to v2");
                        result = TotpConfigFileOpenError;
                        break;
                    }
                }

                flipper_format_file_close(fff_backup_data_file);
                flipper_format_free(fff_backup_data_file);
                flipper_format_rewind(fff_data_file);
            } else {
                FURI_LOG_E(
                    LOGGING_TAG,
                    "An error occurred during taking backup of %s into %s before migration",
                    CONFIG_FILE_PATH,
                    CONFIG_FILE_BACKUP_PATH);
                result = TotpConfigFileOpenError;
                break;
            }
        }

        if(!flipper_format_read_hex(
               fff_data_file, TOTP_CONFIG_KEY_BASE_IV, &plugin_state->base_iv[0], TOTP_IV_SIZE)) {
            FURI_LOG_D(LOGGING_TAG, "Missing base IV");
        }

        if(!flipper_format_rewind(fff_data_file)) {
            result = TotpConfigFileOpenError;
            break;
        }

        uint32_t crypto_size;
        if(flipper_format_get_value_count(
               fff_data_file, TOTP_CONFIG_KEY_CRYPTO_VERIFY, &crypto_size) &&
           crypto_size > 0) {
            plugin_state->crypto_verify_data = malloc(sizeof(uint8_t) * crypto_size);
            furi_check(plugin_state->crypto_verify_data != NULL);
            plugin_state->crypto_verify_data_length = crypto_size;
            if(!flipper_format_read_hex(
                   fff_data_file,
                   TOTP_CONFIG_KEY_CRYPTO_VERIFY,
                   plugin_state->crypto_verify_data,
                   crypto_size)) {
                FURI_LOG_D(LOGGING_TAG, "Missing crypto verify token");
                free(plugin_state->crypto_verify_data);
                plugin_state->crypto_verify_data = NULL;
                plugin_state->crypto_verify_data_length = 0;
            }
        } else {
            plugin_state->crypto_verify_data = NULL;
            plugin_state->crypto_verify_data_length = 0;
        }

        if(!flipper_format_rewind(fff_data_file)) {
            result = TotpConfigFileOpenError;
            break;
        }

        if(!flipper_format_read_float(
               fff_data_file, TOTP_CONFIG_KEY_TIMEZONE, &plugin_state->timezone_offset, 1)) {
            plugin_state->timezone_offset = 0;
            FURI_LOG_D(LOGGING_TAG, "Missing timezone offset information, defaulting to 0");
        }

        if(!flipper_format_rewind(fff_data_file)) {
            result = TotpConfigFileOpenError;
            break;
        }

        if(!flipper_format_read_bool(
               fff_data_file, TOTP_CONFIG_KEY_PINSET, &plugin_state->pin_set, 1)) {
            plugin_state->pin_set = true;
        }

        flipper_format_rewind(fff_data_file);

        uint32_t tmp_uint32;
        if(!flipper_format_read_uint32(
               fff_data_file, TOTP_CONFIG_KEY_NOTIFICATION_METHOD, &tmp_uint32, 1)) {
            tmp_uint32 = NotificationMethodSound | NotificationMethodVibro;
        }

        plugin_state->notification_method = tmp_uint32;
    } while(false);

    furi_string_free(temp_str);
    totp_close_config_file(fff_data_file);
    totp_close_storage();
    return result;
}

TokenLoadingResult totp_config_file_load_tokens(PluginState* const plugin_state) {
    Storage* storage = totp_open_storage();
    FlipperFormat* fff_data_file;
    if(totp_open_config_file(storage, &fff_data_file) != TotpConfigFileOpenSuccess) {
        totp_close_storage();
        return TokenLoadingResultError;
    }

    FuriString* temp_str = furi_string_alloc();
    uint32_t temp_data32;

    if(!flipper_format_read_header(fff_data_file, temp_str, &temp_data32)) {
        FURI_LOG_E(LOGGING_TAG, "Missing or incorrect header");
        totp_close_storage();
        furi_string_free(temp_str);
        return TokenLoadingResultError;
    }

    TokenLoadingResult result = TokenLoadingResultSuccess;
    uint16_t index = 0;
    bool has_any_plain_secret = false;

    while(true) {
        if(!flipper_format_read_string(fff_data_file, TOTP_CONFIG_KEY_TOKEN_NAME, temp_str)) {
            break;
        }

        TokenInfo* tokenInfo = token_info_alloc();

        size_t temp_cstr_len = furi_string_size(temp_str);
        tokenInfo->name = malloc(temp_cstr_len + 1);
        furi_check(tokenInfo->name != NULL);
        strlcpy(tokenInfo->name, furi_string_get_cstr(temp_str), temp_cstr_len + 1);

        uint32_t secret_bytes_count;
        if(!flipper_format_get_value_count(
               fff_data_file, TOTP_CONFIG_KEY_TOKEN_SECRET, &secret_bytes_count)) {
            secret_bytes_count = 0;
        }

        if(secret_bytes_count == 1) { // Plain secret key
            if(flipper_format_read_string(fff_data_file, TOTP_CONFIG_KEY_TOKEN_SECRET, temp_str)) {
                if(token_info_set_secret(
                       tokenInfo,
                       furi_string_get_cstr(temp_str),
                       furi_string_size(temp_str),
                       &plugin_state->iv[0])) {
                    FURI_LOG_W(LOGGING_TAG, "Token \"%s\" has plain secret", tokenInfo->name);
                } else {
                    tokenInfo->token = NULL;
                    tokenInfo->token_length = 0;
                    FURI_LOG_W(LOGGING_TAG, "Token \"%s\" has invalid secret", tokenInfo->name);
                    result = TokenLoadingResultWarning;
                }
            } else {
                tokenInfo->token = NULL;
                tokenInfo->token_length = 0;
                result = TokenLoadingResultWarning;
            }

            has_any_plain_secret = true;
        } else { // encrypted
            tokenInfo->token_length = secret_bytes_count;
            if(secret_bytes_count > 0) {
                tokenInfo->token = malloc(tokenInfo->token_length);
                furi_check(tokenInfo->token != NULL);
                if(!flipper_format_read_hex(
                       fff_data_file,
                       TOTP_CONFIG_KEY_TOKEN_SECRET,
                       tokenInfo->token,
                       tokenInfo->token_length)) {
                    free(tokenInfo->token);
                    tokenInfo->token = NULL;
                    tokenInfo->token_length = 0;
                    result = TokenLoadingResultWarning;
                }
            } else {
                tokenInfo->token = NULL;
                result = TokenLoadingResultWarning;
            }
        }

        if(flipper_format_read_string(fff_data_file, TOTP_CONFIG_KEY_TOKEN_ALGO, temp_str)) {
            token_info_set_algo_from_str(tokenInfo, temp_str);
        } else {
            tokenInfo->algo = SHA1;
        }

        if(!flipper_format_read_uint32(
               fff_data_file, TOTP_CONFIG_KEY_TOKEN_DIGITS, &temp_data32, 1) ||
           !token_info_set_digits_from_int(tokenInfo, temp_data32)) {
            tokenInfo->digits = TOTP_6_DIGITS;
        }

        FURI_LOG_D(LOGGING_TAG, "Found token \"%s\"", tokenInfo->name);

        TOTP_LIST_INIT_OR_ADD(plugin_state->tokens_list, tokenInfo, furi_check);

        index++;
    }

    plugin_state->tokens_count = index;
    plugin_state->token_list_loaded = true;

    FURI_LOG_D(LOGGING_TAG, "Found %" PRIu16 " tokens", index);

    furi_string_free(temp_str);
    totp_close_config_file(fff_data_file);
    totp_close_storage();

    if(has_any_plain_secret) {
        totp_full_save_config_file(plugin_state);
    }

    return result;
}

TotpConfigFileUpdateResult
    totp_config_file_update_crypto_signatures(const PluginState* plugin_state) {
    Storage* storage = totp_open_storage();
    FlipperFormat* config_file;
    TotpConfigFileUpdateResult update_result;
    if(totp_open_config_file(storage, &config_file) == TotpConfigFileOpenSuccess) {
        do {
            if(!flipper_format_insert_or_update_hex(
                   config_file, TOTP_CONFIG_KEY_BASE_IV, plugin_state->base_iv, TOTP_IV_SIZE)) {
                update_result = TotpConfigFileUpdateError;
                break;
            }

            if(!flipper_format_insert_or_update_hex(
                   config_file,
                   TOTP_CONFIG_KEY_CRYPTO_VERIFY,
                   plugin_state->crypto_verify_data,
                   plugin_state->crypto_verify_data_length)) {
                update_result = TotpConfigFileUpdateError;
                break;
            }

            if(!flipper_format_insert_or_update_bool(
                   config_file, TOTP_CONFIG_KEY_PINSET, &plugin_state->pin_set, 1)) {
                update_result = TotpConfigFileUpdateError;
                break;
            }

            update_result = TotpConfigFileUpdateSuccess;
        } while(false);

        totp_close_config_file(config_file);
    } else {
        update_result = TotpConfigFileUpdateError;
    }

    totp_close_storage();
    return update_result;
}