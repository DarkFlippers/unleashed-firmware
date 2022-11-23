#include "cli_helpers.h"
#include <cli/cli.h>

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