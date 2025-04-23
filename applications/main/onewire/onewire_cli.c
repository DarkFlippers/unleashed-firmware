#include <furi.h>
#include <furi_hal.h>

#include <cli/cli_main_commands.h>
#include <power/power_service/power.h>
#include <toolbox/cli/cli_command.h>
#include <toolbox/args.h>

#include <one_wire/one_wire_host.h>

static void onewire_cli_print_usage(void) {
    printf("Usage:\r\n");
    printf("onewire search\r\n");
}

static void onewire_cli_search(PipeSide* pipe) {
    UNUSED(pipe);
    OneWireHost* onewire = onewire_host_alloc(&gpio_ibutton);
    Power* power = furi_record_open(RECORD_POWER);
    uint8_t address[8];
    bool done = false;

    printf("Search started\r\n");

    onewire_host_start(onewire);
    power_enable_otg(power, true);

    while(!done) {
        if(onewire_host_search(onewire, address, OneWireHostSearchModeNormal) != 1) {
            printf("Search finished\r\n");
            onewire_host_reset_search(onewire);
            done = true;
        } else {
            printf("Found: ");
            for(uint8_t i = 0; i < 8; i++) {
                printf("%02X", address[i]);
            }
            printf("\r\n");
        }
        furi_delay_ms(100);
    }

    power_enable_otg(power, false);

    onewire_host_free(onewire);
    furi_record_close(RECORD_POWER);
}

static void execute(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(context);
    FuriString* cmd;
    cmd = furi_string_alloc();

    if(!args_read_string_and_trim(args, cmd)) {
        furi_string_free(cmd);
        onewire_cli_print_usage();
        return;
    }

    if(furi_string_cmp_str(cmd, "search") == 0) {
        onewire_cli_search(pipe);
    }

    furi_string_free(cmd);
}

CLI_COMMAND_INTERFACE(onewire, execute, CliCommandFlagDefault, 1024, CLI_APPID);
