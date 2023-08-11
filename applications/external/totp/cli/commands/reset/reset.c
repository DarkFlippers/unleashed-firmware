#include "reset.h"

#include <stdlib.h>
#include <furi/core/string.h>
#include "../../cli_helpers.h"
#include "../../../ui/scene_director.h"
#include "../../../services/config/config.h"

#define TOTP_CLI_RESET_CONFIRMATION_KEYWORD "YES"

#ifdef TOTP_CLI_RICH_HELP_ENABLED
void totp_cli_command_reset_docopt_commands() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_RESET
                    "            Reset application to default settings\r\n");
}

void totp_cli_command_reset_docopt_usage() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_NAME " " TOTP_CLI_COMMAND_RESET "\r\n");
}
#endif

void totp_cli_command_reset_handle(PluginState* plugin_state, Cli* cli) {
    TOTP_CLI_LOCK_UI(plugin_state);
    TOTP_CLI_PRINTF_WARNING(
        "As a result of reset all the settings and tokens will be permanently lost.\r\n");
    TOTP_CLI_PRINTF_WARNING("Do you really want to reset application?\r\n");
    TOTP_CLI_PRINTF_WARNING("Type \"" TOTP_CLI_RESET_CONFIRMATION_KEYWORD
                            "\" and hit <ENTER> to confirm:\r\n");
    FuriString* temp_str = furi_string_alloc();
    bool is_confirmed = totp_cli_read_line(cli, temp_str, false) &&
                        furi_string_cmpi_str(temp_str, TOTP_CLI_RESET_CONFIRMATION_KEYWORD) == 0;
    furi_string_free(temp_str);
    if(is_confirmed) {
        totp_config_file_reset(plugin_state);
        TOTP_CLI_PRINTF_SUCCESS("Application has been successfully reset to default.\r\n");
        TOTP_CLI_PRINTF_SUCCESS("Now application will be closed to apply all the changes.\r\n");
        totp_cli_force_close_app(plugin_state->event_queue);
    } else {
        TOTP_CLI_PRINTF_INFO("Action was not confirmed by user\r\n");
        TOTP_CLI_UNLOCK_UI(plugin_state);
    }
}