#include "cli_helpers.h"
#include <cli/cli.h>
#include "../types/plugin_event.h"

bool totp_cli_ensure_authenticated(const PluginState* plugin_state, Cli* cli) {
    if(plugin_state->current_scene == TotpSceneAuthentication) {
        TOTP_CLI_PRINTF("Pleases enter PIN on your flipper device\r\n");

        while(plugin_state->current_scene == TotpSceneAuthentication &&
              !cli_cmd_interrupt_received(cli)) {
            furi_delay_ms(100);
        }

        TOTP_CLI_DELETE_LAST_LINE();

        if(plugin_state->current_scene == TotpSceneAuthentication) { //-V547
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
        } else if((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
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
                TOTP_CLI_DELETE_LAST_CHAR();
                furi_string_left(out_str, out_str_size - 1);
            }
        } else if(c == CliSymbolAsciiCR) {
            cli_nl();
            break;
        }
    }

    return true;
}