#include "power_cli.h"
#include <api-hal.h>

void power_cli_poweroff(Cli* cli, string_t args, void* context) {
    Power* power = context;
    power_off(power);
}

void power_cli_reboot(Cli* cli, string_t args, void* context) {
    Power* power = context;
    power_reboot(power, PowerBootModeNormal);
}

void power_cli_dfu(Cli* cli, string_t args, void* context) {
    Power* power = context;
    power_reboot(power, PowerBootModeDfu);
}

void power_cli_factory_reset(Cli* cli, string_t args, void* context) {
    Power* power = context;
    printf("All data will be lost. Are you sure (y/n)?\r\n");
    char c = cli_getc(cli);
    if(c == 'y' || c == 'Y') {
        printf("Data will be wiped after reboot.\r\n");
        api_hal_boot_set_flags(ApiHalBootFlagFactoryReset);
        power_reboot(power, PowerBootModeNormal);
    } else {
        printf("Safe choice.\r\n");
    }
}

void power_cli_info(Cli* cli, string_t args, void* context) {
    api_hal_power_dump_state();
}

void power_cli_otg(Cli* cli, string_t args, void* context) {
    if(!string_cmp(args, "0")) {
        api_hal_power_disable_otg();
    } else if(!string_cmp(args, "1")) {
        api_hal_power_enable_otg();
    } else {
        cli_print_usage("power_otg", "<1|0>", string_get_cstr(args));
    }
}

void power_cli_init(Cli* cli, Power* power) {
    cli_add_command(cli, "poweroff", power_cli_poweroff, power);
    cli_add_command(cli, "reboot", power_cli_reboot, power);
    cli_add_command(cli, "factory_reset", power_cli_factory_reset, power);
    cli_add_command(cli, "dfu", power_cli_dfu, power);
    cli_add_command(cli, "power_info", power_cli_info, power);
    cli_add_command(cli, "power_otg", power_cli_otg, power);
}
