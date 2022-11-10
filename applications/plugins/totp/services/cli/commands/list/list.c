#include "list.h"
#include <stdlib.h>
#include "../../../list/list.h"
#include "../../../../types/token_info.h"
#include "../../../config/constants.h"
#include "../../cli_helpers.h"

static char* get_algo_as_cstr(TokenHashAlgo algo) {
    switch(algo) {
    case SHA1:
        return TOTP_CONFIG_TOKEN_ALGO_SHA1_NAME;
    case SHA256:
        return TOTP_CONFIG_TOKEN_ALGO_SHA256_NAME;
    case SHA512:
        return TOTP_CONFIG_TOKEN_ALGO_SHA512_NAME;
    default:
        break;
    }

    return "UNKNOWN";
}

static uint8_t get_digits_as_int(TokenDigitsCount digits) {
    switch(digits) {
    case TOTP_6_DIGITS:
        return 6;
    case TOTP_8_DIGITS:
        return 8;
    default:
        break;
    }

    return 6;
}

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

    ListNode* node = plugin_state->tokens_list;

    TOTP_CLI_PRINTF("+-----+-----------------------------+--------+--------+\r\n");
    TOTP_CLI_PRINTF("| %-*s | %-*s | %-*s | %-s |\r\n", 3, "#", 27, "Name", 6, "Algo", "Digits");
    TOTP_CLI_PRINTF("+-----+-----------------------------+--------+--------+\r\n");
    uint16_t index = 1;
    while(node != NULL) {
        TokenInfo* token_info = (TokenInfo*)node->data;
        token_info_get_digits_count(token_info);
        TOTP_CLI_PRINTF(
            "| %-3" PRIu16 " | %-27.27s | %-6s | %-6" PRIu8 " |\r\n",
            index,
            token_info->name,
            get_algo_as_cstr(token_info->algo),
            get_digits_as_int(token_info->digits));
        node = node->next;
        index++;
    }
    TOTP_CLI_PRINTF("+-----+-----------------------------+--------+--------+\r\n");
}