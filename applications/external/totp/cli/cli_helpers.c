#include "cli_helpers.h"
#include <cli/cli.h>
#include <lib/toolbox/args.h>
#include "../types/plugin_event.h"

const char* TOTP_CLI_COLOR_ERROR = "91m";
const char* TOTP_CLI_COLOR_WARNING = "93m";
const char* TOTP_CLI_COLOR_SUCCESS = "92m";
const char* TOTP_CLI_COLOR_INFO = "96m";

bool totp_cli_ensure_authenticated(const PluginState* plugin_state, Cli* cli) {
    if(plugin_state->current_scene == TotpSceneAuthentication) {
        TOTP_CLI_PRINTF("Pleases enter PIN on your flipper device\r\n");

        while((plugin_state->current_scene == TotpSceneAuthentication ||
               plugin_state->current_scene == TotpSceneNone) &&
              !cli_cmd_interrupt_received(cli)) {
            furi_delay_ms(100);
        }

        totp_cli_delete_last_line();

        if(plugin_state->current_scene == TotpSceneAuthentication || //-V560
           plugin_state->current_scene == TotpSceneNone) { //-V560
            TOTP_CLI_PRINTF_INFO("Cancelled by user\r\n");
            return false;
        }
    }

    return true;
}

void totp_cli_force_close_app(FuriMessageQueue* event_queue) {
    PluginEvent event = {.type = EventForceCloseApp};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

bool totp_cli_read_line(Cli* cli, FuriString* out_str, bool mask_user_input) {
    uint8_t c;
    while(cli_read(cli, &c, 1) == 1) {
        if(c == CliSymbolAsciiEsc) {
            // Some keys generating escape-sequences
            // We need to ignore them as we care about alpha-numerics only
            uint8_t c2;
            cli_read_timeout(cli, &c2, 1, 0);
            cli_read_timeout(cli, &c2, 1, 0);
        } else if(c == CliSymbolAsciiETX) {
            cli_nl();
            return false;
        } else if(
            (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            c == '/' || c == '=' || c == '+') {
            if(mask_user_input) {
                putc('*', stdout);
            } else {
                putc(c, stdout);
            }
            fflush(stdout);
            furi_string_push_back(out_str, c);
        } else if(c == CliSymbolAsciiBackspace || c == CliSymbolAsciiDel) {
            size_t out_str_size = furi_string_size(out_str);
            if(out_str_size > 0) {
                totp_cli_delete_last_char();
                furi_string_left(out_str, out_str_size - 1);
            }
        } else if(c == CliSymbolAsciiCR) {
            cli_nl();
            break;
        }
    }

    return true;
}

bool args_read_uint8_and_trim(FuriString* args, uint8_t* value) {
    int int_value;
    if(!args_read_int_and_trim(args, &int_value) || int_value < 0 || int_value > UINT8_MAX) {
        return false;
    }

    *value = (uint8_t)int_value;
    return true;
}

void furi_string_secure_free(FuriString* str) {
    for(long i = furi_string_size(str) - 1; i >= 0; i--) {
        furi_string_set_char(str, i, '\0');
    }

    furi_string_free(str);
}

void totp_cli_print_invalid_arguments() {
    TOTP_CLI_PRINTF_ERROR(
        "Invalid command arguments. use \"help\" command to get list of available commands");
}

void totp_cli_print_error_updating_config_file() {
    TOTP_CLI_PRINTF_ERROR("An error has occurred during updating config file\r\n");
}

void totp_cli_print_error_loading_token_info() {
    TOTP_CLI_PRINTF_ERROR("An error has occurred during loading token information\r\n");
}

void totp_cli_print_processing() {
    TOTP_CLI_PRINTF("Processing, please wait...\r\n");
}

void totp_cli_delete_last_char() {
    TOTP_CLI_PRINTF("\b \b");
    fflush(stdout);
}

void totp_cli_delete_current_line() {
    TOTP_CLI_PRINTF("\33[2K\r");
    fflush(stdout);
}

void totp_cli_delete_last_line() {
    TOTP_CLI_PRINTF("\033[A\33[2K\r");
    fflush(stdout);
}
