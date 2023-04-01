#include "list.h"
#include <stdlib.h>
#include "../../../lib/list/list.h"
#include "../../../types/token_info.h"
#include "../../../services/config/constants.h"
#include "../../cli_helpers.h"

void totp_cli_command_list_docopt_commands() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_LIST ", " TOTP_CLI_COMMAND_LIST_ALT
                    "         List all available tokens\r\n");
}

void totp_cli_command_list_docopt_usage() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_NAME " " DOCOPT_REQUIRED(
        TOTP_CLI_COMMAND_LIST " | " TOTP_CLI_COMMAND_LIST_ALT) "\r\n");
}

void totp_cli_command_list_handle(PluginState* plugin_state, Cli* cli) {
    if(!totp_cli_ensure_authenticated(plugin_state, cli)) {
        return;
    }

    if(plugin_state->tokens_list == NULL) {
        TOTP_CLI_PRINTF("There are no tokens");
        return;
    }

    TOTP_CLI_PRINTF("+-----+---------------------------+--------+----+-----+\r\n");
    TOTP_CLI_PRINTF("| %-3s | %-25s | %-6s | %-s | %-s |\r\n", "#", "Name", "Algo", "Ln", "Dur");
    TOTP_CLI_PRINTF("+-----+---------------------------+--------+----+-----+\r\n");
    uint16_t index = 1;
    TOTP_LIST_FOREACH(plugin_state->tokens_list, node, {
        TokenInfo* token_info = (TokenInfo*)node->data;
        TOTP_CLI_PRINTF(
            "| %-3" PRIu16 " | %-25.25s | %-6s | %-2" PRIu8 " | %-3" PRIu8 " |\r\n",
            index,
            token_info->name,
            token_info_get_algo_as_cstr(token_info),
            token_info->digits,
            token_info->duration);
        index++;
    });
    TOTP_CLI_PRINTF("+-----+---------------------------+--------+----+-----+\r\n");
}