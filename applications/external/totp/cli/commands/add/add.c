#include "add.h"
#include <stdlib.h>
#include <lib/toolbox/args.h>
#include "../../../lib/list/list.h"
#include "../../../types/token_info.h"
#include "../../../services/config/config.h"
#include "../../../services/convert/convert.h"
#include "../../cli_helpers.h"
#include "../../../ui/scene_director.h"
#include "../../common_command_arguments.h"

void totp_cli_command_add_docopt_commands() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_ADD ", " TOTP_CLI_COMMAND_ADD_ALT
                    ", " TOTP_CLI_COMMAND_ADD_ALT2 "     Add new token\r\n");
}

void totp_cli_command_add_docopt_usage() {
    TOTP_CLI_PRINTF(
        "  " TOTP_CLI_COMMAND_NAME
        " " DOCOPT_REQUIRED(TOTP_CLI_COMMAND_ADD " | " TOTP_CLI_COMMAND_ADD_ALT " | " TOTP_CLI_COMMAND_ADD_ALT2) " " DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_NAME) " " DOCOPT_OPTIONAL(DOCOPT_OPTION(TOTP_CLI_COMMAND_ARG_ALGO_PREFIX, DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_ALGO))) " " DOCOPT_OPTIONAL(DOCOPT_OPTION(TOTP_CLI_COMMAND_ARG_DIGITS_PREFIX, DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_DIGITS))) " " DOCOPT_OPTIONAL(
            DOCOPT_OPTION(
                TOTP_CLI_COMMAND_ARG_DURATION_PREFIX,
                DOCOPT_ARGUMENT(
                    TOTP_CLI_COMMAND_ARG_DURATION))) " " DOCOPT_OPTIONAL(DOCOPT_SWITCH(TOTP_CLI_COMMAND_ARG_UNSECURE_PREFIX)) " " DOCOPT_MULTIPLE(DOCOPT_OPTIONAL(DOCOPT_OPTION(TOTP_CLI_COMMAND_ARG_AUTOMATION_FEATURE_PREFIX, DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_AUTOMATION_FEATURE)))) "\r\n");
}

void totp_cli_command_add_docopt_arguments() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_ARG_NAME "          Token name\r\n");
}

void totp_cli_command_add_docopt_options() {
    TOTP_CLI_PRINTF("  " DOCOPT_OPTION(
        TOTP_CLI_COMMAND_ARG_ALGO_PREFIX,
        DOCOPT_ARGUMENT(
            TOTP_CLI_COMMAND_ARG_ALGO)) "      Token hashing algorithm. Must be one of: " TOTP_TOKEN_ALGO_SHA1_NAME
                                        ", " TOTP_TOKEN_ALGO_SHA256_NAME
                                        ", " TOTP_TOKEN_ALGO_SHA512_NAME
                                        " " DOCOPT_DEFAULT(TOTP_TOKEN_ALGO_SHA1_NAME) "\r\n");
    TOTP_CLI_PRINTF("  " DOCOPT_OPTION(
        TOTP_CLI_COMMAND_ARG_DIGITS_PREFIX,
        DOCOPT_ARGUMENT(
            TOTP_CLI_COMMAND_ARG_DIGITS)) "    Number of digits to generate, one of: 6, 8 " DOCOPT_DEFAULT("6") "\r\n");
    TOTP_CLI_PRINTF("  " DOCOPT_OPTION(
        TOTP_CLI_COMMAND_ARG_DURATION_PREFIX,
        DOCOPT_ARGUMENT(
            TOTP_CLI_COMMAND_ARG_DURATION)) "  Token lifetime duration in seconds, between: 15 and 255 " DOCOPT_DEFAULT("30") "\r\n");
    TOTP_CLI_PRINTF("  " DOCOPT_SWITCH(
        TOTP_CLI_COMMAND_ARG_UNSECURE_PREFIX) "             Show console user input as-is without masking\r\n");
    TOTP_CLI_PRINTF("  " DOCOPT_OPTION(
        TOTP_CLI_COMMAND_ARG_AUTOMATION_FEATURE_PREFIX,
        DOCOPT_ARGUMENT(
            TOTP_CLI_COMMAND_ARG_AUTOMATION_FEATURE)) "   Token automation features to be enabled. Must be one of: " TOTP_TOKEN_AUTOMATION_FEATURE_NONE_NAME
                                                      ", " TOTP_TOKEN_AUTOMATION_FEATURE_ENTER_AT_THE_END_NAME
                                                      ", " TOTP_TOKEN_AUTOMATION_FEATURE_TAB_AT_THE_END_NAME
                                                      " " DOCOPT_DEFAULT(
                                                          TOTP_TOKEN_AUTOMATION_FEATURE_NONE_NAME) "\r\n");
    TOTP_CLI_PRINTF("                 # " TOTP_TOKEN_AUTOMATION_FEATURE_NONE_NAME
                    " - No features\r\n");
    TOTP_CLI_PRINTF("                 # " TOTP_TOKEN_AUTOMATION_FEATURE_ENTER_AT_THE_END_NAME
                    " - Type <Enter> key at the end of token input automation\r\n");
    TOTP_CLI_PRINTF("                 # " TOTP_TOKEN_AUTOMATION_FEATURE_TAB_AT_THE_END_NAME
                    " - Type <Tab> key at the end of token input automation\r\n");
    TOTP_CLI_PRINTF("                 # " TOTP_TOKEN_AUTOMATION_FEATURE_TYPE_SLOWER_NAME
                    " - Type slower\r\n");
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
        if(!totp_cli_try_read_algo(token_info, temp_str, args, &parsed) &&
           !totp_cli_try_read_digits(token_info, temp_str, args, &parsed) &&
           !totp_cli_try_read_duration(token_info, temp_str, args, &parsed) &&
           !totp_cli_try_read_unsecure_flag(temp_str, &parsed, &mask_user_input) &&
           !totp_cli_try_read_automation_features(token_info, temp_str, args, &parsed)) {
            totp_cli_printf_unknown_argument(temp_str);
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
    if(!totp_cli_read_line(cli, temp_str, mask_user_input) ||
       !totp_cli_ensure_authenticated(plugin_state, cli)) {
        TOTP_CLI_DELETE_LAST_LINE();
        TOTP_CLI_PRINTF_INFO("Cancelled by user\r\n");
        furi_string_secure_free(temp_str);
        token_info_free(token_info);
        return;
    }

    TOTP_CLI_DELETE_LAST_LINE();

    if(!token_info_set_secret(
           token_info,
           furi_string_get_cstr(temp_str),
           furi_string_size(temp_str),
           plugin_state->iv)) {
        TOTP_CLI_PRINTF_ERROR("Token secret seems to be invalid and can not be parsed\r\n");
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
        TOTP_CLI_PRINTF_SUCCESS("Token \"%s\" has been successfully added\r\n", token_info->name);
    } else {
        TOTP_CLI_PRINT_ERROR_UPDATING_CONFIG_FILE();
    }

    if(load_generate_token_scene) {
        totp_scene_director_activate_scene(plugin_state, TotpSceneGenerateToken, NULL);
    }
}