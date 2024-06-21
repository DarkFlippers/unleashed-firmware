#include <cli/cli_i.h>

static void infrared_cli_start_ir_wrapper(Cli* cli, FuriString* args, void* context) {
    cli_plugin_wrapper("infrared", cli, args, context);
}

void infrared_on_system_start(void) {
    Cli* cli = (Cli*)furi_record_open(RECORD_CLI);
    cli_add_command(cli, "ir", CliCommandFlagDefault, infrared_cli_start_ir_wrapper, NULL);
    furi_record_close(RECORD_CLI);
}
