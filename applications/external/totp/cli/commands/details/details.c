#include "details.h"
#include <stdlib.h>
#include <lib/toolbox/args.h>
#include "../../../types/token_info.h"
#include "../../../services/config/constants.h"
#include "../../../services/config/config.h"
#include "../../../ui/scene_director.h"
#include "../../cli_helpers.h"
#include "../../common_command_arguments.h"

#define TOTP_CLI_PRINTF_AUTOMATION_FEATURE(description, header_printed) \
    do {                                                                \
        TOTP_CLI_PRINTF(                                                \
            "| %-20s | %-28.28s |\r\n",                                 \
            header_printed ? "" : "Automation features",                \
            description);                                               \
        header_printed = true;                                          \
    } while(false)

static void print_automation_features(const TokenInfo* token_info) {
    if(token_info->automation_features == TokenAutomationFeatureNone) {
        TOTP_CLI_PRINTF("| %-20s | %-28.28s |\r\n", "Automation features", "None");
        return;
    }

    bool header_printed = false;
    if(token_info->automation_features & TokenAutomationFeatureEnterAtTheEnd) {
        TOTP_CLI_PRINTF_AUTOMATION_FEATURE("Type <Enter> key at the end", header_printed);
    }

    if(token_info->automation_features & TokenAutomationFeatureTabAtTheEnd) {
        TOTP_CLI_PRINTF_AUTOMATION_FEATURE("Type <Tab> key at the end", header_printed);
    }

    if(token_info->automation_features & TokenAutomationFeatureTypeSlower) {
        TOTP_CLI_PRINTF_AUTOMATION_FEATURE("Type slower", header_printed);
    }
}

#ifdef TOTP_CLI_RICH_HELP_ENABLED
void totp_cli_command_details_docopt_commands() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_DETAILS ", " TOTP_CLI_COMMAND_DETAILS_ALT
                    "      Displays token details\r\n");
}

void totp_cli_command_details_docopt_usage() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_NAME " " DOCOPT_REQUIRED(
        TOTP_CLI_COMMAND_DETAILS
        " | " TOTP_CLI_COMMAND_DETAILS_ALT) " " DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_ARG_INDEX) "\r\n");
}
#endif

void totp_cli_command_details_handle(PluginState* plugin_state, FuriString* args, Cli* cli) {
    if(!totp_cli_ensure_authenticated(plugin_state, cli)) {
        return;
    }

    int token_number;
    TokenInfoIteratorContext* iterator_context =
        totp_config_get_token_iterator_context(plugin_state);
    if(!args_read_int_and_trim(args, &token_number) || token_number <= 0 ||
       (size_t)token_number > totp_token_info_iterator_get_total_count(iterator_context)) {
        totp_cli_print_invalid_arguments();
        return;
    }

    TOTP_CLI_LOCK_UI(plugin_state);

    size_t original_token_index =
        totp_token_info_iterator_get_current_token_index(iterator_context);
    if(totp_token_info_iterator_go_to(iterator_context, token_number - 1)) {
        const TokenInfo* token_info = totp_token_info_iterator_get_current_token(iterator_context);

        TOTP_CLI_PRINTF("+----------------------+------------------------------+\r\n");
        TOTP_CLI_PRINTF("| %-20s | %-28s |\r\n", "Property", "Value");
        TOTP_CLI_PRINTF("+----------------------+------------------------------+\r\n");
        TOTP_CLI_PRINTF("| %-20s | %-28d |\r\n", "Index", token_number);
        TOTP_CLI_PRINTF(
            "| %-20s | %-28.28s |\r\n", "Name", furi_string_get_cstr(token_info->name));
        TOTP_CLI_PRINTF(
            "| %-20s | %-28s |\r\n", "Hashing algorithm", token_info_get_algo_as_cstr(token_info));
        TOTP_CLI_PRINTF("| %-20s | %-28" PRIu8 " |\r\n", "Number of digits", token_info->digits);
        TOTP_CLI_PRINTF(
            "| %-20s | %" PRIu8 " sec.%-21s |\r\n", "Token lifetime", token_info->duration, " ");
        print_automation_features(token_info);
        TOTP_CLI_PRINTF("+----------------------+------------------------------+\r\n");
    } else {
        totp_cli_print_error_loading_token_info();
    }

    totp_token_info_iterator_go_to(iterator_context, original_token_index);

    TOTP_CLI_UNLOCK_UI(plugin_state);
}