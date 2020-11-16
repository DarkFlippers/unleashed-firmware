#include "cli_commands.h"
#include <api-hal.h>

void cli_command_help(string_t args, void* context) {
    (void)args;
    Cli* cli = context;
    cli_print("Commands we have:");

    furi_check(osMutexAcquire(cli->mutex, osWaitForever) == osOK);
    CliCommandDict_it_t it;
    for(CliCommandDict_it(it, cli->commands); !CliCommandDict_end_p(it); CliCommandDict_next(it)) {
        CliCommandDict_itref_t* ref = CliCommandDict_ref(it);
        cli_print(" ");
        cli_print(string_get_cstr(ref->key));
    };
    furi_check(osMutexRelease(cli->mutex) == osOK);

    if(string_size(args) > 0) {
        cli_nl();
        cli_print("Also I have no clue what '");
        cli_print(string_get_cstr(args));
        cli_print("' is.");
    }
}

void cli_command_version(string_t args, void* context) {
    (void)args;
    (void)context;
    cli_print_version();
}

void cli_command_uuid(string_t args, void* context) {
    (void)args;
    (void)context;
    size_t uid_size = api_hal_uid_size();
    const uint8_t* uid = api_hal_uid();

    string_t byte_str;
    string_init(byte_str);
    string_cat_printf(byte_str, "UID:");
    for(size_t i = 0; i < uid_size; i++) {
        uint8_t uid_byte = uid[i];
        string_cat_printf(byte_str, "%02X", uid_byte);
    }
    cli_print(string_get_cstr(byte_str));
}

void cli_commands_init(Cli* cli) {
    cli_add_command(cli, "help", cli_command_help, cli);
    cli_add_command(cli, "?", cli_command_help, cli);
    cli_add_command(cli, "version", cli_command_version, cli);
    cli_add_command(cli, "!", cli_command_version, cli);
    cli_add_command(cli, "uid", cli_command_uuid, cli);
}