#include "timezone.h"
#include <lib/toolbox/args.h>
#include "../../../services/config/config.h"
#include "../../../ui/scene_director.h"
#include "../../cli_helpers.h"

#define TOTP_CLI_COMMAND_TIMEZONE_ARG_TIMEZONE "timezone"

void totp_cli_command_timezone_docopt_commands() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_TIMEZONE ", " TOTP_CLI_COMMAND_TIMEZONE_ALT
                    "     Get or set current timezone\r\n");
}

void totp_cli_command_timezone_docopt_usage() {
    TOTP_CLI_PRINTF(
        "  " TOTP_CLI_COMMAND_NAME
        " " DOCOPT_REQUIRED(TOTP_CLI_COMMAND_TIMEZONE " | " TOTP_CLI_COMMAND_TIMEZONE_ALT) " " DOCOPT_OPTIONAL(
            DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_TIMEZONE_ARG_TIMEZONE)) "\r\n");
}

void totp_cli_command_timezone_docopt_arguments() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_TIMEZONE_ARG_TIMEZONE
                    "    Timezone offset in hours to be set\r\n");
}

void totp_cli_command_timezone_handle(PluginState* plugin_state, FuriString* args, Cli* cli) {
    if(!totp_cli_ensure_authenticated(plugin_state, cli)) {
        return;
    }

    FuriString* temp_str = furi_string_alloc();
    if(args_read_string_and_trim(args, temp_str)) {
        char* strtof_endptr;
        float tz = strtof(furi_string_get_cstr(temp_str), &strtof_endptr);
        if(*strtof_endptr == 0 && tz >= -12.75f && tz <= 12.75f) {
            plugin_state->timezone_offset = tz;
            if(totp_config_file_update_timezone_offset(tz) == TotpConfigFileUpdateSuccess) {
                TOTP_CLI_PRINTF_SUCCESS("Timezone is set to %f\r\n", tz);
            } else {
                TOTP_CLI_PRINT_ERROR_UPDATING_CONFIG_FILE();
            }
            if(plugin_state->current_scene == TotpSceneGenerateToken) {
                totp_scene_director_activate_scene(plugin_state, TotpSceneNone, NULL);
                totp_scene_director_activate_scene(plugin_state, TotpSceneGenerateToken, NULL);
            } else if(plugin_state->current_scene == TotpSceneAppSettings) {
                totp_scene_director_activate_scene(plugin_state, TotpSceneNone, NULL);
                totp_scene_director_activate_scene(plugin_state, TotpSceneAppSettings, NULL);
            }
        } else {
            TOTP_CLI_PRINTF_ERROR("Invalid timezone offset\r\n");
        }
    } else {
        TOTP_CLI_PRINTF_INFO("Current timezone offset is %f\r\n", plugin_state->timezone_offset);
    }
    furi_string_free(temp_str);
}