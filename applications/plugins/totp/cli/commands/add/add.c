#include "add.h"
#include <stdlib.h>
#include <lib/toolbox/args.h>
#include "../../../lib/list/list.h"
#include "../../../types/token_info.h"
#include "../../../services/config/config.h"
#include "../../../services/convert/convert.h"
#include "../../cli_helpers.h"
#include "../../../ui/scene_director.h"

#define TOTP_CLI_COMMAND_ADD_ARG_NAME "name"
#define TOTP_CLI_COMMAND_ADD_ARG_ALGO "algo"
#define TOTP_CLI_COMMAND_ADD_ARG_ALGO_PREFIX "-a"
#define TOTP_CLI_COMMAND_ADD_ARG_DIGITS "digits"
#define TOTP_CLI_COMMAND_ADD_ARG_DIGITS_PREFIX "-d"
#define TOTP_CLI_COMMAND_ADD_ARG_UNSECURE_PREFIX "-u"

static bool token_info_set_algo_from_str(TokenInfo* token_info, const FuriString* str) {
    if(furi_string_cmpi_str(str, TOTP_CONFIG_TOKEN_ALGO_SHA1_NAME) == 0) {
        token_info->algo = SHA1;
        return true;
    }

    if(furi_string_cmpi_str(str, TOTP_CONFIG_TOKEN_ALGO_SHA256_NAME) == 0) {
        token_info->algo = SHA256;
        return true;
    }

    if(furi_string_cmpi_str(str, TOTP_CONFIG_TOKEN_ALGO_SHA512_NAME) == 0) {
        token_info->algo = SHA512;
        return true;
    }

    return false;
}

void totp_cli_command_add_docopt_commands() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_ADD ", " TOTP_CLI_COMMAND_ADD_ALT
                    ", " TOTP_CLI_COMMAND_ADD_ALT2 "     Add new token\r\n");
}

void totp_cli_command_add_docopt_usage() {
    TOTP_CLI_PRINTF(
        "  " TOTP_CLI_COMMAND_NAME
        " " DOCOPT_REQUIRED(TOTP_CLI_COMMAND_ADD " | " TOTP_CLI_COMMAND_ADD_ALT " | " TOTP_CLI_COMMAND_ADD_ALT2) " " DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ADD_ARG_NAME) " " DOCOPT_OPTIONAL(
            DOCOPT_OPTION(
                TOTP_CLI_COMMAND_ADD_ARG_ALGO_PREFIX,
                DOCOPT_ARGUMENT(
                    TOTP_CLI_COMMAND_ADD_ARG_ALGO))) " " DOCOPT_OPTIONAL(DOCOPT_OPTION(TOTP_CLI_COMMAND_ADD_ARG_DIGITS_PREFIX, DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ADD_ARG_DIGITS))) " " DOCOPT_OPTIONAL(DOCOPT_SWITCH(TOTP_CLI_COMMAND_ADD_ARG_UNSECURE_PREFIX)) "\r\n");
}

void totp_cli_command_add_docopt_arguments() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_ADD_ARG_NAME "        Token name\r\n");
}

void totp_cli_command_add_docopt_options() {
    TOTP_CLI_PRINTF("  " DOCOPT_OPTION(
        TOTP_CLI_COMMAND_ADD_ARG_ALGO_PREFIX,
        DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ADD_ARG_ALGO)) "      Token hashing algorithm.\r\n");
    TOTP_CLI_PRINTF(
        "                 Could be one of: sha1, sha256, sha512 " DOCOPT_DEFAULT("sha1") "\r\n");
    cli_nl();
    TOTP_CLI_PRINTF("  " DOCOPT_OPTION(
        TOTP_CLI_COMMAND_ADD_ARG_DIGITS_PREFIX,
        DOCOPT_ARGUMENT(
            TOTP_CLI_COMMAND_ADD_ARG_DIGITS)) "    Number of digits to generate, one of: 6, 8 " DOCOPT_DEFAULT("6") "\r\n");
    TOTP_CLI_PRINTF("  " DOCOPT_SWITCH(
        TOTP_CLI_COMMAND_ADD_ARG_UNSECURE_PREFIX) "             Show console user input as-is without masking\r\n");
}

static void furi_string_secure_free(FuriString* str) {
    for(long i = furi_string_size(str) - 1; i >= 0; i--) {
        furi_string_set_char(str, i, '\0');
    }

    furi_string_free(str);
}

static bool totp_cli_read_secret(Cli* cli, FuriString* out_str, bool mask_user_input) {
    uint8_t c;
    while(cli_read(cli, &c, 1) == 1) {
        if(c == CliSymbolAsciiEsc) {
            // Some keys generating escape-sequences
            // We need to ignore them as we care about alpha-numerics only
            uint8_t c2;
            cli_read_timeout(cli, &c2, 1, 0);
            cli_read_timeout(cli, &c2, 1, 0);
        } else if(c == CliSymbolAsciiETX) {
            TOTP_CLI_DELETE_CURRENT_LINE();
            TOTP_CLI_PRINTF("Cancelled by user\r\n");
            return false;
        } else if((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
            if(mask_user_input) {
                putc('*', stdout);
            } else {
                putc(c, stdout);
            }
            fflush(stdout);
            furi_string_push_back(out_str, c);
        } else if(c == CliSymbolAsciiBackspace || c == CliSymbolAsciiDel) {
            size_t out_str_size = furi_string_size(out_str);
            if(out_str_size > 0) {
                TOTP_CLI_DELETE_LAST_CHAR();
                furi_string_left(out_str, out_str_size - 1);
            }
        } else if(c == CliSymbolAsciiCR) {
            cli_nl();
            break;
        }
    }

    TOTP_CLI_DELETE_LAST_LINE();
    return true;
}

void totp_cli_command_add_handle(PluginState* plugin_state, FuriString* args, Cli* cli) {
    FuriString* temp_str = furi_string_alloc();
    TokenInfo* token_info = token_info_alloc();

    // Reading token name
    if(!args_read_probably_quoted_string_and_trim(args, temp_str)) {
        TOTP_CLI_PRINT_INVALID_ARGUMENTS();
        furi_string_free(temp_str);
        token_info_free(token_info);
        return;
    }

    size_t temp_cstr_len = furi_string_size(temp_str);
    token_info->name = malloc(temp_cstr_len + 1);
    furi_check(token_info->name != NULL);
    strlcpy(token_info->name, furi_string_get_cstr(temp_str), temp_cstr_len + 1);

    // Read optional arguments
    bool mask_user_input = true;
    while(args_read_string_and_trim(args, temp_str)) {
        bool parsed = false;
        if(furi_string_cmpi_str(temp_str, TOTP_CLI_COMMAND_ADD_ARG_ALGO_PREFIX) == 0) {
            if(!args_read_string_and_trim(args, temp_str)) {
                TOTP_CLI_PRINTF("Missed value for argument \"" TOTP_CLI_COMMAND_ADD_ARG_ALGO_PREFIX
                                "\"\r\n");
            } else if(!token_info_set_algo_from_str(token_info, temp_str)) {
                TOTP_CLI_PRINTF(
                    "\"%s\" is incorrect value for argument \"" TOTP_CLI_COMMAND_ADD_ARG_ALGO_PREFIX
                    "\"\r\n",
                    furi_string_get_cstr(temp_str));
            } else {
                parsed = true;
            }
        } else if(furi_string_cmpi_str(temp_str, TOTP_CLI_COMMAND_ADD_ARG_DIGITS_PREFIX) == 0) {
            if(!args_read_string_and_trim(args, temp_str)) {
                TOTP_CLI_PRINTF(
                    "Missed value for argument \"" TOTP_CLI_COMMAND_ADD_ARG_DIGITS_PREFIX
                    "\"\r\n");
            } else if(!token_info_set_digits_from_int(
                          token_info, CONVERT_CHAR_TO_DIGIT(furi_string_get_char(temp_str, 0)))) {
                TOTP_CLI_PRINTF(
                    "\"%s\" is incorrect value for argument \"" TOTP_CLI_COMMAND_ADD_ARG_DIGITS_PREFIX
                    "\"\r\n",
                    furi_string_get_cstr(temp_str));
            } else {
                parsed = true;
            }
        } else if(furi_string_cmpi_str(temp_str, TOTP_CLI_COMMAND_ADD_ARG_UNSECURE_PREFIX) == 0) {
            mask_user_input = false;
            parsed = true;
        } else {
            TOTP_CLI_PRINTF("Unknown argument \"%s\"\r\n", furi_string_get_cstr(temp_str));
        }

        if(!parsed) {
            TOTP_CLI_PRINT_INVALID_ARGUMENTS();
            furi_string_free(temp_str);
            token_info_free(token_info);
            return;
        }
    }

    // Reading token secret
    furi_string_reset(temp_str);
    TOTP_CLI_PRINTF("Enter token secret and confirm with [ENTER]\r\n");
    if(!totp_cli_read_secret(cli, temp_str, mask_user_input) ||
       !totp_cli_ensure_authenticated(plugin_state, cli)) {
        furi_string_secure_free(temp_str);
        token_info_free(token_info);
        return;
    }

    if(!token_info_set_secret(
           token_info,
           furi_string_get_cstr(temp_str),
           furi_string_size(temp_str),
           plugin_state->iv)) {
        TOTP_CLI_PRINTF("Token secret seems to be invalid and can not be parsed\r\n");
        furi_string_secure_free(temp_str);
        token_info_free(token_info);
        return;
    }

    furi_string_secure_free(temp_str);

    bool load_generate_token_scene = false;
    if(plugin_state->current_scene == TotpSceneGenerateToken) {
        totp_scene_director_activate_scene(plugin_state, TotpSceneNone, NULL);
        load_generate_token_scene = true;
    }

    TOTP_LIST_INIT_OR_ADD(plugin_state->tokens_list, token_info, furi_check);
    plugin_state->tokens_count++;
    if(totp_config_file_save_new_token(token_info) == TotpConfigFileUpdateSuccess) {
        TOTP_CLI_PRINTF("Token \"%s\" has been successfully added\r\n", token_info->name);
    } else {
        TOTP_CLI_PRINT_ERROR_UPDATING_CONFIG_FILE();
    }

    if(load_generate_token_scene) {
        totp_scene_director_activate_scene(plugin_state, TotpSceneGenerateToken, NULL);
    }
}