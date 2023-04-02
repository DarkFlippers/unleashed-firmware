#include "move.h"

#include <stdlib.h>
#include <lib/toolbox/args.h>
#include "../../../lib/list/list.h"
#include "../../../types/token_info.h"
#include "../../../services/config/config.h"
#include "../../cli_helpers.h"
#include "../../../ui/scene_director.h"
#include "../../common_command_arguments.h"

#define TOTP_CLI_COMMAND_MOVE_ARG_NEW_INDEX "new_index"

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

void totp_cli_command_move_handle(PluginState* plugin_state, FuriString* args, Cli* cli) {
    if(!totp_cli_ensure_authenticated(plugin_state, cli)) {
        return;
    }

    int token_index;
    if(!args_read_int_and_trim(args, &token_index) || token_index < 1 ||
       token_index > plugin_state->tokens_count) {
        TOTP_CLI_PRINT_INVALID_ARGUMENTS();
        return;
    }

    int new_token_index = 0;

    if(!args_read_int_and_trim(args, &new_token_index) || new_token_index < 1 ||
       new_token_index > plugin_state->tokens_count) {
        TOTP_CLI_PRINT_INVALID_ARGUMENTS();
        return;
    }

    bool activate_generate_token_scene = false;
    if(plugin_state->current_scene != TotpSceneAuthentication) {
        totp_scene_director_activate_scene(plugin_state, TotpSceneNone, NULL);
        activate_generate_token_scene = true;
    }

    TokenInfo* token_info = NULL;
    plugin_state->tokens_list =
        list_remove_at(plugin_state->tokens_list, token_index - 1, (void**)&token_info);
    furi_check(token_info != NULL);
    plugin_state->tokens_list =
        list_insert_at(plugin_state->tokens_list, new_token_index - 1, token_info);

    if(totp_full_save_config_file(plugin_state) == TotpConfigFileUpdateSuccess) {
        TOTP_CLI_PRINTF_SUCCESS(
            "Token \"%s\" has been successfully updated\r\n", token_info->name);
    } else {
        TOTP_CLI_PRINT_ERROR_UPDATING_CONFIG_FILE();
    }

    if(activate_generate_token_scene) {
        totp_scene_director_activate_scene(plugin_state, TotpSceneGenerateToken, NULL);
    }
}