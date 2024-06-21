#include <furi.h>
#include <furi_hal.h>

#include <cli/cli.h>
#include <toolbox/args.h>

#include <one_wire/one_wire_host.h>

static void onewire_cli_print_usage(void) {
    printf("Usage:\r\n");
    printf("onewire search\r\n");
}

static void onewire_cli_search(Cli* cli) {
    UNUSED(cli);
    OneWireHost* onewire = onewire_host_alloc(&gpio_ibutton);
    uint8_t address[8];
    bool done = false;

    printf("Search started\r\n");

    onewire_host_start(onewire);
    furi_hal_power_enable_otg();

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

    furi_hal_power_disable_otg();
    onewire_host_free(onewire);
}

void onewire_cli(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);
    FuriString* cmd;
    cmd = furi_string_alloc();

    if(!args_read_string_and_trim(args, cmd)) {
        furi_string_free(cmd);
        onewire_cli_print_usage();
        return;
    }

    if(furi_string_cmp_str(cmd, "search") == 0) {
        onewire_cli_search(cli);
    }

    furi_string_free(cmd);
}

#include <flipper_application/flipper_application.h>
#include <cli/cli_i.h>

static const FlipperAppPluginDescriptor plugin_descriptor = {
    .appid = CLI_PLUGIN_APP_ID,
    .ep_api_version = CLI_PLUGIN_API_VERSION,
    .entry_point = &onewire_cli,
};

const FlipperAppPluginDescriptor* onewire_cli_plugin_ep(void) {
    return &plugin_descriptor;
}
