#include "add.h"
#include <stdlib.h>
#include <lib/toolbox/args.h>
#include "../../../types/token_info.h"
#include "../../../services/config/config.h"
#include "../../../services/convert/convert.h"
#include "../../cli_helpers.h"
#include "../../../ui/scene_director.h"
#include "../../common_command_arguments.h"

struct TotpAddContext {
    FuriString* args;
    Cli* cli;
    const CryptoSettings* crypto_settings;
};

enum TotpIteratorUpdateTokenResultsEx {
    TotpIteratorUpdateTokenResultInvalidSecret = 1,
    TotpIteratorUpdateTokenResultCancelled = 2,
    TotpIteratorUpdateTokenResultInvalidArguments = 3
};

static TotpIteratorUpdateTokenResult
    add_token_handler(TokenInfo* token_info, const void* context) {
    const struct TotpAddContext* context_t = context;

    // Reading token name
    if(!args_read_probably_quoted_string_and_trim(context_t->args, token_info->name)) {
        return TotpIteratorUpdateTokenResultInvalidArguments;
    }

    FuriString* temp_str = furi_string_alloc();

    // Read optional arguments
    bool mask_user_input = true;
    PlainTokenSecretEncoding token_secret_encoding = PlainTokenSecretEncodingBase32;
    while(args_read_string_and_trim(context_t->args, temp_str)) {
        bool parsed = false;
        if(!totp_cli_try_read_algo(token_info, temp_str, context_t->args, &parsed) &&
           !totp_cli_try_read_digits(token_info, temp_str, context_t->args, &parsed) &&
           !totp_cli_try_read_duration(token_info, temp_str, context_t->args, &parsed) &&
           !totp_cli_try_read_unsecure_flag(temp_str, &parsed, &mask_user_input) &&
           !totp_cli_try_read_automation_features(token_info, temp_str, context_t->args, &parsed) &&
           !totp_cli_try_read_plain_token_secret_encoding(
               temp_str, context_t->args, &parsed, &token_secret_encoding)) {
            totp_cli_printf_unknown_argument(temp_str);
        }

        if(!parsed) {
            furi_string_free(temp_str);
            return TotpIteratorUpdateTokenResultInvalidArguments;
        }
    }

    // Reading token secret
    furi_string_reset(temp_str);
    TOTP_CLI_PRINTF("Enter token secret and confirm with [ENTER]:\r\n");
    if(!totp_cli_read_line(context_t->cli, temp_str, mask_user_input)) {
        totp_cli_delete_last_line();
        furi_string_secure_free(temp_str);
        return TotpIteratorUpdateTokenResultCancelled;
    }

    totp_cli_delete_last_line();

    bool secret_set = token_info_set_secret(
        token_info,
        furi_string_get_cstr(temp_str),
        furi_string_size(temp_str),
        token_secret_encoding,
        context_t->crypto_settings);

    furi_string_secure_free(temp_str);

    if(!secret_set) {
        return TotpIteratorUpdateTokenResultInvalidSecret;
    }

    return TotpIteratorUpdateTokenResultSuccess;
}

#ifdef TOTP_CLI_RICH_HELP_ENABLED
void totp_cli_command_add_docopt_commands() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_ADD ", " TOTP_CLI_COMMAND_ADD_ALT
                    ", " TOTP_CLI_COMMAND_ADD_ALT2 "     Add new token\r\n");
}

void totp_cli_command_add_docopt_usage() {
    TOTP_CLI_PRINTF(
        "  " TOTP_CLI_COMMAND_NAME
        " " DOCOPT_REQUIRED(TOTP_CLI_COMMAND_ADD " | " TOTP_CLI_COMMAND_ADD_ALT " | " TOTP_CLI_COMMAND_ADD_ALT2) " " DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_NAME) " " DOCOPT_OPTIONAL(DOCOPT_OPTION(TOTP_CLI_COMMAND_ARG_ALGO_PREFIX, DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_ALGO))) " " DOCOPT_OPTIONAL(DOCOPT_OPTION(TOTP_CLI_COMMAND_ARG_SECRET_ENCODING_PREFIX, DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_SECRET_ENCODING))) " " DOCOPT_OPTIONAL(
            DOCOPT_OPTION(
                TOTP_CLI_COMMAND_ARG_DIGITS_PREFIX,
                DOCOPT_ARGUMENT(
                    TOTP_CLI_COMMAND_ARG_DIGITS))) " " DOCOPT_OPTIONAL(DOCOPT_OPTION(TOTP_CLI_COMMAND_ARG_DURATION_PREFIX, DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_DURATION))) " " DOCOPT_OPTIONAL(DOCOPT_SWITCH(TOTP_CLI_COMMAND_ARG_UNSECURE_PREFIX)) " " DOCOPT_MULTIPLE(DOCOPT_OPTIONAL(DOCOPT_OPTION(TOTP_CLI_COMMAND_ARG_AUTOMATION_FEATURE_PREFIX, DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_AUTOMATION_FEATURE)))) "\r\n");
}

void totp_cli_command_add_docopt_arguments() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_ARG_NAME "          Token name\r\n");
}

void totp_cli_command_add_docopt_options() {
    TOTP_CLI_PRINTF("  " DOCOPT_OPTION(
        TOTP_CLI_COMMAND_ARG_ALGO_PREFIX,
        DOCOPT_ARGUMENT(
            TOTP_CLI_COMMAND_ARG_ALGO)) "      Token hashing algorithm. Must be one of: " TOKEN_HASH_ALGO_SHA1_NAME
                                        ", " TOKEN_HASH_ALGO_SHA256_NAME
                                        ", " TOKEN_HASH_ALGO_SHA512_NAME
                                        ", " TOKEN_HASH_ALGO_STEAM_NAME
                                        " " DOCOPT_DEFAULT(TOKEN_HASH_ALGO_SHA1_NAME) "\r\n");
    TOTP_CLI_PRINTF(
        "  " DOCOPT_OPTION(
            TOTP_CLI_COMMAND_ARG_DIGITS_PREFIX,
            DOCOPT_ARGUMENT(
                TOTP_CLI_COMMAND_ARG_DIGITS)) "    Number of digits to generate, one of: %" PRIu8
                                              ", %" PRIu8 ", %" PRIu8
                                              " " DOCOPT_DEFAULT("%" PRIu8) "\r\n",
        TokenDigitsCountFive,
        TokenDigitsCountSix,
        TokenDigitsCountEight,
        TokenDigitsCountSix);

    TOTP_CLI_PRINTF("  " DOCOPT_OPTION(
        TOTP_CLI_COMMAND_ARG_SECRET_ENCODING_PREFIX,
        DOCOPT_ARGUMENT(
            TOTP_CLI_COMMAND_ARG_SECRET_ENCODING)) "  Token secret encoding, one of " PLAIN_TOKEN_ENCODING_BASE32_NAME
                                                   ", " PLAIN_TOKEN_ENCODING_BASE64_NAME
                                                   " " DOCOPT_DEFAULT(
                                                       PLAIN_TOKEN_ENCODING_BASE32_NAME) "\r\n");

    TOTP_CLI_PRINTF(
        "  " DOCOPT_OPTION(
            TOTP_CLI_COMMAND_ARG_DURATION_PREFIX,
            DOCOPT_ARGUMENT(
                TOTP_CLI_COMMAND_ARG_DURATION)) "  Token lifetime duration in seconds, between: %" PRIu8
                                                " and %" PRIu8
                                                " " DOCOPT_DEFAULT("%" PRIu8) "\r\n",
        TokenDurationMin,
        TokenDurationMax,
        TokenDurationDefault);
    TOTP_CLI_PRINTF("  " DOCOPT_SWITCH(
        TOTP_CLI_COMMAND_ARG_UNSECURE_PREFIX) "             Show console user input as-is without masking\r\n");
    TOTP_CLI_PRINTF("  " DOCOPT_OPTION(
        TOTP_CLI_COMMAND_ARG_AUTOMATION_FEATURE_PREFIX,
        DOCOPT_ARGUMENT(
            TOTP_CLI_COMMAND_ARG_AUTOMATION_FEATURE)) "   Token automation features to be enabled. Must be one of: " TOKEN_AUTOMATION_FEATURE_NONE_NAME
                                                      ", " TOKEN_AUTOMATION_FEATURE_ENTER_AT_THE_END_NAME
                                                      ", " TOKEN_AUTOMATION_FEATURE_TAB_AT_THE_END_NAME
                                                      " " DOCOPT_DEFAULT(
                                                          TOKEN_AUTOMATION_FEATURE_NONE_NAME) "\r\n");
    TOTP_CLI_PRINTF("                 # " TOKEN_AUTOMATION_FEATURE_NONE_NAME " - No features\r\n");
    TOTP_CLI_PRINTF("                 # " TOKEN_AUTOMATION_FEATURE_ENTER_AT_THE_END_NAME
                    " - Type <Enter> key at the end of token input automation\r\n");
    TOTP_CLI_PRINTF("                 # " TOKEN_AUTOMATION_FEATURE_TAB_AT_THE_END_NAME
                    " - Type <Tab> key at the end of token input automation\r\n");
    TOTP_CLI_PRINTF("                 # " TOKEN_AUTOMATION_FEATURE_TYPE_SLOWER_NAME
                    " - Type slower\r\n");
}
#endif

void totp_cli_command_add_handle(PluginState* plugin_state, FuriString* args, Cli* cli) {
    if(!totp_cli_ensure_authenticated(plugin_state, cli)) {
        return;
    }

    TokenInfoIteratorContext* iterator_context =
        totp_config_get_token_iterator_context(plugin_state);

    TOTP_CLI_LOCK_UI(plugin_state);

    struct TotpAddContext add_context = {
        .args = args, .cli = cli, .crypto_settings = &plugin_state->crypto_settings};
    TotpIteratorUpdateTokenResult add_result =
        totp_token_info_iterator_add_new_token(iterator_context, &add_token_handler, &add_context);

    if(add_result == TotpIteratorUpdateTokenResultSuccess) {
        TOTP_CLI_PRINTF_SUCCESS(
            "Token \"%s\" has been successfully added\r\n",
            furi_string_get_cstr(
                totp_token_info_iterator_get_current_token(iterator_context)->name));
    } else if(add_result == TotpIteratorUpdateTokenResultCancelled) {
        TOTP_CLI_PRINTF_INFO("Cancelled by user\r\n");
    } else if(add_result == TotpIteratorUpdateTokenResultInvalidArguments) {
        totp_cli_print_invalid_arguments();
    } else if(add_result == TotpIteratorUpdateTokenResultInvalidSecret) {
        TOTP_CLI_PRINTF_ERROR("Token secret seems to be invalid and can not be parsed\r\n");
    } else if(add_result == TotpIteratorUpdateTokenResultFileUpdateFailed) {
        totp_cli_print_error_updating_config_file();
    }

    TOTP_CLI_UNLOCK_UI(plugin_state);
}