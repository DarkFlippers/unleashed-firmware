#include "power_cli.h"

#include <power/power_service/power.h>
#include <cli/cli.h>
#include <furi-hal.h>

void power_cli_poweroff(Cli* cli, string_t args, void* context) {
    power_off();
    printf("It's now safe to disconnect USB from your flipper\r\n");
    while(cli_getc(cli)) {
    }
}

void power_cli_reboot(Cli* cli, string_t args, void* context) {
    power_reboot(PowerBootModeNormal);
}

void power_cli_dfu(Cli* cli, string_t args, void* context) {
    power_reboot(PowerBootModeDfu);
}

void power_cli_factory_reset(Cli* cli, string_t args, void* context) {
    printf("All data will be lost. Are you sure (y/n)?\r\n");
    char c = cli_getc(cli);
    if(c == 'y' || c == 'Y') {
        printf("Data will be wiped after reboot.\r\n");
        furi_hal_boot_set_flags(FuriHalBootFlagFactoryReset);
        power_reboot(PowerBootModeNormal);
    } else {
        printf("Safe choice.\r\n");
    }
}

void power_cli_info(Cli* cli, string_t args, void* context) {
    furi_hal_power_dump_state();
}

void power_cli_otg(Cli* cli, string_t args, void* context) {
    if(!string_cmp(args, "0")) {
        furi_hal_power_disable_otg();
    } else if(!string_cmp(args, "1")) {
        furi_hal_power_enable_otg();
    } else {
        cli_print_usage("power_otg", "<1|0>", string_get_cstr(args));
    }
}

void power_cli_ext(Cli* cli, string_t args, void* context) {
    if(!string_cmp(args, "0")) {
        furi_hal_power_disable_external_3_3v();
    } else if(!string_cmp(args, "1")) {
        furi_hal_power_enable_external_3_3v();
    } else {
        cli_print_usage("power_ext", "<1|0>", string_get_cstr(args));
    }
}

void power_cli_init() {
    Cli* cli = furi_record_open("cli");

    cli_add_command(cli, "poweroff", CliCommandFlagParallelSafe, power_cli_poweroff, NULL);
    cli_add_command(cli, "reboot", CliCommandFlagParallelSafe, power_cli_reboot, NULL);
    cli_add_command(
        cli, "factory_reset", CliCommandFlagParallelSafe, power_cli_factory_reset, NULL);
    cli_add_command(cli, "dfu", CliCommandFlagParallelSafe, power_cli_dfu, NULL);
    cli_add_command(cli, "power_info", CliCommandFlagParallelSafe, power_cli_info, NULL);
    cli_add_command(cli, "power_otg", CliCommandFlagParallelSafe, power_cli_otg, NULL);
    cli_add_command(cli, "power_ext", CliCommandFlagParallelSafe, power_cli_ext, NULL);

    furi_record_close("cli");
}
