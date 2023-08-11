#include "delete.h"

#include <stdlib.h>
#include <ctype.h>
#include <lib/toolbox/args.h>
#include "../../../services/config/config.h"
#include "../../cli_helpers.h"
#include "../../../ui/scene_director.h"
#include "../../common_command_arguments.h"

#define TOTP_CLI_COMMAND_DELETE_ARG_FORCE_PREFIX "-f"

#ifdef TOTP_CLI_RICH_HELP_ENABLED
void totp_cli_command_delete_docopt_commands() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_DELETE ", " TOTP_CLI_COMMAND_DELETE_ALT
                    "       Delete existing token\r\n");
}

void totp_cli_command_delete_docopt_usage() {
    TOTP_CLI_PRINTF(
        "  " TOTP_CLI_COMMAND_NAME
        " " DOCOPT_REQUIRED(TOTP_CLI_COMMAND_DELETE " | " TOTP_CLI_COMMAND_DELETE_ALT) " " DOCOPT_ARGUMENT(
            TOTP_CLI_COMMAND_ARG_INDEX) " " DOCOPT_OPTIONAL(DOCOPT_SWITCH(TOTP_CLI_COMMAND_DELETE_ARG_FORCE_PREFIX)) "\r\n");
}

void totp_cli_command_delete_docopt_arguments() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_ARG_INDEX "         Token index in the list\r\n");
}

void totp_cli_command_delete_docopt_options() {
    TOTP_CLI_PRINTF("  " DOCOPT_SWITCH(
        TOTP_CLI_COMMAND_DELETE_ARG_FORCE_PREFIX) "             Force command to do not ask user for interactive confirmation\r\n");
}
#endif

void totp_cli_command_delete_handle(PluginState* plugin_state, FuriString* args, Cli* cli) {
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

    FuriString* temp_str = furi_string_alloc();
    bool confirm_needed = true;
    if(args_read_string_and_trim(args, temp_str)) {
        if(furi_string_cmpi_str(temp_str, TOTP_CLI_COMMAND_DELETE_ARG_FORCE_PREFIX) == 0) {
            confirm_needed = false;
        } else {
            totp_cli_printf_unknown_argument(temp_str);
            totp_cli_print_invalid_arguments();
            furi_string_free(temp_str);
            return;
        }
    }
    furi_string_free(temp_str);

    TOTP_CLI_LOCK_UI(plugin_state);

    size_t original_token_index =
        totp_token_info_iterator_get_current_token_index(iterator_context);
    totp_token_info_iterator_go_to(iterator_context, token_number - 1);
    const TokenInfo* token_info = totp_token_info_iterator_get_current_token(iterator_context);
    const char* token_info_name = furi_string_get_cstr(token_info->name);

    bool confirmed = !confirm_needed;
    if(confirm_needed) {
        TOTP_CLI_PRINTF_WARNING("WARNING!\r\n");
        TOTP_CLI_PRINTF_WARNING(
            "TOKEN \"%s\" WILL BE PERMANENTLY DELETED WITHOUT ABILITY TO RECOVER IT.\r\n",
            token_info_name);
        TOTP_CLI_PRINTF_WARNING("Confirm? [y/n]\r\n");
        fflush(stdout);
        char user_pick;
        do {
            user_pick = tolower(cli_getc(cli));
        } while(user_pick != 'y' && user_pick != 'n' && user_pick != CliSymbolAsciiCR &&
                user_pick != CliSymbolAsciiETX && user_pick != CliSymbolAsciiEsc);

        confirmed = user_pick == 'y' || user_pick == CliSymbolAsciiCR;
    }

    if(confirmed) {
        totp_cli_print_processing();
        if(totp_token_info_iterator_remove_current_token_info(iterator_context)) {
            totp_cli_delete_last_line();
            TOTP_CLI_PRINTF_SUCCESS(
                "Token \"%s\" has been successfully deleted\r\n", token_info_name);
            totp_token_info_iterator_go_to(iterator_context, 0);
        } else {
            totp_cli_delete_last_line();
            totp_cli_print_error_updating_config_file();
            totp_token_info_iterator_go_to(iterator_context, original_token_index);
        }
    } else {
        TOTP_CLI_PRINTF_INFO("User has not confirmed\r\n");
        totp_token_info_iterator_go_to(iterator_context, original_token_index);
    }

    TOTP_CLI_UNLOCK_UI(plugin_state);
}