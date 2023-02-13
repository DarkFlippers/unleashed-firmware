#include "move.h"

#include <stdlib.h>
#include <lib/toolbox/args.h>
#include "../../../lib/list/list.h"
#include "../../../types/token_info.h"
#include "../../../services/config/config.h"
#include "../../cli_helpers.h"
#include "../../../ui/scene_director.h"

#define TOTP_CLI_COMMAND_MOVE_ARG_INDEX "index"

#define TOTP_CLI_COMMAND_MOVE_ARG_NEW_NAME "name"
#define TOTP_CLI_COMMAND_MOVE_ARG_NEW_NAME_PREFIX "-n"

#define TOTP_CLI_COMMAND_MOVE_ARG_NEW_INDEX "index"
#define TOTP_CLI_COMMAND_MOVE_ARG_NEW_INDEX_PREFIX "-i"

void totp_cli_command_move_docopt_commands() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_MOVE ", " TOTP_CLI_COMMAND_MOVE_ALT
                    "         Move\\rename token\r\n");
}

void totp_cli_command_move_docopt_usage() {
    TOTP_CLI_PRINTF(
        "  " TOTP_CLI_COMMAND_NAME
        " " DOCOPT_REQUIRED(TOTP_CLI_COMMAND_MOVE " | " TOTP_CLI_COMMAND_MOVE_ALT) " " DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_MOVE_ARG_INDEX) " " DOCOPT_OPTIONAL(
            DOCOPT_OPTION(
                TOTP_CLI_COMMAND_MOVE_ARG_NEW_NAME_PREFIX,
                DOCOPT_ARGUMENT(
                    TOTP_CLI_COMMAND_MOVE_ARG_NEW_NAME))) " " DOCOPT_OPTIONAL(DOCOPT_OPTION(TOTP_CLI_COMMAND_MOVE_ARG_NEW_INDEX_PREFIX, DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_MOVE_ARG_NEW_INDEX))) "\r\n");
}

void totp_cli_command_move_docopt_options() {
    TOTP_CLI_PRINTF("  " DOCOPT_OPTION(
        TOTP_CLI_COMMAND_MOVE_ARG_NEW_NAME_PREFIX,
        DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_MOVE_ARG_NEW_NAME)) "      New token name\r\n");
    TOTP_CLI_PRINTF("  " DOCOPT_OPTION(
        TOTP_CLI_COMMAND_MOVE_ARG_NEW_INDEX_PREFIX,
        DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_MOVE_ARG_NEW_INDEX)) "     New token index\r\n");
}

void totp_cli_command_move_handle(PluginState* plugin_state, FuriString* args, Cli* cli) {
    int token_index;
    if(!args_read_int_and_trim(args, &token_index)) {
        TOTP_CLI_PRINT_INVALID_ARGUMENTS();
        return;
    }

    if(!totp_cli_ensure_authenticated(plugin_state, cli)) {
        return;
    }

    if(token_index < 1 || token_index > plugin_state->tokens_count) {
        TOTP_CLI_PRINT_INVALID_ARGUMENTS();
        return;
    }

    FuriString* temp_str = furi_string_alloc();

    char* new_token_name = NULL;
    int new_token_index = 0;

    while(args_read_string_and_trim(args, temp_str)) {
        bool parsed = false;
        if(furi_string_cmpi_str(temp_str, TOTP_CLI_COMMAND_MOVE_ARG_NEW_NAME_PREFIX) == 0) {
            if(!args_read_string_and_trim(args, temp_str)) {
                TOTP_CLI_PRINTF(
                    "Missed value for argument \"" TOTP_CLI_COMMAND_MOVE_ARG_NEW_NAME_PREFIX
                    "\"\r\n");
            } else {
                if(new_token_name != NULL) {
                    free(new_token_name);
                }

                new_token_name = malloc(furi_string_size(temp_str) + 1);
                if(new_token_name == NULL) {
                    furi_string_free(temp_str);
                    return;
                }

                strlcpy(
                    new_token_name,
                    furi_string_get_cstr(temp_str),
                    furi_string_size(temp_str) + 1);
                parsed = true;
            }
        } else if(furi_string_cmpi_str(temp_str, TOTP_CLI_COMMAND_MOVE_ARG_NEW_INDEX_PREFIX) == 0) {
            if(!args_read_int_and_trim(args, &new_token_index)) {
                TOTP_CLI_PRINTF(
                    "Missed value for argument \"" TOTP_CLI_COMMAND_MOVE_ARG_NEW_INDEX_PREFIX
                    "\"\r\n");
            } else if(new_token_index < 1 || new_token_index > plugin_state->tokens_count) {
                TOTP_CLI_PRINTF(
                    "\"%" PRId16
                    "\" is incorrect value for argument \"" TOTP_CLI_COMMAND_MOVE_ARG_NEW_INDEX_PREFIX
                    "\"\r\n",
                    new_token_index);
            } else {
                parsed = true;
            }
        } else {
            TOTP_CLI_PRINTF("Unknown argument \"%s\"\r\n", furi_string_get_cstr(temp_str));
        }

        if(!parsed) {
            TOTP_CLI_PRINT_INVALID_ARGUMENTS();
            furi_string_free(temp_str);
            if(new_token_name != NULL) {
                free(new_token_name);
            }
            return;
        }
    }

    if(!totp_cli_ensure_authenticated(plugin_state, cli)) {
        furi_string_free(temp_str);
        if(new_token_name != NULL) {
            free(new_token_name);
        }
        return;
    }

    bool activate_generate_token_scene = false;
    if(plugin_state->current_scene != TotpSceneAuthentication) {
        totp_scene_director_activate_scene(plugin_state, TotpSceneNone, NULL);
        activate_generate_token_scene = true;
    }

    bool token_updated = false;
    TokenInfo* token_info = NULL;
    if(new_token_index > 0) {
        plugin_state->tokens_list =
            list_remove_at(plugin_state->tokens_list, token_index - 1, (void**)&token_info);
        furi_check(token_info != NULL);
        plugin_state->tokens_list =
            list_insert_at(plugin_state->tokens_list, new_token_index - 1, token_info);
        token_updated = true;
    } else {
        token_info = list_element_at(plugin_state->tokens_list, token_index - 1)->data;
    }

    if(new_token_name != NULL) {
        free(token_info->name);
        token_info->name = new_token_name;
        token_updated = true;
    }

    if(token_updated) {
        if(totp_full_save_config_file(plugin_state) == TotpConfigFileUpdateSuccess) {
            TOTP_CLI_PRINTF("Token \"%s\" has been successfully updated\r\n", token_info->name);
        } else {
            TOTP_CLI_PRINT_ERROR_UPDATING_CONFIG_FILE();
        }
    } else {
        TOTP_CLI_PRINT_INVALID_ARGUMENTS();
    }

    if(activate_generate_token_scene) {
        totp_scene_director_activate_scene(plugin_state, TotpSceneGenerateToken, NULL);
    }

    furi_string_free(temp_str);
}