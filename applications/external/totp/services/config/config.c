#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <flipper_format/flipper_format.h>
#include <furi_hal_rtc.h>
#include <flipper_format/flipper_format_i.h>
#include <flipper_format/flipper_format_stream.h>
#include <memset_s.h>
#include "../../types/common.h"
#include "../../types/token_info.h"
#include "../../features_config.h"
#include "../crypto/crypto.h"
#include "migrations/common_migration.h"

#define CONFIG_FILE_PATH CONFIG_FILE_DIRECTORY_PATH "/totp.conf"
#define CONFIG_FILE_BACKUP_DIR CONFIG_FILE_DIRECTORY_PATH "/backups"
#define CONFIG_FILE_BACKUP_BASE_PATH CONFIG_FILE_BACKUP_DIR "/totp.conf"
#define CONFIG_FILE_TEMP_PATH CONFIG_FILE_PATH ".tmp"
#define CONFIG_FILE_ORIG_PATH CONFIG_FILE_PATH ".orig"
#define CONFIG_FILE_PATH_PREVIOUS EXT_PATH("apps/Misc") "/totp.conf"

struct ConfigFileContext {
    /**
     * @brief Config file reference
     */
    FlipperFormat* config_file;

    /**
     * @brief Storage reference
     */
    Storage* storage;

    /**
     * @brief Token list iterator context 
     */
    TokenInfoIteratorContext* token_info_iterator_context;
};

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
 * @brief Tries to take a config file backup
 * @param storage storage record
 * @return backup path if backup successfully taken; \c NULL otherwise
 */
static char* totp_config_file_backup_i(Storage* storage) {
    if(!storage_dir_exists(storage, CONFIG_FILE_BACKUP_DIR) &&
       !storage_simply_mkdir(storage, CONFIG_FILE_BACKUP_DIR)) {
        return NULL;
    }

    FuriHalRtcDateTime current_datetime;
    furi_hal_rtc_get_datetime(&current_datetime);

    uint8_t backup_path_size = sizeof(CONFIG_FILE_BACKUP_BASE_PATH) + 14;
    char* backup_path = malloc(backup_path_size);
    furi_check(backup_path != NULL);
    memcpy(backup_path, CONFIG_FILE_BACKUP_BASE_PATH, sizeof(CONFIG_FILE_BACKUP_BASE_PATH));
    uint16_t i = 1;
    bool backup_file_exists;
    do {
        snprintf(
            backup_path,
            backup_path_size,
            CONFIG_FILE_BACKUP_BASE_PATH ".%4" PRIu16 "%02" PRIu8 "%02" PRIu8 "-%" PRIu16,
            current_datetime.year,
            current_datetime.month,
            current_datetime.day,
            i);
        i++;
    } while((backup_file_exists = storage_common_exists(storage, backup_path)) && i <= 9999);

    if(backup_file_exists ||
       storage_common_copy(storage, CONFIG_FILE_PATH, backup_path) != FSE_OK) {
        FURI_LOG_E(LOGGING_TAG, "Unable to take a backup");
        free(backup_path);
        return NULL;
    }

    FURI_LOG_I(LOGGING_TAG, "Took config file backup to %s", backup_path);
    return backup_path;
}

/**
 * @brief Opens or creates TOTP application standard config file
 * @param storage storage record to use
 * @param[out] file opened config file
 * @return Config file open result
 */
static bool totp_open_config_file(Storage* storage, FlipperFormat** file) {
    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);

    if(storage_common_stat(storage, CONFIG_FILE_PATH, NULL) == FSE_OK) {
        FURI_LOG_D(LOGGING_TAG, "Config file %s found", CONFIG_FILE_PATH);
        if(!flipper_format_file_open_existing(fff_data_file, CONFIG_FILE_PATH)) {
            FURI_LOG_E(LOGGING_TAG, "Error opening existing file %s", CONFIG_FILE_PATH);
            totp_close_config_file(fff_data_file);
            return false;
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
                return false;
            }
        }
        if(storage_common_rename(storage, CONFIG_FILE_PATH_PREVIOUS, CONFIG_FILE_PATH) != FSE_OK) {
            FURI_LOG_E(LOGGING_TAG, "Error moving config to %s", CONFIG_FILE_PATH);
            totp_close_config_file(fff_data_file);
            return false;
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
                return false;
            }
        }

        if(!flipper_format_file_open_new(fff_data_file, CONFIG_FILE_PATH)) {
            totp_close_config_file(fff_data_file);
            FURI_LOG_E(LOGGING_TAG, "Error creating new file %s", CONFIG_FILE_PATH);
            return false;
        }

        flipper_format_write_header_cstr(
            fff_data_file, CONFIG_FILE_HEADER, CONFIG_FILE_ACTUAL_VERSION);

        flipper_format_write_comment_cstr(
            fff_data_file,
            "Config file format specification can be found here: https://github.com/akopachov/flipper-zero_authenticator/blob/master/docs/conf-file_description.md");

        float tmp_tz = 0;
        flipper_format_write_float(fff_data_file, TOTP_CONFIG_KEY_TIMEZONE, &tmp_tz, 1);

        uint32_t tmp_uint32 = NotificationMethodSound | NotificationMethodVibro;
        flipper_format_write_uint32(
            fff_data_file, TOTP_CONFIG_KEY_NOTIFICATION_METHOD, &tmp_uint32, 1);

        tmp_uint32 = AutomationMethodBadUsb;
        flipper_format_write_uint32(
            fff_data_file, TOTP_CONFIG_KEY_AUTOMATION_METHOD, &tmp_uint32, 1);

        if(!flipper_format_rewind(fff_data_file)) {
            totp_close_config_file(fff_data_file);
            FURI_LOG_E(LOGGING_TAG, "Rewind error");
            return false;
        }
    }

    *file = fff_data_file;
    return true;
}

char* totp_config_file_backup(const PluginState* plugin_state) {
    if(plugin_state->config_file_context == NULL) return NULL;

    totp_close_config_file(plugin_state->config_file_context->config_file);

    char* result = totp_config_file_backup_i(plugin_state->config_file_context->storage);

    totp_open_config_file(
        plugin_state->config_file_context->storage,
        &plugin_state->config_file_context->config_file);

    totp_token_info_iterator_attach_to_config_file(
        plugin_state->config_file_context->token_info_iterator_context,
        plugin_state->config_file_context->config_file);

    return result;
}

bool totp_config_file_update_timezone_offset(const PluginState* plugin_state) {
    FlipperFormat* file = plugin_state->config_file_context->config_file;
    flipper_format_rewind(file);
    bool update_result = false;

    do {
        if(!flipper_format_insert_or_update_float(
               file, TOTP_CONFIG_KEY_TIMEZONE, &plugin_state->timezone_offset, 1)) {
            break;
        }

        update_result = true;
    } while(false);

    return update_result;
}

bool totp_config_file_update_notification_method(const PluginState* plugin_state) {
    FlipperFormat* file = plugin_state->config_file_context->config_file;
    flipper_format_rewind(file);
    bool update_result = false;

    do {
        uint32_t tmp_uint32 = plugin_state->notification_method;
        if(!flipper_format_insert_or_update_uint32(
               file, TOTP_CONFIG_KEY_NOTIFICATION_METHOD, &tmp_uint32, 1)) {
            break;
        }

        update_result = true;
    } while(false);

    return update_result;
}

bool totp_config_file_update_automation_method(const PluginState* plugin_state) {
    FlipperFormat* file = plugin_state->config_file_context->config_file;
    flipper_format_rewind(file);
    bool update_result = false;

    do {
        uint32_t tmp_uint32 = plugin_state->automation_method;
        if(!flipper_format_insert_or_update_uint32(
               file, TOTP_CONFIG_KEY_AUTOMATION_METHOD, &tmp_uint32, 1)) {
            break;
        }

        update_result = true;
    } while(false);

    return update_result;
}

bool totp_config_file_update_user_settings(const PluginState* plugin_state) {
    FlipperFormat* file = plugin_state->config_file_context->config_file;
    flipper_format_rewind(file);
    bool update_result = false;
    do {
        if(!flipper_format_insert_or_update_float(
               file, TOTP_CONFIG_KEY_TIMEZONE, &plugin_state->timezone_offset, 1)) {
            break;
        }
        uint32_t tmp_uint32 = plugin_state->notification_method;
        if(!flipper_format_insert_or_update_uint32(
               file, TOTP_CONFIG_KEY_NOTIFICATION_METHOD, &tmp_uint32, 1)) {
            break;
        }

        tmp_uint32 = plugin_state->automation_method;
        if(!flipper_format_insert_or_update_uint32(
               file, TOTP_CONFIG_KEY_AUTOMATION_METHOD, &tmp_uint32, 1)) {
            break;
        }

        update_result = true;
    } while(false);

    return update_result;
}

bool totp_config_file_load(PluginState* const plugin_state) {
    Storage* storage = totp_open_storage();
    FlipperFormat* fff_data_file;
    if(!totp_open_config_file(storage, &fff_data_file)) {
        totp_close_storage();
        return false;
    }

    flipper_format_rewind(fff_data_file);

    bool result = false;

    plugin_state->timezone_offset = 0;

    FuriString* temp_str = furi_string_alloc();

    do {
        uint32_t file_version;
        if(!flipper_format_read_header(fff_data_file, temp_str, &file_version)) {
            FURI_LOG_E(LOGGING_TAG, "Missing or incorrect header");
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

            char* backup_path = totp_config_file_backup_i(storage);

            if(backup_path != NULL) {
                if(totp_open_config_file(storage, &fff_data_file) != true) {
                    break;
                }

                FlipperFormat* fff_backup_data_file = flipper_format_file_alloc(storage);
                if(!flipper_format_file_open_existing(fff_backup_data_file, backup_path)) {
                    flipper_format_file_close(fff_backup_data_file);
                    flipper_format_free(fff_backup_data_file);
                    break;
                }

                if(totp_config_migrate_to_latest(fff_data_file, fff_backup_data_file)) {
                    FURI_LOG_I(
                        LOGGING_TAG,
                        "Applied migration to version %" PRId16,
                        CONFIG_FILE_ACTUAL_VERSION);
                    file_version = CONFIG_FILE_ACTUAL_VERSION;
                } else {
                    FURI_LOG_W(
                        LOGGING_TAG,
                        "An error occurred during migration to version %" PRId16,
                        CONFIG_FILE_ACTUAL_VERSION);
                    break;
                }

                flipper_format_file_close(fff_backup_data_file);
                flipper_format_free(fff_backup_data_file);
                flipper_format_rewind(fff_data_file);
                free(backup_path);
            } else {
                FURI_LOG_E(
                    LOGGING_TAG,
                    "An error occurred during taking backup of %s before migration",
                    CONFIG_FILE_PATH);
                break;
            }
        }

        if(!flipper_format_read_hex(
               fff_data_file, TOTP_CONFIG_KEY_BASE_IV, &plugin_state->base_iv[0], TOTP_IV_SIZE)) {
            FURI_LOG_D(LOGGING_TAG, "Missing base IV");
        }

        if(!flipper_format_rewind(fff_data_file)) {
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
            break;
        }

        if(!flipper_format_read_float(
               fff_data_file, TOTP_CONFIG_KEY_TIMEZONE, &plugin_state->timezone_offset, 1)) {
            plugin_state->timezone_offset = 0;
            FURI_LOG_D(LOGGING_TAG, "Missing timezone offset information, defaulting to 0");
        }

        if(!flipper_format_rewind(fff_data_file)) {
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

        flipper_format_rewind(fff_data_file);

        if(!flipper_format_read_uint32(
               fff_data_file, TOTP_CONFIG_KEY_AUTOMATION_METHOD, &tmp_uint32, 1)) {
            tmp_uint32 = AutomationMethodBadUsb;
        }

        plugin_state->automation_method = tmp_uint32;

        plugin_state->config_file_context = malloc(sizeof(ConfigFileContext));
        furi_check(plugin_state->config_file_context != NULL);
        plugin_state->config_file_context->storage = storage;
        plugin_state->config_file_context->config_file = fff_data_file;
        plugin_state->config_file_context->token_info_iterator_context =
            totp_token_info_iterator_alloc(
                storage, plugin_state->config_file_context->config_file, plugin_state->iv);
        result = true;
    } while(false);

    furi_string_free(temp_str);
    return result;
}

bool totp_config_file_update_crypto_signatures(const PluginState* plugin_state) {
    FlipperFormat* config_file = plugin_state->config_file_context->config_file;
    flipper_format_rewind(config_file);
    bool update_result = false;
    do {
        if(!flipper_format_insert_or_update_hex(
               config_file, TOTP_CONFIG_KEY_BASE_IV, plugin_state->base_iv, TOTP_IV_SIZE)) {
            break;
        }

        if(!flipper_format_insert_or_update_hex(
               config_file,
               TOTP_CONFIG_KEY_CRYPTO_VERIFY,
               plugin_state->crypto_verify_data,
               plugin_state->crypto_verify_data_length)) {
            break;
        }

        if(!flipper_format_insert_or_update_bool(
               config_file, TOTP_CONFIG_KEY_PINSET, &plugin_state->pin_set, 1)) {
            break;
        }

        update_result = true;
    } while(false);

    return update_result;
}

void totp_config_file_close(PluginState* const plugin_state) {
    if(plugin_state->config_file_context == NULL) return;
    totp_token_info_iterator_free(plugin_state->config_file_context->token_info_iterator_context);
    totp_close_config_file(plugin_state->config_file_context->config_file);
    free(plugin_state->config_file_context);
    plugin_state->config_file_context = NULL;
    totp_close_storage();
}

void totp_config_file_reset(PluginState* const plugin_state) {
    totp_config_file_close(plugin_state);
    Storage* storage = totp_open_storage();
    storage_simply_remove(storage, CONFIG_FILE_PATH);
    totp_close_storage();
}

bool totp_config_file_update_encryption(
    PluginState* plugin_state,
    const uint8_t* new_pin,
    uint8_t new_pin_length) {
    FlipperFormat* config_file = plugin_state->config_file_context->config_file;
    Stream* stream = flipper_format_get_raw_stream(config_file);
    size_t original_offset = stream_tell(stream);
    if(!stream_rewind(stream)) {
        return false;
    }

    uint8_t old_iv[TOTP_IV_SIZE];
    memcpy(&old_iv[0], &plugin_state->iv[0], TOTP_IV_SIZE);

    memset(&plugin_state->iv[0], 0, TOTP_IV_SIZE);
    memset(&plugin_state->base_iv[0], 0, TOTP_IV_SIZE);
    if(plugin_state->crypto_verify_data != NULL) {
        free(plugin_state->crypto_verify_data);
        plugin_state->crypto_verify_data = NULL;
    }

    CryptoSeedIVResult seed_result =
        totp_crypto_seed_iv(plugin_state, new_pin_length > 0 ? new_pin : NULL, new_pin_length);
    if(seed_result & CryptoSeedIVResultFlagSuccess &&
       seed_result & CryptoSeedIVResultFlagNewCryptoVerifyData) {
        if(!totp_config_file_update_crypto_signatures(plugin_state)) {
            return false;
        }
    } else if(seed_result == CryptoSeedIVResultFailed) {
        return false;
    }

    char buffer[sizeof(TOTP_CONFIG_KEY_TOKEN_SECRET) + 1];
    bool result = true;

    while(true) {
        if(!stream_seek_to_char(stream, '\n', StreamDirectionForward)) {
            break;
        }

        size_t buffer_read_size;
        if((buffer_read_size = stream_read(stream, (uint8_t*)&buffer[0], sizeof(buffer))) == 0) {
            break;
        }

        if(!stream_seek(stream, -(int32_t)buffer_read_size, StreamOffsetFromCurrent)) {
            result = false;
            break;
        }

        if(strncmp(buffer, "\n" TOTP_CONFIG_KEY_TOKEN_SECRET ":", sizeof(buffer)) == 0) {
            uint32_t secret_bytes_count;
            if(!flipper_format_get_value_count(
                   config_file, TOTP_CONFIG_KEY_TOKEN_SECRET, &secret_bytes_count)) {
                secret_bytes_count = 0;
            }

            if(secret_bytes_count > 1) {
                size_t secret_token_start = stream_tell(stream) + 1;
                uint8_t* encrypted_token = malloc(secret_bytes_count);
                furi_check(encrypted_token != NULL);

                if(!flipper_format_read_hex(
                       config_file,
                       TOTP_CONFIG_KEY_TOKEN_SECRET,
                       encrypted_token,
                       secret_bytes_count)) {
                    result = false;
                    free(encrypted_token);
                    break;
                }

                size_t plain_token_length;
                uint8_t* plain_token = totp_crypto_decrypt(
                    encrypted_token, secret_bytes_count, &old_iv[0], &plain_token_length);

                free(encrypted_token);
                size_t encrypted_token_length;
                encrypted_token = totp_crypto_encrypt(
                    plain_token, plain_token_length, &plugin_state->iv[0], &encrypted_token_length);

                memset_s(plain_token, plain_token_length, 0, plain_token_length);
                free(plain_token);

                if(!stream_seek(stream, secret_token_start, StreamOffsetFromStart)) {
                    result = false;
                    free(encrypted_token);
                    break;
                }

                if(!flipper_format_write_hex(
                       config_file,
                       TOTP_CONFIG_KEY_TOKEN_SECRET,
                       encrypted_token,
                       encrypted_token_length)) {
                    free(encrypted_token);
                    result = false;
                    break;
                }

                free(encrypted_token);
            }
        }
    }

    stream_seek(stream, original_offset, StreamOffsetFromStart);

    return result;
}

TokenInfoIteratorContext* totp_config_get_token_iterator_context(const PluginState* plugin_state) {
    return plugin_state->config_file_context->token_info_iterator_context;
}