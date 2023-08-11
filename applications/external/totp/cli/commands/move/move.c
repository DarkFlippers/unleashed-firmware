#include "move.h"

#include <stdlib.h>
#include <lib/toolbox/args.h>
#include "../../../types/token_info.h"
#include "../../../services/config/config.h"
#include "../../cli_helpers.h"
#include "../../../ui/scene_director.h"
#include "../../common_command_arguments.h"

#define TOTP_CLI_COMMAND_MOVE_ARG_NEW_INDEX "new_index"

#ifdef TOTP_CLI_RICH_HELP_ENABLED
void totp_cli_command_move_docopt_commands() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_MOVE ", " TOTP_CLI_COMMAND_MOVE_ALT
                    "         Move token\r\n");
}

void totp_cli_command_move_docopt_usage() {
    TOTP_CLI_PRINTF(
        "  " TOTP_CLI_COMMAND_NAME
        " " DOCOPT_REQUIRED(TOTP_CLI_COMMAND_MOVE " | " TOTP_CLI_COMMAND_MOVE_ALT) " " DOCOPT_ARGUMENT(
            TOTP_CLI_COMMAND_ARG_INDEX) " " DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_MOVE_ARG_NEW_INDEX) "\r\n");
}

void totp_cli_command_move_docopt_arguments() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_MOVE_ARG_NEW_INDEX
                    "     New token index in the list\r\n");
}
#endif

void totp_cli_command_move_handle(PluginState* plugin_state, FuriString* args, Cli* cli) {
    if(!totp_cli_ensure_authenticated(plugin_state, cli)) {
        return;
    }

    int token_number;
    TokenInfoIteratorContext* iterator_context =
        totp_config_get_token_iterator_context(plugin_state);
    size_t total_count = totp_token_info_iterator_get_total_count(iterator_context);
    if(!args_read_int_and_trim(args, &token_number) || token_number < 1 ||
       (size_t)token_number > total_count) {
        totp_cli_print_invalid_arguments();
        return;
    }

    int new_token_number = 0;

    if(!args_read_int_and_trim(args, &new_token_number) || new_token_number < 1 ||
       (size_t)new_token_number > total_count) {
        totp_cli_print_invalid_arguments();
        return;
    }

    if(token_number == new_token_number) {
        TOTP_CLI_PRINTF_ERROR("New token number matches current token number\r\n");
        return;
    }

    TOTP_CLI_LOCK_UI(plugin_state);

    size_t token_index = token_number - 1;
    size_t new_token_index = new_token_number - 1;

    size_t original_token_index =
        totp_token_info_iterator_get_current_token_index(iterator_context);

    totp_cli_print_processing();

    if(totp_token_info_iterator_go_to(iterator_context, token_index) &&
       totp_token_info_iterator_move_current_token_info(iterator_context, new_token_index)) {
        totp_cli_delete_last_line();
        TOTP_CLI_PRINTF_SUCCESS(
            "Token \"%s\" has been successfully updated\r\n",
            furi_string_get_cstr(
                totp_token_info_iterator_get_current_token(iterator_context)->name));
    } else {
        totp_cli_delete_last_line();
        totp_cli_print_error_updating_config_file();
    }

    totp_token_info_iterator_go_to(iterator_context, original_token_index);

    TOTP_CLI_UNLOCK_UI(plugin_state);
}