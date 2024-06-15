#include <cli/cli_i.h>

static void onewire_cli_wrapper(Cli* cli, FuriString* args, void* context) {
    cli_plugin_wrapper("onewire", cli, args, context);
}

void onewire_on_system_start(void) {
    Cli* cli = furi_record_open(RECORD_CLI);
    cli_add_command(cli, "onewire", CliCommandFlagDefault, onewire_cli_wrapper, cli);
    furi_record_close(RECORD_CLI);
}
