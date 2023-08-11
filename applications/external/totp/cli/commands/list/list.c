#include "list.h"
#include <stdlib.h>
#include "../../../types/token_info.h"
#include "../../../services/config/constants.h"
#include "../../../services/config/config.h"
#include "../../../ui/scene_director.h"
#include "../../cli_helpers.h"

#ifdef TOTP_CLI_RICH_HELP_ENABLED
void totp_cli_command_list_docopt_commands() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_LIST ", " TOTP_CLI_COMMAND_LIST_ALT
                    "         List all available tokens\r\n");
}

void totp_cli_command_list_docopt_usage() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_NAME " " DOCOPT_REQUIRED(
        TOTP_CLI_COMMAND_LIST " | " TOTP_CLI_COMMAND_LIST_ALT) "\r\n");
}
#endif

void totp_cli_command_list_handle(PluginState* plugin_state, Cli* cli) {
    if(!totp_cli_ensure_authenticated(plugin_state, cli)) {
        return;
    }

    TokenInfoIteratorContext* iterator_context =
        totp_config_get_token_iterator_context(plugin_state);
    size_t total_count = totp_token_info_iterator_get_total_count(iterator_context);
    if(total_count <= 0) {
        TOTP_CLI_PRINTF("There are no tokens");
        return;
    }

    TOTP_CLI_LOCK_UI(plugin_state);

    size_t original_index = totp_token_info_iterator_get_current_token_index(iterator_context);

    TOTP_CLI_PRINTF("+-----+---------------------------+--------+----+-----+\r\n");
    TOTP_CLI_PRINTF("| %-3s | %-25s | %-6s | %-s | %-s |\r\n", "#", "Name", "Algo", "Ln", "Dur");
    TOTP_CLI_PRINTF("+-----+---------------------------+--------+----+-----+\r\n");
    for(size_t i = 0; i < total_count; i++) {
        totp_token_info_iterator_go_to(iterator_context, i);
        const TokenInfo* token_info = totp_token_info_iterator_get_current_token(iterator_context);
        TOTP_CLI_PRINTF(
            "| %-3" PRIu16 " | %-25.25s | %-6s | %-2" PRIu8 " | %-3" PRIu8 " |\r\n",
            i + 1,
            furi_string_get_cstr(token_info->name),
            token_info_get_algo_as_cstr(token_info),
            token_info->digits,
            token_info->duration);
    }

    TOTP_CLI_PRINTF("+-----+---------------------------+--------+----+-----+\r\n");

    totp_token_info_iterator_go_to(iterator_context, original_index);

    TOTP_CLI_UNLOCK_UI(plugin_state);
}