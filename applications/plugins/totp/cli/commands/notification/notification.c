#include "notification.h"
#include <lib/toolbox/args.h>
#include "../../../services/config/config.h"
#include "../../../ui/scene_director.h"
#include "../../cli_helpers.h"

#define TOTP_CLI_COMMAND_NOTIFICATION_ARG_METHOD "method"
#define TOTP_CLI_COMMAND_NOTIFICATION_METHOD_NONE "none"
#define TOTP_CLI_COMMAND_NOTIFICATION_METHOD_SOUND "sound"
#define TOTP_CLI_COMMAND_NOTIFICATION_METHOD_VIBRO "vibro"

void totp_cli_command_notification_docopt_commands() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_NOTIFICATION
                    "           Get or set notification method\r\n");
}

void totp_cli_command_notification_docopt_usage() {
    TOTP_CLI_PRINTF(
        "  " TOTP_CLI_COMMAND_NAME " " TOTP_CLI_COMMAND_NOTIFICATION " " DOCOPT_OPTIONAL(
            DOCOPT_MULTIPLE(DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_NOTIFICATION_ARG_METHOD))) "\r\n");
}

void totp_cli_command_notification_docopt_arguments() {
    TOTP_CLI_PRINTF(
        "  " TOTP_CLI_COMMAND_NOTIFICATION_ARG_METHOD
        "      Notification method to be set. Must be one of [" TOTP_CLI_COMMAND_NOTIFICATION_METHOD_NONE
        ", " TOTP_CLI_COMMAND_NOTIFICATION_METHOD_SOUND
        ", " TOTP_CLI_COMMAND_NOTIFICATION_METHOD_VIBRO "]\r\n");
}

static void totp_cli_command_notification_print_method(NotificationMethod method) {
    bool has_previous_method = false;
    if(method & NotificationMethodSound) {
        TOTP_CLI_PRINTF("\"" TOTP_CLI_COMMAND_NOTIFICATION_METHOD_SOUND "\"");
        has_previous_method = true;
    }
    if(method & NotificationMethodVibro) {
        if(has_previous_method) {
            TOTP_CLI_PRINTF(" and ");
        }

        TOTP_CLI_PRINTF("\"" TOTP_CLI_COMMAND_NOTIFICATION_METHOD_VIBRO "\"");
    }
    if(method == NotificationMethodNone) {
        TOTP_CLI_PRINTF("\"" TOTP_CLI_COMMAND_NOTIFICATION_METHOD_NONE "\"");
    }
}

void totp_cli_command_notification_handle(PluginState* plugin_state, FuriString* args, Cli* cli) {
    if(!totp_cli_ensure_authenticated(plugin_state, cli)) {
        return;
    }

    FuriString* temp_str = furi_string_alloc();
    bool new_method_provided = false;
    NotificationMethod new_method = NotificationMethodNone;
    bool args_valid = true;
    while(args_read_string_and_trim(args, temp_str)) {
        if(furi_string_cmpi_str(temp_str, TOTP_CLI_COMMAND_NOTIFICATION_METHOD_NONE) == 0) {
            new_method_provided = true;
            new_method = NotificationMethodNone;
        } else if(furi_string_cmpi_str(temp_str, TOTP_CLI_COMMAND_NOTIFICATION_METHOD_SOUND) == 0) {
            new_method_provided = true;
            new_method |= NotificationMethodSound;
        } else if(furi_string_cmpi_str(temp_str, TOTP_CLI_COMMAND_NOTIFICATION_METHOD_VIBRO) == 0) {
            new_method_provided = true;
            new_method |= NotificationMethodVibro;
        } else {
            args_valid = false;
            break;
        }
    }

    do {
        if(!args_valid) {
            TOTP_CLI_PRINT_INVALID_ARGUMENTS();
            break;
        }

        if(new_method_provided) {
            Scene previous_scene = TotpSceneNone;
            if(plugin_state->current_scene == TotpSceneGenerateToken ||
               plugin_state->current_scene == TotpSceneAppSettings) {
                previous_scene = plugin_state->current_scene;
                totp_scene_director_activate_scene(plugin_state, TotpSceneNone, NULL);
            }

            plugin_state->notification_method = new_method;
            totp_config_file_update_notification_method(new_method);

            if(previous_scene != TotpSceneNone) {
                totp_scene_director_activate_scene(plugin_state, previous_scene, NULL);
            }

            TOTP_CLI_PRINTF("Notification method is set to ");
            totp_cli_command_notification_print_method(new_method);
            cli_nl();
        } else {
            TOTP_CLI_PRINTF("Current notification method is ");
            totp_cli_command_notification_print_method(plugin_state->notification_method);
            cli_nl();
        }
    } while(false);

    furi_string_free(temp_str);
}