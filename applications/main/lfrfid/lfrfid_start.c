#include <cli/cli_i.h>

static void lfrfid_cli_wrapper(Cli* cli, FuriString* args, void* context) {
    cli_plugin_wrapper("lfrfid", cli, args, context);
}

void lfrfid_on_system_start(void) {
    Cli* cli = furi_record_open(RECORD_CLI);
    cli_add_command(cli, "rfid", CliCommandFlagDefault, lfrfid_cli_wrapper, NULL);
    furi_record_close(RECORD_CLI);
}
