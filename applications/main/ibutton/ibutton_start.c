#include <cli/cli_i.h>

static void ibutton_cli_wrapper(Cli* cli, FuriString* args, void* context) {
    cli_plugin_wrapper("ibutton", cli, args, context);
}

void ibutton_on_system_start(void) {
    Cli* cli = furi_record_open(RECORD_CLI);
    cli_add_command(cli, "ikey", CliCommandFlagDefault, ibutton_cli_wrapper, cli);
    furi_record_close(RECORD_CLI);
}
