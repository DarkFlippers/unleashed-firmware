#include "automation.h"
#include <lib/toolbox/args.h>
#include "../../../services/config/config.h"
#include "../../../ui/scene_director.h"
#include "../../cli_helpers.h"

#define TOTP_CLI_COMMAND_AUTOMATION_ARG_METHOD "automation"
#define TOTP_CLI_COMMAND_AUTOMATION_METHOD_NONE "none"
#define TOTP_CLI_COMMAND_AUTOMATION_METHOD_USB "usb"
#ifdef TOTP_BADBT_AUTOMATION_ENABLED
#define TOTP_CLI_COMMAND_AUTOMATION_METHOD_BT "bt"
#endif
#define TOTP_CLI_COMMAND_AUTOMATION_LAYOUT_QWERTY "QWERTY"
#define TOTP_CLI_COMMAND_AUTOMATION_LAYOUT_AZERTY "AZERTY"
#define TOTP_CLI_COMMAND_AUTOMATION_ARG_KB_LAYOUT_PREFIX "-k"
#define TOTP_CLI_COMMAND_AUTOMATION_ARG_KB_LAYOUT "layout"

void totp_cli_command_automation_docopt_commands() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_AUTOMATION "       Get or set automation settings\r\n");
}

void totp_cli_command_automation_docopt_usage() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_NAME " " TOTP_CLI_COMMAND_AUTOMATION " " DOCOPT_OPTIONAL(DOCOPT_OPTION(
        TOTP_CLI_COMMAND_AUTOMATION_ARG_KB_LAYOUT_PREFIX,
        DOCOPT_ARGUMENT(
            TOTP_CLI_COMMAND_AUTOMATION_ARG_KB_LAYOUT))) " " DOCOPT_OPTIONAL(DOCOPT_MULTIPLE(DOCOPT_ARGUMENT(TOTP_CLI_COMMAND_AUTOMATION_ARG_METHOD))) "\r\n");
}

void totp_cli_command_automation_docopt_arguments() {
    TOTP_CLI_PRINTF(
        "  " TOTP_CLI_COMMAND_AUTOMATION_ARG_METHOD
        "    Automation method to be set. Must be one of: " TOTP_CLI_COMMAND_AUTOMATION_METHOD_NONE
        ", " TOTP_CLI_COMMAND_AUTOMATION_METHOD_USB
#ifdef TOTP_BADBT_AUTOMATION_ENABLED
        ", " TOTP_CLI_COMMAND_AUTOMATION_METHOD_BT
#endif
        "\r\n");
}

void totp_cli_command_automation_docopt_options() {
    TOTP_CLI_PRINTF("  " DOCOPT_OPTION(
        TOTP_CLI_COMMAND_AUTOMATION_ARG_KB_LAYOUT_PREFIX,
        DOCOPT_ARGUMENT(
            TOTP_CLI_COMMAND_AUTOMATION_ARG_KB_LAYOUT)) "    Automation keyboard layout. Must be one of: " TOTP_CLI_COMMAND_AUTOMATION_LAYOUT_QWERTY
                                                        ", " TOTP_CLI_COMMAND_AUTOMATION_LAYOUT_AZERTY
                                                        "\r\n");
}

static void print_method(AutomationMethod method, const char* color) {
#ifdef TOTP_BADBT_AUTOMATION_ENABLED
    bool has_previous_method = false;
#endif
    if(method & AutomationMethodBadUsb) {
        TOTP_CLI_PRINTF_COLORFUL(color, "\"" TOTP_CLI_COMMAND_AUTOMATION_METHOD_USB "\"");
#ifdef TOTP_BADBT_AUTOMATION_ENABLED
        has_previous_method = true;
#endif
    }

#ifdef TOTP_BADBT_AUTOMATION_ENABLED
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

static void print_kb_layout(AutomationKeyboardLayout layout, const char* color) {
    char* layoutToPrint;
    switch(layout) {
    case AutomationKeyboardLayoutQWERTY:
        layoutToPrint = TOTP_CLI_COMMAND_AUTOMATION_LAYOUT_QWERTY;
        break;
    case AutomationKeyboardLayoutAZERTY:
        layoutToPrint = TOTP_CLI_COMMAND_AUTOMATION_LAYOUT_AZERTY;
        break;
    default:
        furi_crash("Unknown automation keyboard layout");
        break;
    }

    TOTP_CLI_PRINTF_COLORFUL(color, "%s", layoutToPrint);
}

static bool
    parse_automation_keyboard_layout(const FuriString* str, AutomationKeyboardLayout* out) {
    bool result = true;
    if(furi_string_cmpi_str(str, TOTP_CLI_COMMAND_AUTOMATION_LAYOUT_QWERTY) == 0) {
        *out = AutomationKeyboardLayoutQWERTY;
    } else if(furi_string_cmpi_str(str, TOTP_CLI_COMMAND_AUTOMATION_LAYOUT_AZERTY) == 0) {
        *out = AutomationKeyboardLayoutAZERTY;
    } else {
        result = false;
    }

    return result;
}

void totp_cli_command_automation_handle(PluginState* plugin_state, FuriString* args, Cli* cli) {
    if(!totp_cli_ensure_authenticated(plugin_state, cli)) {
        return;
    }

    FuriString* temp_str = furi_string_alloc();
    bool new_method_provided = false;
    AutomationMethod new_method = AutomationMethodNone;
    AutomationKeyboardLayout new_kb_layout = plugin_state->automation_kb_layout;
    bool args_valid = true;
    while(args_read_string_and_trim(args, temp_str)) {
        if(furi_string_cmpi_str(temp_str, TOTP_CLI_COMMAND_AUTOMATION_METHOD_NONE) == 0) {
            new_method_provided = true;
            new_method = AutomationMethodNone;
        } else if(furi_string_cmpi_str(temp_str, TOTP_CLI_COMMAND_AUTOMATION_METHOD_USB) == 0) {
            new_method_provided = true;
            new_method |= AutomationMethodBadUsb;
        }
#ifdef TOTP_BADBT_AUTOMATION_ENABLED
        else if(furi_string_cmpi_str(temp_str, TOTP_CLI_COMMAND_AUTOMATION_METHOD_BT) == 0) {
            new_method_provided = true;
            new_method |= AutomationMethodBadBt;
        }
#endif
        else if(furi_string_cmpi_str(temp_str, TOTP_CLI_COMMAND_AUTOMATION_ARG_KB_LAYOUT_PREFIX) == 0) {
            if(!args_read_string_and_trim(args, temp_str) ||
               !parse_automation_keyboard_layout(temp_str, &new_kb_layout)) {
                args_valid = false;
                break;
            }
        } else {
            args_valid = false;
            break;
        }
    }

    do {
        if(!args_valid) {
            totp_cli_print_invalid_arguments();
            break;
        }

        if(new_method_provided) {
            TOTP_CLI_LOCK_UI(plugin_state);

            plugin_state->automation_method = new_method;
            plugin_state->automation_kb_layout = new_kb_layout;
            if(totp_config_file_update_automation_method(plugin_state)) {
                TOTP_CLI_PRINTF_SUCCESS("Automation method is set to ");
                print_method(new_method, TOTP_CLI_COLOR_SUCCESS);
                TOTP_CLI_PRINTF_SUCCESS(" (");
                print_kb_layout(plugin_state->automation_kb_layout, TOTP_CLI_COLOR_SUCCESS);
                TOTP_CLI_PRINTF_SUCCESS(")");
                cli_nl();
            } else {
                totp_cli_print_error_updating_config_file();
            }

#ifdef TOTP_BADBT_AUTOMATION_ENABLED
            if(!(new_method & AutomationMethodBadBt) &&
               plugin_state->bt_type_code_worker_context != NULL) {
                totp_bt_type_code_worker_free(plugin_state->bt_type_code_worker_context);
                plugin_state->bt_type_code_worker_context = NULL;
            }
#endif

            TOTP_CLI_UNLOCK_UI(plugin_state);
        } else {
            TOTP_CLI_PRINTF_INFO("Current automation method is ");
            print_method(plugin_state->automation_method, TOTP_CLI_COLOR_INFO);
            TOTP_CLI_PRINTF_INFO(" (");
            print_kb_layout(plugin_state->automation_kb_layout, TOTP_CLI_COLOR_INFO);
            TOTP_CLI_PRINTF_INFO(")");
            cli_nl();
        }
    } while(false);

    furi_string_free(temp_str);
}