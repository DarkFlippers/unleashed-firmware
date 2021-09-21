#include "bt_cli.h"
#include <furi.h>
#include <furi-hal.h>

void bt_cli_init() {
    Cli* cli = furi_record_open("cli");

    cli_add_command(cli, "bt_info", CliCommandFlagDefault, bt_cli_command_info, NULL);
    cli_add_command(cli, "bt_tx_carrier", CliCommandFlagDefault, bt_cli_command_carrier_tx, NULL);
    cli_add_command(cli, "bt_rx_carrier", CliCommandFlagDefault, bt_cli_command_carrier_rx, NULL);
    cli_add_command(cli, "bt_tx_pt", CliCommandFlagDefault, bt_cli_command_packet_tx, NULL);
    cli_add_command(cli, "bt_rx_pt", CliCommandFlagDefault, bt_cli_command_packet_rx, NULL);

    furi_record_close("cli");
}

void bt_cli_command_info(Cli* cli, string_t args, void* context) {
    string_t buffer;
    string_init(buffer);
    furi_hal_bt_dump_state(buffer);
    printf("%s", string_get_cstr(buffer));
    string_clear(buffer);
}

void bt_cli_command_carrier_tx(Cli* cli, string_t args, void* context) {
    uint16_t channel;
    uint16_t power;
    int ret = sscanf(string_get_cstr(args), "%hu %hu", &channel, &power);
    if(ret != 2) {
        printf("sscanf returned %d, channel: %hu, power: %hu\r\n", ret, channel, power);
        cli_print_usage("bt_tx_carrier", "<Channel number> <Power>", string_get_cstr(args));
        return;
    }
    if(channel > 39) {
        printf("Channel number must be in 0...39 range, not %hu\r\n", channel);
        return;
    }
    if(power > 6) {
        printf("Power must be in 0...6 dB range, not %hu\r\n", power);
        return;
    }
    printf("Transmitting carrier at %hu channel at %hu dB power\r\n", channel, power);
    printf("Press CTRL+C to stop\r\n");
    furi_hal_bt_start_tone_tx(channel, 0x19 + power);

    while(!cli_cmd_interrupt_received(cli)) {
        osDelay(250);
    }
    furi_hal_bt_stop_tone_tx();
}

void bt_cli_command_carrier_rx(Cli* cli, string_t args, void* context) {
    uint16_t channel;
    int ret = sscanf(string_get_cstr(args), "%hu", &channel);
    if(ret != 1) {
        printf("sscanf returned %d, channel: %hu\r\n", ret, channel);
        cli_print_usage("bt_rx_carrier", "<Channel number>", string_get_cstr(args));
        return;
    }
    if(channel > 39) {
        printf("Channel number must be in 0...39 range, not %hu\r\n", channel);
        return;
    }
    printf("Receiving carrier at %hu channel\r\n", channel);
    printf("Press CTRL+C to stop\r\n");

    furi_hal_bt_start_packet_rx(channel, 1);

    while(!cli_cmd_interrupt_received(cli)) {
        osDelay(1024 / 4);
        printf("RSSI: %6.1f dB\r", furi_hal_bt_get_rssi());
        fflush(stdout);
    }

    furi_hal_bt_stop_packet_test();
}

void bt_cli_command_packet_tx(Cli* cli, string_t args, void* context) {
    uint16_t channel;
    uint16_t pattern;
    uint16_t datarate;
    int ret = sscanf(string_get_cstr(args), "%hu %hu %hu", &channel, &pattern, &datarate);
    if(ret != 3) {
        printf("sscanf returned %d, channel: %hu %hu %hu\r\n", ret, channel, pattern, datarate);
        cli_print_usage(
            "bt_tx_pt", "<Channel number> <Pattern> <Datarate>", string_get_cstr(args));
        return;
    }
    if(channel > 39) {
        printf("Channel number must be in 0...39 range, not %hu\r\n", channel);
        return;
    }
    if(pattern > 5) {
        printf("Pattern must be in 0...5 range, not %hu\r\n", pattern);
        printf("0 - Pseudo-Random bit sequence 9\r\n");
        printf("1 - Pattern of alternating bits '11110000'\r\n");
        printf("2 - Pattern of alternating bits '10101010'\r\n");
        printf("3 - Pseudo-Random bit sequence 15\r\n");
        printf("4 - Pattern of All '1' bits\r\n");
        printf("5 - Pattern of All '0' bits\r\n");
        return;
    }
    if(datarate < 1 || datarate > 2) {
        printf("Datarate must be in 1 or 2 Mb, not %hu\r\n", datarate);
        return;
    }

    printf(
        "Transmitting %hu pattern packet at %hu channel at %hu M datarate\r\n",
        pattern,
        channel,
        datarate);
    printf("Press CTRL+C to stop\r\n");
    furi_hal_bt_start_packet_tx(channel, pattern, datarate);

    while(!cli_cmd_interrupt_received(cli)) {
        osDelay(250);
    }
    furi_hal_bt_stop_packet_test();
    printf("Transmitted %lu packets", furi_hal_bt_get_transmitted_packets());
}

void bt_cli_command_packet_rx(Cli* cli, string_t args, void* context) {
    uint16_t channel;
    uint16_t datarate;
    int ret = sscanf(string_get_cstr(args), "%hu %hu", &channel, &datarate);
    if(ret != 2) {
        printf("sscanf returned %d, channel: %hu datarate: %hu\r\n", ret, channel, datarate);
        cli_print_usage("bt_rx_pt", "<Channel number> <Datarate>", string_get_cstr(args));
        return;
    }
    if(channel > 39) {
        printf("Channel number must be in 0...39 range, not %hu\r\n", channel);
        return;
    }
    if(datarate < 1 || datarate > 2) {
        printf("Datarate must be in 1 or 2 Mb, not %hu\r\n", datarate);
        return;
    }
    printf("Receiving packets at %hu channel at %hu M datarate\r\n", channel, datarate);
    printf("Press CTRL+C to stop\r\n");
    furi_hal_bt_start_packet_rx(channel, datarate);

    float rssi_raw = 0;
    while(!cli_cmd_interrupt_received(cli)) {
        osDelay(250);
        rssi_raw = furi_hal_bt_get_rssi();
        printf("RSSI: %03.1f dB\r", rssi_raw);
        fflush(stdout);
    }
    uint16_t packets_received = furi_hal_bt_stop_packet_test();
    printf("Received %hu packets", packets_received);
}
