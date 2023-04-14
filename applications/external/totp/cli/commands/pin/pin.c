#include "pin.h"

#include <stdlib.h>
#include <lib/toolbox/args.h>
#include <linked_list.h>
#include "../../../types/token_info.h"
#include "../../../types/user_pin_codes.h"
#include "../../../services/config/config.h"
#include "../../cli_helpers.h"
#include <memset_s.h>
#include "../../../services/crypto/crypto.h"
#include "../../../ui/scene_director.h"

#define TOTP_CLI_COMMAND_PIN_COMMAND_SET "set"
#define TOTP_CLI_COMMAND_PIN_COMMAND_REMOVE "remove"

void totp_cli_command_pin_docopt_commands() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_PIN "              Set\\change\\remove PIN\r\n");
}

void totp_cli_command_pin_docopt_usage() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_NAME " " TOTP_CLI_COMMAND_PIN " " DOCOPT_REQUIRED(
        TOTP_CLI_COMMAND_PIN_COMMAND_SET " | " TOTP_CLI_COMMAND_PIN_COMMAND_REMOVE) "\r\n");
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
            TOTP_CLI_DELETE_CURRENT_LINE();
            TOTP_CLI_PRINTF_INFO("Cancelled by user\r\n");
            return false;
        } else if(c == CliSymbolAsciiBackspace || c == CliSymbolAsciiDel) {
            if(*pin_length > 0) {
                *pin_length = *pin_length - 1;
                pin[*pin_length] = 0;
                TOTP_CLI_DELETE_LAST_CHAR();
            }
        } else if(c == CliSymbolAsciiCR) {
            cli_nl();
            break;
        }
    }

    TOTP_CLI_DELETE_LAST_LINE();
    return true;
}

void totp_cli_command_pin_handle(PluginState* plugin_state, FuriString* args, Cli* cli) {
    UNUSED(plugin_state);
    FuriString* temp_str = furi_string_alloc();

    bool do_change = false;
    bool do_remove = false;
    UNUSED(do_remove);
    if(args_read_string_and_trim(args, temp_str)) {
        if(furi_string_cmpi_str(temp_str, TOTP_CLI_COMMAND_PIN_COMMAND_SET) == 0) {
            do_change = true;
        } else if(furi_string_cmpi_str(temp_str, TOTP_CLI_COMMAND_PIN_COMMAND_REMOVE) == 0) {
            do_remove = true;
        } else {
            TOTP_CLI_PRINT_INVALID_ARGUMENTS();
        }
    } else {
        TOTP_CLI_PRINT_INVALID_ARGUMENTS();
    }

    if((do_change || do_remove) && totp_cli_ensure_authenticated(plugin_state, cli)) {
        bool load_generate_token_scene = false;
        do {
            uint8_t old_iv[TOTP_IV_SIZE];
            memcpy(&old_iv[0], &plugin_state->iv[0], TOTP_IV_SIZE);
            uint8_t new_pin[TOTP_IV_SIZE];
            uint8_t new_pin_length = 0;
            if(do_change) {
                if(!totp_cli_read_pin(cli, &new_pin[0], &new_pin_length) ||
                   !totp_cli_ensure_authenticated(plugin_state, cli)) {
                    memset_s(&new_pin[0], TOTP_IV_SIZE, 0, TOTP_IV_SIZE);
                    break;
                }
            } else if(do_remove) {
                new_pin_length = 0;
                memset(&new_pin[0], 0, TOTP_IV_SIZE);
            }

            char* backup_path = totp_config_file_backup();
            if(backup_path != NULL) {
                TOTP_CLI_PRINTF_WARNING("Backup conf file %s has been created\r\n", backup_path);
                TOTP_CLI_PRINTF_WARNING(
                    "Once you make sure everything is fine and works as expected, please delete this backup file\r\n");
                free(backup_path);
            } else {
                memset_s(&new_pin[0], TOTP_IV_SIZE, 0, TOTP_IV_SIZE);
                TOTP_CLI_PRINTF_ERROR(
                    "An error has occurred during taking backup of config file\r\n");
                break;
            }

            if(plugin_state->current_scene == TotpSceneGenerateToken) {
                totp_scene_director_activate_scene(plugin_state, TotpSceneNone, NULL);
                load_generate_token_scene = true;
            }

            TOTP_CLI_PRINTF("Encrypting, please wait...\r\n");

            memset(&plugin_state->iv[0], 0, TOTP_IV_SIZE);
            memset(&plugin_state->base_iv[0], 0, TOTP_IV_SIZE);
            if(plugin_state->crypto_verify_data != NULL) {
                free(plugin_state->crypto_verify_data);
                plugin_state->crypto_verify_data = NULL;
            }

            if(!totp_crypto_seed_iv(
                   plugin_state, new_pin_length > 0 ? &new_pin[0] : NULL, new_pin_length)) {
                memset_s(&new_pin[0], TOTP_IV_SIZE, 0, TOTP_IV_SIZE);
                TOTP_CLI_PRINT_ERROR_UPDATING_CONFIG_FILE();
                break;
            }

            memset_s(&new_pin[0], TOTP_IV_SIZE, 0, TOTP_IV_SIZE);

            TOTP_LIST_FOREACH(plugin_state->tokens_list, node, {
                TokenInfo* token_info = node->data;
                size_t plain_token_length;
                uint8_t* plain_token = totp_crypto_decrypt(
                    token_info->token, token_info->token_length, &old_iv[0], &plain_token_length);
                free(token_info->token);
                token_info->token = totp_crypto_encrypt(
                    plain_token,
                    plain_token_length,
                    &plugin_state->iv[0],
                    &token_info->token_length);
                memset_s(plain_token, plain_token_length, 0, plain_token_length);
                free(plain_token);
            });

            TOTP_CLI_DELETE_LAST_LINE();

            if(totp_full_save_config_file(plugin_state) == TotpConfigFileUpdateSuccess) {
                if(do_change) {
                    TOTP_CLI_PRINTF_SUCCESS("PIN has been successfully changed\r\n");
                } else if(do_remove) {
                    TOTP_CLI_PRINTF_SUCCESS("PIN has been successfully removed\r\n");
                }
            } else {
                TOTP_CLI_PRINT_ERROR_UPDATING_CONFIG_FILE();
            }

        } while(false);

        if(load_generate_token_scene) {
            totp_scene_director_activate_scene(plugin_state, TotpSceneGenerateToken, NULL);
        }
    }

    furi_string_free(temp_str);
}