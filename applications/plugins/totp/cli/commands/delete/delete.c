#include "delete.h"

#include <stdlib.h>
#include <ctype.h>
#include <lib/toolbox/args.h>
#include "../../../lib/list/list.h"
#include "../../../services/config/config.h"
#include "../../cli_helpers.h"
#include "../../../ui/scene_director.h"

#define TOTP_CLI_COMMAND_DELETE_ARG_INDEX "index"
#define TOTP_CLI_COMMAND_DELETE_ARG_FORCE_SUFFIX "-f"

void totp_cli_command_delete_docopt_commands() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_DELETE ", " TOTP_CLI_COMMAND_DELETE_ALT
                    "       Delete existing token\r\n");
}

void totp_cli_command_delete_docopt_usage() {
    TOTP_CLI_PRINTF(
        "  " TOTP_CLI_COMMAND_NAME
        " " DOCOPT_REQUIRED(TOTP_CLI_COMMAND_DELETE " | " TOTP_CLI_COMMAND_DELETE_ALT) " " DOCOPT_ARGUMENT(
            TOTP_CLI_COMMAND_DELETE_ARG_INDEX) " " DOCOPT_OPTIONAL(DOCOPT_SWITCH(TOTP_CLI_COMMAND_DELETE_ARG_FORCE_SUFFIX)) "\r\n");
}

void totp_cli_command_delete_docopt_arguments() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_DELETE_ARG_INDEX "       Token index in the list\r\n");
}

void totp_cli_command_delete_docopt_options() {
    TOTP_CLI_PRINTF("  " DOCOPT_SWITCH(
        TOTP_CLI_COMMAND_DELETE_ARG_FORCE_SUFFIX) "             Force command to do not ask user for interactive confirmation\r\n");
}

void totp_cli_command_delete_handle(PluginState* plugin_state, FuriString* args, Cli* cli) {
    int token_number;
    if(!args_read_int_and_trim(args, &token_number) || token_number <= 0 ||
       token_number > plugin_state->tokens_count) {
        TOTP_CLI_PRINT_INVALID_ARGUMENTS();
        return;
    }

    FuriString* temp_str = furi_string_alloc();
    bool confirm_needed = true;
    if(args_read_string_and_trim(args, temp_str)) {
        if(furi_string_cmpi_str(temp_str, TOTP_CLI_COMMAND_DELETE_ARG_FORCE_SUFFIX) == 0) {
            confirm_needed = false;
        } else {
            TOTP_CLI_PRINTF("Unknown argument \"%s\"\r\n", furi_string_get_cstr(temp_str));
            TOTP_CLI_PRINT_INVALID_ARGUMENTS();
            furi_string_free(temp_str);
            return;
        }
    }
    furi_string_free(temp_str);

    if(!totp_cli_ensure_authenticated(plugin_state, cli)) {
        return;
    }

    ListNode* list_node = list_element_at(plugin_state->tokens_list, token_number - 1);

    TokenInfo* token_info = list_node->data;

    bool confirmed = !confirm_needed;
    if(confirm_needed) {
        TOTP_CLI_PRINTF("WARNING!\r\n");
        TOTP_CLI_PRINTF(
            "TOKEN \"%s\" WILL BE PERMANENTLY DELETED WITHOUT ABILITY TO RECOVER IT.\r\n",
            token_info->name);
        TOTP_CLI_PRINTF("Confirm? [y/n]\r\n");
        fflush(stdout);
        char user_pick;
        do {
            user_pick = tolower(cli_getc(cli));
        } while(user_pick != 'y' && user_pick != 'n' && user_pick != CliSymbolAsciiCR &&
                user_pick != CliSymbolAsciiETX && user_pick != CliSymbolAsciiEsc);

        confirmed = user_pick == 'y' || user_pick == CliSymbolAsciiCR;
    }

    if(confirmed) {
        if(!totp_cli_ensure_authenticated(plugin_state, cli)) {
            return;
        }

        bool activate_generate_token_scene = false;
        if(plugin_state->current_scene != TotpSceneAuthentication) {
            totp_scene_director_activate_scene(plugin_state, TotpSceneNone, NULL);
            activate_generate_token_scene = true;
        }

        plugin_state->tokens_list = list_remove(plugin_state->tokens_list, list_node);
        plugin_state->tokens_count--;

        if(totp_full_save_config_file(plugin_state) == TotpConfigFileUpdateSuccess) {
            TOTP_CLI_PRINTF("Token \"%s\" has been successfully deleted\r\n", token_info->name);
        } else {
            TOTP_CLI_PRINT_ERROR_UPDATING_CONFIG_FILE();
        }

        token_info_free(token_info);

        if(activate_generate_token_scene) {
            totp_scene_director_activate_scene(plugin_state, TotpSceneGenerateToken, NULL);
        }
    } else {
        TOTP_CLI_PRINTF("User not confirmed\r\n");
    }
}