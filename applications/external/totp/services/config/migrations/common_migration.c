#include "common_migration.h"
#include "../constants.h"
#include "../../../types/token_info.h"
#include <flipper_format/flipper_format_i.h>

bool totp_config_migrate_to_latest(
    FlipperFormat* fff_data_file,
    FlipperFormat* fff_backup_data_file) {
    FuriString* temp_str = furi_string_alloc();
    uint32_t current_version = 0;
    bool result = false;
    do {
        flipper_format_write_header_cstr(
            fff_data_file, CONFIG_FILE_HEADER, CONFIG_FILE_ACTUAL_VERSION);

        if(!flipper_format_read_header(fff_backup_data_file, temp_str, &current_version)) {
            break;
        }

        if(flipper_format_read_string(fff_backup_data_file, TOTP_CONFIG_KEY_BASE_IV, temp_str)) {
            flipper_format_write_string(fff_data_file, TOTP_CONFIG_KEY_BASE_IV, temp_str);
        }

        flipper_format_rewind(fff_backup_data_file);

        if(flipper_format_read_string(
               fff_backup_data_file, TOTP_CONFIG_KEY_CRYPTO_VERIFY, temp_str)) {
            flipper_format_write_string(fff_data_file, TOTP_CONFIG_KEY_CRYPTO_VERIFY, temp_str);
        }

        flipper_format_rewind(fff_backup_data_file);

        if(flipper_format_read_string(fff_backup_data_file, TOTP_CONFIG_KEY_TIMEZONE, temp_str)) {
            flipper_format_write_string(fff_data_file, TOTP_CONFIG_KEY_TIMEZONE, temp_str);
        }

        flipper_format_rewind(fff_backup_data_file);

        if(flipper_format_read_string(fff_backup_data_file, TOTP_CONFIG_KEY_PINSET, temp_str)) {
            flipper_format_write_string(fff_data_file, TOTP_CONFIG_KEY_PINSET, temp_str);
        }

        flipper_format_rewind(fff_backup_data_file);

        if(flipper_format_read_string(
               fff_backup_data_file, TOTP_CONFIG_KEY_NOTIFICATION_METHOD, temp_str)) {
            flipper_format_write_string(
                fff_data_file, TOTP_CONFIG_KEY_NOTIFICATION_METHOD, temp_str);
        }

        flipper_format_rewind(fff_backup_data_file);

        if(flipper_format_read_string(
               fff_backup_data_file, TOTP_CONFIG_KEY_AUTOMATION_METHOD, temp_str)) {
            flipper_format_write_string(
                fff_data_file, TOTP_CONFIG_KEY_AUTOMATION_METHOD, temp_str);
        }

        flipper_format_rewind(fff_backup_data_file);

        // Font

        if(flipper_format_read_string(
               fff_backup_data_file, TOTP_CONFIG_KEY_SELECTED_FONT, temp_str)) {
            flipper_format_write_string(fff_data_file, TOTP_CONFIG_KEY_SELECTED_FONT, temp_str);
        }

        flipper_format_rewind(fff_backup_data_file);

        while(true) {
            if(!flipper_format_read_string(
                   fff_backup_data_file, TOTP_CONFIG_KEY_TOKEN_NAME, temp_str)) {
                break;
            }

            flipper_format_write_string(fff_data_file, TOTP_CONFIG_KEY_TOKEN_NAME, temp_str);

            flipper_format_read_string(
                fff_backup_data_file, TOTP_CONFIG_KEY_TOKEN_SECRET, temp_str);
            flipper_format_write_string(fff_data_file, TOTP_CONFIG_KEY_TOKEN_SECRET, temp_str);

            if(current_version > 1) {
                flipper_format_read_string(
                    fff_backup_data_file, TOTP_CONFIG_KEY_TOKEN_ALGO, temp_str);

                if(current_version < 5) {
                    uint32_t algo_as_uint32t = SHA1;
                    if(furi_string_cmpi_str(temp_str, TOTP_TOKEN_ALGO_SHA256_NAME) == 0) {
                        algo_as_uint32t = SHA256;
                    } else if(furi_string_cmpi_str(temp_str, TOTP_TOKEN_ALGO_SHA512_NAME) == 0) {
                        algo_as_uint32t = SHA512;
                    } else if(furi_string_cmpi_str(temp_str, TOTP_TOKEN_ALGO_STEAM_NAME) == 0) {
                        algo_as_uint32t = STEAM;
                    }

                    flipper_format_write_uint32(
                        fff_data_file, TOTP_CONFIG_KEY_TOKEN_ALGO, &algo_as_uint32t, 1);
                } else {
                    flipper_format_write_string(
                        fff_data_file, TOTP_CONFIG_KEY_TOKEN_ALGO, temp_str);
                }

                flipper_format_read_string(
                    fff_backup_data_file, TOTP_CONFIG_KEY_TOKEN_DIGITS, temp_str);
                flipper_format_write_string(fff_data_file, TOTP_CONFIG_KEY_TOKEN_DIGITS, temp_str);
            } else {
                const uint32_t default_algo = SHA1;
                flipper_format_write_uint32(
                    fff_data_file, TOTP_CONFIG_KEY_TOKEN_ALGO, &default_algo, 1);
                const uint32_t default_digits = TotpSixDigitsCount;
                flipper_format_write_uint32(
                    fff_data_file, TOTP_CONFIG_KEY_TOKEN_DIGITS, &default_digits, 1);
            }

            if(current_version > 2) {
                flipper_format_read_string(
                    fff_backup_data_file, TOTP_CONFIG_KEY_TOKEN_DURATION, temp_str);
                flipper_format_write_string(
                    fff_data_file, TOTP_CONFIG_KEY_TOKEN_DURATION, temp_str);
            } else {
                const uint32_t default_duration = TOTP_TOKEN_DURATION_DEFAULT;
                flipper_format_write_uint32(
                    fff_data_file, TOTP_CONFIG_KEY_TOKEN_DURATION, &default_duration, 1);
            }

            if(current_version > 3) {
                flipper_format_read_string(
                    fff_backup_data_file, TOTP_CONFIG_KEY_TOKEN_AUTOMATION_FEATURES, temp_str);
                flipper_format_write_string(
                    fff_data_file, TOTP_CONFIG_KEY_TOKEN_AUTOMATION_FEATURES, temp_str);
            } else {
                const uint32_t default_automation_features = TokenAutomationFeatureNone;
                flipper_format_write_uint32(
                    fff_data_file,
                    TOTP_CONFIG_KEY_TOKEN_AUTOMATION_FEATURES,
                    &default_automation_features,
                    1);
            }
        }

        Stream* stream = flipper_format_get_raw_stream(fff_data_file);
        size_t current_pos = stream_tell(stream);
        size_t total_size = stream_size(stream);
        if(current_pos < total_size) {
            stream_delete(stream, total_size - current_pos);
        }

        result = true;
    } while(false);

    furi_string_free(temp_str);
    return result;
}