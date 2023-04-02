#include "automation.h"
#include <lib/toolbox/args.h>
#include "../../../services/config/config.h"
#include "../../../ui/scene_director.h"
#include "../../cli_helpers.h"

#define TOTP_CLI_COMMAND_AUTOMATION_ARG_METHOD "automation"
#define TOTP_CLI_COMMAND_AUTOMATION_METHOD_NONE "none"
#define TOTP_CLI_COMMAND_AUTOMATION_METHOD_USB "usb"
#ifdef TOTP_BADBT_TYPE_ENABLED
#define TOTP_CLI_COMMAND_AUTOMATION_METHOD_BT "bt"
#endif

void totp_cli_command_automation_docopt_commands() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_AUTOMATION "       Get or set automation method\r\n");
}

void totp_cli_command_automation_docopt_usage() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_NAME " " TOTP_CLI_COMMAND_AUTOMATION " " DOCOPT_OPTIONAL(
        DOCOPT_MULTIPLE(DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_AUTOMATION_ARG_METHOD))) "\r\n");
}

void totp_cli_command_automation_docopt_arguments() {
    TOTP_CLI_PRINTF(
        "  " TOTP_CLI_COMMAND_AUTOMATION_ARG_METHOD
        "    Automation method to be set. Must be one of: " TOTP_CLI_COMMAND_AUTOMATION_METHOD_NONE
        ", " TOTP_CLI_COMMAND_AUTOMATION_METHOD_USB
#ifdef TOTP_BADBT_TYPE_ENABLED
        ", " TOTP_CLI_COMMAND_AUTOMATION_METHOD_BT
#endif
        "\r\n");
}

static void totp_cli_command_automation_print_method(AutomationMethod method, char* color) {
#ifdef TOTP_BADBT_TYPE_ENABLED
    bool has_previous_method = false;
#endif
    if(method & AutomationMethodBadUsb) {
        TOTP_CLI_PRINTF_COLORFUL(color, "\"" TOTP_CLI_COMMAND_AUTOMATION_METHOD_USB "\"");
#ifdef TOTP_BADBT_TYPE_ENABLED
        has_previous_method = true;
#endif
    }

#ifdef TOTP_BADBT_TYPE_ENABLED
    if(method & AutomationMethodBadBt) {
        if(has_previous_method) {
            TOTP_CLI_PRINTF_COLORFUL(color, " and ");
        }

        TOTP_CLI_PRINTF_COLORFUL(color, "\"" TOTP_CLI_COMMAND_AUTOMATION_METHOD_BT "\"");
    }
#endif

    if(method == AutomationMethodNone) {
        TOTP_CLI_PRINTF_COLORFUL(color, "\"" TOTP_CLI_COMMAND_AUTOMATION_METHOD_NONE "\"");
    }
}

void totp_cli_command_automation_handle(PluginState* plugin_state, FuriString* args, Cli* cli) {
    if(!totp_cli_ensure_authenticated(plugin_state, cli)) {
        return;
    }

    FuriString* temp_str = furi_string_alloc();
    bool new_method_provided = false;
    AutomationMethod new_method = AutomationMethodNone;
    bool args_valid = true;
    while(args_read_string_and_trim(args, temp_str)) {
        if(furi_string_cmpi_str(temp_str, TOTP_CLI_COMMAND_AUTOMATION_METHOD_NONE) == 0) {
            new_method_provided = true;
            new_method = AutomationMethodNone;
        } else if(furi_string_cmpi_str(temp_str, TOTP_CLI_COMMAND_AUTOMATION_METHOD_USB) == 0) {
            new_method_provided = true;
            new_method |= AutomationMethodBadUsb;
        }
#ifdef TOTP_BADBT_TYPE_ENABLED
        else if(furi_string_cmpi_str(temp_str, TOTP_CLI_COMMAND_AUTOMATION_METHOD_BT) == 0) {
            new_method_provided = true;
            new_method |= AutomationMethodBadBt;
        }
#endif
        else {
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

            plugin_state->automation_method = new_method;
            if(totp_config_file_update_automation_method(new_method) ==
               TotpConfigFileUpdateSuccess) {
                TOTP_CLI_PRINTF_SUCCESS("Automation method is set to ");
                totp_cli_command_automation_print_method(new_method, TOTP_CLI_COLOR_SUCCESS);
                cli_nl();
            } else {
                TOTP_CLI_PRINT_ERROR_UPDATING_CONFIG_FILE();
            }

#ifdef TOTP_BADBT_TYPE_ENABLED
            if(!(new_method & AutomationMethodBadBt) &&
               plugin_state->bt_type_code_worker_context != NULL) {
                totp_bt_type_code_worker_free(plugin_state->bt_type_code_worker_context);
                plugin_state->bt_type_code_worker_context = NULL;
            }
#endif

            if(previous_scene != TotpSceneNone) {
                totp_scene_director_activate_scene(plugin_state, previous_scene, NULL);
            }
        } else {
            TOTP_CLI_PRINTF_INFO("Current automation method is ");
            totp_cli_command_automation_print_method(
                plugin_state->automation_method, TOTP_CLI_COLOR_INFO);
            cli_nl();
        }
    } while(false);

    furi_string_free(temp_str);
}