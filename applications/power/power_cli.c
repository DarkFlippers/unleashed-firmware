#include "power_cli.h"
#include <api-hal.h>

void power_cli_poweroff(string_t args, void* context) {
    api_hal_power_off();
}

void power_cli_reset(string_t args, void* context) {
    NVIC_SystemReset();
}

void power_cli_dfu(string_t args, void* context) {
    api_hal_boot_set_mode(ApiHalBootModeDFU);
    NVIC_SystemReset();
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

void power_cli_init(Cli* cli) {
    cli_add_command(cli, "poweroff", power_cli_poweroff, NULL);
    cli_add_command(cli, "reset", power_cli_reset, NULL);
    cli_add_command(cli, "dfu", power_cli_dfu, NULL);
    cli_add_command(cli, "power_test", power_cli_test, NULL);
    cli_add_command(cli, "power_otg_on", power_cli_otg_on, NULL);
    cli_add_command(cli, "power_otg_off", power_cli_otg_off, NULL);
}
