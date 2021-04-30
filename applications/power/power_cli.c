#include "power_cli.h"
#include <api-hal.h>

void power_cli_poweroff(string_t args, void* context) {
    Power* power = context;
    power_off(power);
}

void power_cli_reset(string_t args, void* context) {
    Power* power = context;
    power_reset(power, PowerBootModeNormal);
}

void power_cli_dfu(string_t args, void* context) {
    Power* power = context;
    power_reset(power, PowerBootModeDfu);
}

void power_cli_test(string_t args, void* context) {
    api_hal_power_dump_state();
}

void power_cli_otg_on(string_t args, void* context) {
    api_hal_power_enable_otg();
}

void power_cli_otg_off(string_t args, void* context) {
    api_hal_power_disable_otg();
}

void power_cli_init(Cli* cli, Power* power) {
    cli_add_command(cli, "poweroff", power_cli_poweroff, power);
    cli_add_command(cli, "reset", power_cli_reset, power);
    cli_add_command(cli, "dfu", power_cli_dfu, power);
    cli_add_command(cli, "power_test", power_cli_test, power);
    cli_add_command(cli, "power_otg_on", power_cli_otg_on, power);
    cli_add_command(cli, "power_otg_off", power_cli_otg_off, power);
}
