#include <cli/cli_i.h>

static void nfc_cli_wrapper(Cli* cli, FuriString* args, void* context) {
    cli_plugin_wrapper("nfc", cli, args, context);
}

void nfc_on_system_start(void) {
    Cli* cli = furi_record_open(RECORD_CLI);
    cli_add_command(cli, "nfc", CliCommandFlagDefault, nfc_cli_wrapper, NULL);
    furi_record_close(RECORD_CLI);
}
