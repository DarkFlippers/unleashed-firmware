#include "subghz_cli.h"
#include <furi.h>
#include <api-hal.h>

static const uint8_t subghz_test_packet_data[] = {
    0x30, // 48bytes to transmit
    0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
};

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
    if(frequency < 400000000) {
        api_hal_subghz_set_path(ApiHalSubGhzPath315);
    } else if(frequency < 500000000) {
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
    if(frequency < 400000000) {
        api_hal_subghz_set_path(ApiHalSubGhzPath315);
    } else if(frequency < 500000000) {
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
    uint32_t frequency;
    uint32_t pattern;
    uint32_t count;

    int ret = sscanf(string_get_cstr(args), "%lu %lu %lu", &frequency, &pattern, &count);
    if(ret != 3) {
        printf(
            "sscanf returned %d, frequency: %lu; pattern: %lu; count: %lu\r\n",
            ret,
            frequency,
            pattern,
            count);
        cli_print_usage(
            "subghz_tx_pt", "<Frequency in HZ> <Pattern> <Count>", string_get_cstr(args));
        return;
    }

    if(frequency < 300000000 || frequency > 925000000) {
        printf("Frequency must be in 300000000...925000000 range, not %lu\r\n", frequency);
        return;
    }
    if(pattern > 1) {
        printf("Pattern must be 1, not %lu\r\n", pattern);
    }

    api_hal_subghz_reset();
    api_hal_subghz_idle();

    api_hal_subghz_load_preset(ApiHalSubGhzPreset2FskPacket);

    frequency = api_hal_subghz_set_frequency(frequency);
    if(frequency < 400000000) {
        api_hal_subghz_set_path(ApiHalSubGhzPath315);
    } else if(frequency < 500000000) {
        api_hal_subghz_set_path(ApiHalSubGhzPath433);
    } else {
        api_hal_subghz_set_path(ApiHalSubGhzPath868);
    }
    hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);

    uint8_t status = api_hal_subghz_get_status();
    FURI_LOG_D("SUBGHZ CLI", "Status: %02X", status);

    while(!cli_cmd_interrupt_received(cli) && count) {
        api_hal_subghz_idle();
        api_hal_subghz_write_packet(subghz_test_packet_data, sizeof(subghz_test_packet_data));
        api_hal_subghz_tx();
        while(!hal_gpio_read(&gpio_cc1101_g0))
            ; // Wait for sync
        while(hal_gpio_read(&gpio_cc1101_g0))
            ; // Wait end of transaction
        count--;
    }

    api_hal_subghz_reset();
    api_hal_subghz_set_path(ApiHalSubGhzPathIsolate);
    hal_gpio_init(&gpio_cc1101_g0, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
}

void subghz_cli_command_rx_pt(Cli* cli, string_t args, void* context) {
    uint32_t frequency;

    int ret = sscanf(string_get_cstr(args), "%lu", &frequency);
    if(ret != 1) {
        printf("sscanf returned %d, frequency: %lu\r\n", ret, frequency);
        cli_print_usage("subghz_rx_pt", "<Frequency in HZ>", string_get_cstr(args));
        return;
    }

    if(frequency < 300000000 || frequency > 925000000) {
        printf("Frequency must be in 300000000...925000000 range, not %lu\r\n", frequency);
        return;
    }

    api_hal_subghz_reset();
    api_hal_subghz_idle();
    api_hal_subghz_load_preset(ApiHalSubGhzPreset2FskPacket);

    frequency = api_hal_subghz_set_frequency(frequency);
    if(frequency < 400000000) {
        api_hal_subghz_set_path(ApiHalSubGhzPath315);
    } else if(frequency < 500000000) {
        api_hal_subghz_set_path(ApiHalSubGhzPath433);
    } else {
        api_hal_subghz_set_path(ApiHalSubGhzPath868);
    }
    hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);

    uint8_t status = api_hal_subghz_get_status();
    FURI_LOG_D("SUBGHZ CLI", "Status: %02X", status);
    printf("Start receiving packets. Press CTRL+C to stop\r\n");

    api_hal_subghz_flush_rx();
    api_hal_subghz_rx();
    uint32_t packet_cnt = 0;

    while(!cli_cmd_interrupt_received(cli)) {
        if(hal_gpio_read(&gpio_cc1101_g0)) {
            while(hal_gpio_read(&gpio_cc1101_g0))
                ; // Wait reception
            packet_cnt++;
            api_hal_subghz_idle();
            api_hal_subghz_flush_rx();
            api_hal_subghz_rx();
        }
    }

    printf("Received %lu packets", packet_cnt);

    api_hal_subghz_reset();
    api_hal_subghz_set_path(ApiHalSubGhzPathIsolate);
    hal_gpio_init(&gpio_cc1101_g0, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
}
