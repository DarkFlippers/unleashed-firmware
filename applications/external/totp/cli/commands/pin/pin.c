#include "pin.h"

#include <stdlib.h>
#include <lib/toolbox/args.h>
#include "../../../types/token_info.h"
#include "../../../types/user_pin_codes.h"
#include "../../../services/config/config.h"
#include "../../cli_helpers.h"
#include <memset_s.h>
#include "../../../services/crypto/crypto_facade.h"
#include "../../../ui/scene_director.h"

#define TOTP_CLI_COMMAND_PIN_COMMAND_SET "set"
#define TOTP_CLI_COMMAND_PIN_COMMAND_REMOVE "remove"
#define TOTP_CLI_COMMAND_PIN_ARG_NEW_CRYPTO_KEY_SLOT_PREFIX "-c"
#define TOTP_CLI_COMMAND_PIN_ARG_NEW_CRYPTO_KEY_SLOT "slot"

void totp_cli_command_pin_docopt_commands() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_PIN "              Set\\change\\remove PIN\r\n");
}

void totp_cli_command_pin_docopt_usage() {
    TOTP_CLI_PRINTF(
        "  " TOTP_CLI_COMMAND_NAME " " TOTP_CLI_COMMAND_PIN
        " " DOCOPT_REQUIRED(TOTP_CLI_COMMAND_PIN_COMMAND_SET " | " TOTP_CLI_COMMAND_PIN_COMMAND_REMOVE) " " DOCOPT_OPTIONAL(
            DOCOPT_OPTION(
                TOTP_CLI_COMMAND_PIN_ARG_NEW_CRYPTO_KEY_SLOT_PREFIX,
                DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_PIN_ARG_NEW_CRYPTO_KEY_SLOT))) "\r\n");
}

void totp_cli_command_pin_docopt_options() {
    TOTP_CLI_PRINTF(
        "  " DOCOPT_OPTION(
            TOTP_CLI_COMMAND_PIN_ARG_NEW_CRYPTO_KEY_SLOT_PREFIX,
            DOCOPT_ARGUMENT(
                TOTP_CLI_COMMAND_PIN_ARG_NEW_CRYPTO_KEY_SLOT)) "      New crypto key slot. Must be between %d and %d\r\n",
        ACCEPTABLE_CRYPTO_KEY_SLOT_START,
        ACCEPTABLE_CRYPTO_KEY_SLOT_END);
}

static inline uint8_t totp_cli_key_to_pin_code(uint8_t key) {
    uint8_t code = 0;
    switch(key) {
    case 0x44: // left
        code = PinCodeArrowLeft;
        break;
    case 0x41: // up
        code = PinCodeArrowUp;
        break;
    case 0x43: // right
        code = PinCodeArrowRight;
        break;
    case 0x42: // down
        code = PinCodeArrowDown;
        break;
    default:
        break;
    }

    return code;
}

static bool totp_cli_read_pin(Cli* cli, uint8_t* pin, uint8_t* pin_length) {
    TOTP_CLI_PRINTF("Enter new PIN (use arrow keys on your keyboard): ");
    fflush(stdout);
    uint8_t c;
    *pin_length = 0;
    while(cli_read(cli, &c, 1) == 1) {
        if(c == CliSymbolAsciiEsc) {
            uint8_t c2;
            uint8_t c3;
            if(cli_read_timeout(cli, &c2, 1, 0) == 1 && cli_read_timeout(cli, &c3, 1, 0) == 1 &&
               c2 == 0x5b) {
                uint8_t code = totp_cli_key_to_pin_code(c3);
                if(code > 0) {
                    pin[*pin_length] = code;
                    *pin_length = *pin_length + 1;
                    putc('*', stdout);
                    fflush(stdout);
                }
            }
        } else if(c == CliSymbolAsciiETX) {
            totp_cli_delete_current_line();
            TOTP_CLI_PRINTF_INFO("Cancelled by user\r\n");
            return false;
        } else if(c == CliSymbolAsciiBackspace || c == CliSymbolAsciiDel) {
            if(*pin_length > 0) {
                *pin_length = *pin_length - 1;
                pin[*pin_length] = 0;
                totp_cli_delete_last_char();
            }
        } else if(c == CliSymbolAsciiCR) {
            cli_nl();
            break;
        }
    }

    totp_cli_delete_last_line();
    return true;
}

void totp_cli_command_pin_handle(PluginState* plugin_state, FuriString* args, Cli* cli) {
    UNUSED(plugin_state);
    FuriString* temp_str = furi_string_alloc();

    bool do_change = false;
    bool do_remove = false;
    uint8_t crypto_key_slot = plugin_state->crypto_settings.crypto_key_slot;

    bool arguments_parsed = true;
    while(args_read_string_and_trim(args, temp_str)) {
        if(furi_string_cmpi_str(temp_str, TOTP_CLI_COMMAND_PIN_COMMAND_SET) == 0) {
            do_change = true;
        } else if(furi_string_cmpi_str(temp_str, TOTP_CLI_COMMAND_PIN_COMMAND_REMOVE) == 0) {
            do_remove = true;
        } else if(
            furi_string_cmpi_str(temp_str, TOTP_CLI_COMMAND_PIN_ARG_NEW_CRYPTO_KEY_SLOT_PREFIX) ==
            0) {
            if(!args_read_uint8_and_trim(args, &crypto_key_slot) ||
               !totp_crypto_check_key_slot(crypto_key_slot)) {
                TOTP_CLI_PRINTF_ERROR("Slot \"%" PRIu8 "\" can not be used\r\n", crypto_key_slot);
                arguments_parsed = false;
                break;
            }
        } else {
            totp_cli_print_invalid_arguments();
            arguments_parsed = false;
            break;
        }
    }

    if(!(do_change || do_remove) || (do_change && do_remove)) {
        totp_cli_print_invalid_arguments();
        arguments_parsed = false;
    }

    if(arguments_parsed && totp_cli_ensure_authenticated(plugin_state, cli)) {
        TOTP_CLI_LOCK_UI(plugin_state);
        do {
            uint8_t new_pin[CRYPTO_IV_LENGTH];
            memset(&new_pin[0], 0, CRYPTO_IV_LENGTH);
            uint8_t new_pin_length = 0;
            if(do_change) {
                if(!totp_cli_read_pin(cli, &new_pin[0], &new_pin_length)) {
                    memset_s(&new_pin[0], CRYPTO_IV_LENGTH, 0, CRYPTO_IV_LENGTH);
                    break;
                }
            } else if(do_remove) {
                new_pin_length = 0;
                memset(&new_pin[0], 0, CRYPTO_IV_LENGTH);
            }

            char* backup_path = totp_config_file_backup(plugin_state);
            if(backup_path != NULL) {
                TOTP_CLI_PRINTF_WARNING("Backup conf file %s has been created\r\n", backup_path);
                TOTP_CLI_PRINTF_WARNING(
                    "Once you make sure everything is fine and works as expected, please delete this backup file\r\n");
                free(backup_path);
            } else {
                memset_s(&new_pin[0], CRYPTO_IV_LENGTH, 0, CRYPTO_IV_LENGTH);
                TOTP_CLI_PRINTF_ERROR(
                    "An error has occurred during taking backup of config file\r\n");
                break;
            }

            TOTP_CLI_PRINTF("Encrypting...\r\n");

            bool update_result = totp_config_file_update_encryption(
                plugin_state, crypto_key_slot, new_pin, new_pin_length);

            memset_s(&new_pin[0], CRYPTO_IV_LENGTH, 0, CRYPTO_IV_LENGTH);

            totp_cli_delete_last_line();

            if(update_result) {
                if(do_change) {
                    TOTP_CLI_PRINTF_SUCCESS("PIN has been successfully changed\r\n");
                } else if(do_remove) {
                    TOTP_CLI_PRINTF_SUCCESS("PIN has been successfully removed\r\n");
                }
            } else {
                totp_cli_print_error_updating_config_file();
            }

        } while(false);

        TOTP_CLI_UNLOCK_UI(plugin_state);
    }

    furi_string_free(temp_str);
}