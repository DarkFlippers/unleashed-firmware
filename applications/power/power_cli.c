#include "power_cli.h"

#include <furi_hal.h>
#include <cli/cli.h>
#include <lib/toolbox/args.h>
#include <power/power_service/power.h>

void power_cli_off(Cli* cli, string_t args) {
    UNUSED(cli);
    UNUSED(args);
    Power* power = furi_record_open(RECORD_POWER);
    printf("It's now safe to disconnect USB from your flipper\r\n");
    furi_delay_ms(666);
    power_off(power);
}

void power_cli_reboot(Cli* cli, string_t args) {
    UNUSED(cli);
    UNUSED(args);
    power_reboot(PowerBootModeNormal);
}

void power_cli_reboot2dfu(Cli* cli, string_t args) {
    UNUSED(cli);
    UNUSED(args);
    power_reboot(PowerBootModeDfu);
}

static void power_cli_info_callback(const char* key, const char* value, bool last, void* context) {
    UNUSED(last);
    UNUSED(context);
    printf("%-24s: %s\r\n", key, value);
}

void power_cli_info(Cli* cli, string_t args) {
    UNUSED(cli);
    UNUSED(args);
    furi_hal_power_info_get(power_cli_info_callback, NULL);
}

void power_cli_debug(Cli* cli, string_t args) {
    UNUSED(cli);
    UNUSED(args);
    furi_hal_power_dump_state();
}

void power_cli_5v(Cli* cli, string_t args) {
    UNUSED(cli);
    if(!string_cmp(args, "0")) {
        furi_hal_power_disable_otg();
    } else if(!string_cmp(args, "1")) {
        furi_hal_power_enable_otg();
    } else {
        cli_print_usage("power_otg", "<1|0>", string_get_cstr(args));
    }
}

void power_cli_3v3(Cli* cli, string_t args) {
    UNUSED(cli);
    if(!string_cmp(args, "0")) {
        furi_hal_power_disable_external_3_3v();
    } else if(!string_cmp(args, "1")) {
        furi_hal_power_enable_external_3_3v();
    } else {
        cli_print_usage("power_ext", "<1|0>", string_get_cstr(args));
    }
}

static void power_cli_command_print_usage() {
    printf("Usage:\r\n");
    printf("power <cmd> <args>\r\n");
    printf("Cmd list:\r\n");

    printf("\toff\t - shutdown power\r\n");
    printf("\treboot\t - reboot\r\n");
    printf("\treboot2dfu\t - reboot to dfu bootloader\r\n");
    printf("\tinfo\t - show power info\r\n");
    printf("\tdebug\t - show debug information\r\n");
    printf("\t5v <0 or 1>\t - enable or disable 5v ext\r\n");
    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
        printf("\t3v3 <0 or 1>\t - enable or disable 3v3 ext\r\n");
    }
}

void power_cli(Cli* cli, string_t args, void* context) {
    UNUSED(context);
    string_t cmd;
    string_init(cmd);

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            power_cli_command_print_usage();
            break;
        }

        if(string_cmp_str(cmd, "off") == 0) {
            power_cli_off(cli, args);
            break;
        }

        if(string_cmp_str(cmd, "reboot") == 0) {
            power_cli_reboot(cli, args);
            break;
        }

        if(string_cmp_str(cmd, "reboot2dfu") == 0) {
            power_cli_reboot2dfu(cli, args);
            break;
        }

        if(string_cmp_str(cmd, "info") == 0) {
            power_cli_info(cli, args);
            break;
        }

        if(string_cmp_str(cmd, "debug") == 0) {
            power_cli_debug(cli, args);
            break;
        }

        if(string_cmp_str(cmd, "5v") == 0) {
            power_cli_5v(cli, args);
            break;
        }

        if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
            if(string_cmp_str(cmd, "3v3") == 0) {
                power_cli_3v3(cli, args);
                break;
            }
        }

        power_cli_command_print_usage();
    } while(false);

    string_clear(cmd);
}

void power_on_system_start() {
#ifdef SRV_CLI
    Cli* cli = furi_record_open(RECORD_CLI);

    cli_add_command(cli, "power", CliCommandFlagParallelSafe, power_cli, NULL);

    furi_record_close(RECORD_CLI);
#else
    UNUSED(power_cli);
#endif
}
