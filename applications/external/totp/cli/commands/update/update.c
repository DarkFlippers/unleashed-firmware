#include "update.h"
#include <stdlib.h>
#include <lib/toolbox/args.h>
#include "../../../types/token_info.h"
#include "../../../services/config/config.h"
#include "../../../services/convert/convert.h"
#include "../../cli_helpers.h"
#include "../../../ui/scene_director.h"
#include "../../common_command_arguments.h"

#define TOTP_CLI_COMMAND_UPDATE_ARG_SECRET_PREFIX "-s"

struct TotpUpdateContext {
    FuriString* args;
    Cli* cli;
    const CryptoSettings* crypto_settings;
};

enum TotpIteratorUpdateTokenResultsEx {
    TotpIteratorUpdateTokenResultInvalidSecret = 1,
    TotpIteratorUpdateTokenResultCancelled = 2,
    TotpIteratorUpdateTokenResultInvalidArguments = 3
};

static bool totp_cli_try_read_name(
    TokenInfo* token_info,
    const FuriString* arg,
    FuriString* args,
    bool* parsed) {
    if(furi_string_cmpi_str(arg, TOTP_CLI_COMMAND_ARG_NAME_PREFIX) == 0) {
        if(!args_read_probably_quoted_string_and_trim(args, token_info->name) ||
           furi_string_empty(token_info->name)) {
            totp_cli_printf_missed_argument_value(TOTP_CLI_COMMAND_ARG_NAME_PREFIX);
        } else {
            *parsed = true;
        }

        return true;
    }

    return false;
}

static bool totp_cli_try_read_change_secret_flag(const FuriString* arg, bool* parsed, bool* flag) {
    if(furi_string_cmpi_str(arg, TOTP_CLI_COMMAND_UPDATE_ARG_SECRET_PREFIX) == 0) {
        *flag = true;
        *parsed = true;
        return true;
    }

    return false;
}

static TotpIteratorUpdateTokenResult
    update_token_handler(TokenInfo* token_info, const void* context) {
    const struct TotpUpdateContext* context_t = context;

    // Read optional arguments
    FuriString* temp_str = furi_string_alloc();
    bool mask_user_input = true;
    bool update_token_secret = false;
    PlainTokenSecretEncoding token_secret_encoding = PlainTokenSecretEncodingBase32;
    while(args_read_string_and_trim(context_t->args, temp_str)) {
        bool parsed = false;
        if(!totp_cli_try_read_name(token_info, temp_str, context_t->args, &parsed) &&
           !totp_cli_try_read_algo(token_info, temp_str, context_t->args, &parsed) &&
           !totp_cli_try_read_digits(token_info, temp_str, context_t->args, &parsed) &&
           !totp_cli_try_read_duration(token_info, temp_str, context_t->args, &parsed) &&
           !totp_cli_try_read_unsecure_flag(temp_str, &parsed, &mask_user_input) &&
           !totp_cli_try_read_change_secret_flag(temp_str, &parsed, &update_token_secret) &&
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

    if(update_token_secret) {
        // Reading token secret
        furi_string_reset(temp_str);
        TOTP_CLI_PRINTF("Enter token secret and confirm with [ENTER]:\r\n");
        bool token_secret_read = totp_cli_read_line(context_t->cli, temp_str, mask_user_input);
        totp_cli_delete_last_line();
        if(!token_secret_read) {
            furi_string_secure_free(temp_str);
            return TotpIteratorUpdateTokenResultCancelled;
        }

        if(!token_info_set_secret(
               token_info,
               furi_string_get_cstr(temp_str),
               furi_string_size(temp_str),
               token_secret_encoding,
               context_t->crypto_settings)) {
            furi_string_secure_free(temp_str);
            return TotpIteratorUpdateTokenResultInvalidSecret;
        }
    }

    furi_string_secure_free(temp_str);

    return TotpIteratorUpdateTokenResultSuccess;
}

#ifdef TOTP_CLI_RICH_HELP_ENABLED
void totp_cli_command_update_docopt_commands() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_UPDATE "           Update existing token\r\n");
}

void totp_cli_command_update_docopt_usage() {
    TOTP_CLI_PRINTF(
        "  " TOTP_CLI_COMMAND_NAME
        " " DOCOPT_REQUIRED(TOTP_CLI_COMMAND_UPDATE) " " DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_INDEX) " " DOCOPT_OPTIONAL(DOCOPT_OPTION(TOTP_CLI_COMMAND_ARG_ALGO_PREFIX, DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_ALGO))) " " DOCOPT_OPTIONAL(DOCOPT_OPTION(TOTP_CLI_COMMAND_ARG_SECRET_ENCODING_PREFIX, DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_SECRET_ENCODING))) " " DOCOPT_OPTIONAL(DOCOPT_OPTION(TOTP_CLI_COMMAND_ARG_NAME_PREFIX, DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_NAME))) " " DOCOPT_OPTIONAL(
            DOCOPT_OPTION(
                TOTP_CLI_COMMAND_ARG_DIGITS_PREFIX,
                DOCOPT_ARGUMENT(
                    TOTP_CLI_COMMAND_ARG_DIGITS))) " " DOCOPT_OPTIONAL(DOCOPT_OPTION(TOTP_CLI_COMMAND_ARG_DURATION_PREFIX, DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_DURATION))) " " DOCOPT_OPTIONAL(DOCOPT_SWITCH(TOTP_CLI_COMMAND_ARG_UNSECURE_PREFIX)) " " DOCOPT_OPTIONAL(DOCOPT_SWITCH(TOTP_CLI_COMMAND_UPDATE_ARG_SECRET_PREFIX)) " " DOCOPT_MULTIPLE(DOCOPT_OPTIONAL(DOCOPT_OPTION(TOTP_CLI_COMMAND_ARG_AUTOMATION_FEATURE_PREFIX, DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_AUTOMATION_FEATURE)))) "\r\n");
}

void totp_cli_command_update_docopt_options() {
    TOTP_CLI_PRINTF("  " DOCOPT_OPTION(
        TOTP_CLI_COMMAND_ARG_NAME_PREFIX,
        DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_NAME)) "      Token name\r\n");

    TOTP_CLI_PRINTF("  " DOCOPT_SWITCH(
        TOTP_CLI_COMMAND_UPDATE_ARG_SECRET_PREFIX) "             Update token secret\r\n");
}
#endif

void totp_cli_command_update_handle(PluginState* plugin_state, FuriString* args, Cli* cli) {
    if(!totp_cli_ensure_authenticated(plugin_state, cli)) {
        return;
    }

    TokenInfoIteratorContext* iterator_context =
        totp_config_get_token_iterator_context(plugin_state);

    int token_number;
    if(!args_read_int_and_trim(args, &token_number) || token_number <= 0 ||
       (size_t)token_number > totp_token_info_iterator_get_total_count(iterator_context)) {
        totp_cli_print_invalid_arguments();
        return;
    }

    TOTP_CLI_LOCK_UI(plugin_state);

    size_t previous_index = totp_token_info_iterator_get_current_token_index(iterator_context);
    totp_token_info_iterator_go_to(iterator_context, token_number - 1);

    struct TotpUpdateContext update_context = {
        .args = args, .cli = cli, .crypto_settings = &plugin_state->crypto_settings};
    TotpIteratorUpdateTokenResult update_result = totp_token_info_iterator_update_current_token(
        iterator_context, &update_token_handler, &update_context);

    if(update_result == TotpIteratorUpdateTokenResultSuccess) {
        TOTP_CLI_PRINTF_SUCCESS(
            "Token \"%s\" has been successfully updated\r\n",
            furi_string_get_cstr(
                totp_token_info_iterator_get_current_token(iterator_context)->name));
    } else if(update_result == TotpIteratorUpdateTokenResultInvalidArguments) {
        totp_cli_print_invalid_arguments();
    } else if(update_result == TotpIteratorUpdateTokenResultCancelled) {
        TOTP_CLI_PRINTF_INFO("Cancelled by user\r\n");
    } else if(update_result == TotpIteratorUpdateTokenResultInvalidSecret) {
        TOTP_CLI_PRINTF_ERROR("Token secret seems to be invalid and can not be parsed\r\n");
    } else if(update_result == TotpIteratorUpdateTokenResultFileUpdateFailed) {
        totp_cli_print_error_updating_config_file();
    }

    totp_token_info_iterator_go_to(iterator_context, previous_index);
    TOTP_CLI_UNLOCK_UI(plugin_state);
}