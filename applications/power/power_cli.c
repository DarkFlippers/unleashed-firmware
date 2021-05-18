#include "power_cli.h"
#include <api-hal.h>

void power_cli_poweroff(Cli* cli, string_t args, void* context) {
    Power* power = context;
    power_off(power);
}

void power_cli_reset(Cli* cli, string_t args, void* context) {
    Power* power = context;
    power_reset(power, PowerBootModeNormal);
}

void power_cli_dfu(Cli* cli, string_t args, void* context) {
    Power* power = context;
    power_reset(power, PowerBootModeDfu);
}

void power_cli_factory_reset(Cli* cli, string_t args, void* context) {
    Power* power = context;
    printf("All data will be lost. Are you sure (y/n)?\r\n");
    char c = cli_getc(cli);
    if(c == 'y' || c == 'Y') {
        printf("Data will be wiped after reboot.\r\n");
        api_hal_boot_set_flags(ApiHalBootFlagFactoryReset);
        power_reset(power, PowerBootModeNormal);
    } else {
        printf("Safe choice.\r\n");
    }
}

void power_cli_test(Cli* cli, string_t args, void* context) {
    api_hal_power_dump_state();
}

void power_cli_otg_on(Cli* cli, string_t args, void* context) {
    api_hal_power_enable_otg();
}

void power_cli_otg_off(Cli* cli, string_t args, void* context) {
    api_hal_power_disable_otg();
}

void power_cli_init(Cli* cli, Power* power) {
    cli_add_command(cli, "poweroff", power_cli_poweroff, power);
    cli_add_command(cli, "reset", power_cli_reset, power);
    cli_add_command(cli, "factory_reset", power_cli_factory_reset, power);
    cli_add_command(cli, "dfu", power_cli_dfu, power);
    cli_add_command(cli, "power_test", power_cli_test, power);
    cli_add_command(cli, "power_otg_on", power_cli_otg_on, power);
    cli_add_command(cli, "power_otg_off", power_cli_otg_off, power);
}
