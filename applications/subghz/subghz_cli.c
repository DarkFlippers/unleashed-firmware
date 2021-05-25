#include "subghz_cli.h"
#include <furi.h>
#include <api-hal.h>

void subghz_cli_init() {
    Cli* cli = furi_record_open("cli");

    cli_add_command(cli, "subghz_tx_carrier", subghz_cli_command_tx_carrier, NULL);
    cli_add_command(cli, "subghz_rx_carrier", subghz_cli_command_rx_carrier, NULL);
    cli_add_command(cli, "subghz_tx_pt", subghz_cli_command_tx_pt, NULL);
    cli_add_command(cli, "subghz_rx_pt", subghz_cli_command_rx_pt, NULL);

    furi_record_close("cli");
}

void subghz_cli_command_tx_carrier(Cli* cli, string_t args, void* context) {
    uint32_t frequency;
    int ret = sscanf(string_get_cstr(args), "%lu", &frequency);
    if(ret != 1) {
        printf("sscanf returned %d, frequency: %lu\r\n", ret, frequency);
        cli_print_usage("subghz_tx_carrier", "<Frequency in HZ>", string_get_cstr(args));
        return;
    }

    if(frequency < 300000000 || frequency > 925000000) {
        printf("Frequency must be in 300000000...925000000 range, not %lu\r\n", frequency);
        return;
    }

    api_hal_subghz_reset();
    api_hal_subghz_load_preset(ApiHalSubGhzPresetOokAsync);
    frequency = api_hal_subghz_set_frequency(frequency);
    printf("Transmitting at frequency %lu Hz\r\n", frequency);
    printf("Press CTRL+C to stop\r\n");
    if(frequency < 400) {
        api_hal_subghz_set_path(ApiHalSubGhzPath315);
    } else if(frequency < 500) {
        api_hal_subghz_set_path(ApiHalSubGhzPath433);
    } else {
        api_hal_subghz_set_path(ApiHalSubGhzPath868);
    }

    hal_gpio_init(&gpio_cc1101_g0, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    hal_gpio_write(&gpio_cc1101_g0, false);

    api_hal_subghz_tx();

    while(!cli_cmd_interrupt_received(cli)) {
        osDelay(250);
    }

    api_hal_subghz_reset();
    api_hal_subghz_set_path(ApiHalSubGhzPathIsolate);
    hal_gpio_init(&gpio_cc1101_g0, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
}

void subghz_cli_command_rx_carrier(Cli* cli, string_t args, void* context) {
    uint32_t frequency;
    int ret = sscanf(string_get_cstr(args), "%lu", &frequency);
    if(ret != 1) {
        printf("sscanf returned %d, frequency: %lu\r\n", ret, frequency);
        cli_print_usage("subghz_tx_carrier", "<Frequency in HZ>", string_get_cstr(args));
        return;
    }

    if(frequency < 300000000 || frequency > 925000000) {
        printf("Frequency must be in 300000000...925000000 range, not %lu\r\n", frequency);
        return;
    }

    api_hal_subghz_reset();
    api_hal_subghz_load_preset(ApiHalSubGhzPresetOokAsync);
    frequency = api_hal_subghz_set_frequency(frequency);
    printf("Receiving at frequency %lu Hz\r\n", frequency);
    printf("Press CTRL+C to stop\r\n");
    if(frequency < 400) {
        api_hal_subghz_set_path(ApiHalSubGhzPath315);
    } else if(frequency < 500) {
        api_hal_subghz_set_path(ApiHalSubGhzPath433);
    } else {
        api_hal_subghz_set_path(ApiHalSubGhzPath868);
    }

    hal_gpio_init(&gpio_cc1101_g0, GpioModeAnalog, GpioPullNo, GpioSpeedLow);

    api_hal_subghz_rx();

    while(!cli_cmd_interrupt_received(cli)) {
        osDelay(250);
        printf("RSSI: %03.1fdbm\r", api_hal_subghz_get_rssi());
        fflush(stdout);
    }

    api_hal_subghz_reset();
    api_hal_subghz_set_path(ApiHalSubGhzPathIsolate);
}

void subghz_cli_command_tx_pt(Cli* cli, string_t args, void* context) {
    printf("Not implemented\r\n");
}

void subghz_cli_command_rx_pt(Cli* cli, string_t args, void* context) {
    printf("Not implemented\r\n");
}
