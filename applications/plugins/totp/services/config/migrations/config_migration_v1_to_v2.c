#include "config_migration_v1_to_v2.h"
#include <flipper_format/flipper_format.h>
#include "../constants.h"
#include "../../../types/token_info.h"

#define NEW_VERSION 2

bool totp_config_migrate_v1_to_v2(
    FlipperFormat* fff_data_file,
    FlipperFormat* fff_backup_data_file) {
    flipper_format_write_header_cstr(fff_data_file, CONFIG_FILE_HEADER, NEW_VERSION);

    FuriString* temp_str = furi_string_alloc();

    if(flipper_format_read_string(fff_backup_data_file, TOTP_CONFIG_KEY_BASE_IV, temp_str)) {
        flipper_format_write_string(fff_data_file, TOTP_CONFIG_KEY_BASE_IV, temp_str);
    }

    if(flipper_format_read_string(fff_backup_data_file, TOTP_CONFIG_KEY_CRYPTO_VERIFY, temp_str)) {
        flipper_format_write_string(fff_data_file, TOTP_CONFIG_KEY_CRYPTO_VERIFY, temp_str);
    }

    if(flipper_format_read_string(fff_backup_data_file, TOTP_CONFIG_KEY_TIMEZONE, temp_str)) {
        flipper_format_write_string(fff_data_file, TOTP_CONFIG_KEY_TIMEZONE, temp_str);
    }

    while(true) {
        if(!flipper_format_read_string(
               fff_backup_data_file, TOTP_CONFIG_KEY_TOKEN_NAME, temp_str)) {
            break;
        }

        flipper_format_write_string(fff_data_file, TOTP_CONFIG_KEY_TOKEN_NAME, temp_str);

        flipper_format_read_string(fff_backup_data_file, TOTP_CONFIG_KEY_TOKEN_SECRET, temp_str);
        flipper_format_write_string(fff_data_file, TOTP_CONFIG_KEY_TOKEN_SECRET, temp_str);

        flipper_format_write_string_cstr(
            fff_data_file, TOTP_CONFIG_KEY_TOKEN_ALGO, TOTP_CONFIG_TOKEN_ALGO_SHA1_NAME);
        const uint32_t default_digits = TOTP_6_DIGITS;
        flipper_format_write_uint32(
            fff_data_file, TOTP_CONFIG_KEY_TOKEN_DIGITS, &default_digits, 1);
    }

    furi_string_free(temp_str);
    return true;
}
