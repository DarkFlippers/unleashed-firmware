#include <furi.h>
#include <furi_hal.h>
#include <cli/cli.h>
#include <lib/toolbox/args.h>

#include <ble/ble.h>
#include "bt_settings.h"
#include "bt_service/bt.h"

static void bt_cli_command_hci_info(Cli* cli, string_t args, void* context) {
    UNUSED(cli);
    UNUSED(args);
    UNUSED(context);
    string_t buffer;
    string_init(buffer);
    furi_hal_bt_dump_state(buffer);
    printf("%s", string_get_cstr(buffer));
    string_clear(buffer);
}

static void bt_cli_command_carrier_tx(Cli* cli, string_t args, void* context) {
    UNUSED(context);
    int channel = 0;
    int power = 0;

    do {
        if(!args_read_int_and_trim(args, &channel) && (channel < 0 || channel > 39)) {
            printf("Incorrect or missing channel, expected int 0-39");
            break;
        }
        if(!args_read_int_and_trim(args, &power) && (power < 0 || power > 6)) {
            printf("Incorrect or missing power, expected int 0-6");
            break;
        }

        Bt* bt = furi_record_open(RECORD_BT);
        bt_disconnect(bt);
        furi_hal_bt_reinit();
        printf("Transmitting carrier at %d channel at %d dB power\r\n", channel, power);
        printf("Press CTRL+C to stop\r\n");
        furi_hal_bt_start_tone_tx(channel, 0x19 + power);

        while(!cli_cmd_interrupt_received(cli)) {
            furi_delay_ms(250);
        }
        furi_hal_bt_stop_tone_tx();

        bt_set_profile(bt, BtProfileSerial);
        furi_record_close(RECORD_BT);
    } while(false);
}

static void bt_cli_command_carrier_rx(Cli* cli, string_t args, void* context) {
    UNUSED(context);
    int channel = 0;

    do {
        if(!args_read_int_and_trim(args, &channel) && (channel < 0 || channel > 39)) {
            printf("Incorrect or missing channel, expected int 0-39");
            break;
        }

        Bt* bt = furi_record_open(RECORD_BT);
        bt_disconnect(bt);
        furi_hal_bt_reinit();
        printf("Receiving carrier at %d channel\r\n", channel);
        printf("Press CTRL+C to stop\r\n");

        furi_hal_bt_start_packet_rx(channel, 1);

        while(!cli_cmd_interrupt_received(cli)) {
            furi_delay_ms(250);
            printf("RSSI: %6.1f dB\r", (double)furi_hal_bt_get_rssi());
            fflush(stdout);
        }

        furi_hal_bt_stop_packet_test();

        bt_set_profile(bt, BtProfileSerial);
        furi_record_close(RECORD_BT);
    } while(false);
}

static void bt_cli_command_packet_tx(Cli* cli, string_t args, void* context) {
    UNUSED(context);
    int channel = 0;
    int pattern = 0;
    int datarate = 1;

    do {
        if(!args_read_int_and_trim(args, &channel) && (channel < 0 || channel > 39)) {
            printf("Incorrect or missing channel, expected int 0-39");
            break;
        }
        if(!args_read_int_and_trim(args, &pattern) && (pattern < 0 || pattern > 5)) {
            printf("Incorrect or missing pattern, expected int 0-5 \r\n");
            printf("0 - Pseudo-Random bit sequence 9\r\n");
            printf("1 - Pattern of alternating bits '11110000'\r\n");
            printf("2 - Pattern of alternating bits '10101010'\r\n");
            printf("3 - Pseudo-Random bit sequence 15\r\n");
            printf("4 - Pattern of All '1' bits\r\n");
            printf("5 - Pattern of All '0' bits\r\n");
            break;
        }
        if(!args_read_int_and_trim(args, &datarate) && (datarate < 1 || datarate > 2)) {
            printf("Incorrect or missing datarate, expected int 1-2");
            break;
        }

        Bt* bt = furi_record_open(RECORD_BT);
        bt_disconnect(bt);
        furi_hal_bt_reinit();
        printf(
            "Transmitting %d pattern packet at %d channel at %d M datarate\r\n",
            pattern,
            channel,
            datarate);
        printf("Press CTRL+C to stop\r\n");
        furi_hal_bt_start_packet_tx(channel, pattern, datarate);

        while(!cli_cmd_interrupt_received(cli)) {
            furi_delay_ms(250);
        }
        furi_hal_bt_stop_packet_test();
        printf("Transmitted %lu packets", furi_hal_bt_get_transmitted_packets());

        bt_set_profile(bt, BtProfileSerial);
        furi_record_close(RECORD_BT);
    } while(false);
}

static void bt_cli_command_packet_rx(Cli* cli, string_t args, void* context) {
    UNUSED(context);
    int channel = 0;
    int datarate = 1;

    do {
        if(!args_read_int_and_trim(args, &channel) && (channel < 0 || channel > 39)) {
            printf("Incorrect or missing channel, expected int 0-39");
            break;
        }
        if(!args_read_int_and_trim(args, &datarate) && (datarate < 1 || datarate > 2)) {
            printf("Incorrect or missing datarate, expected int 1-2");
            break;
        }

        Bt* bt = furi_record_open(RECORD_BT);
        bt_disconnect(bt);
        furi_hal_bt_reinit();
        printf("Receiving packets at %d channel at %d M datarate\r\n", channel, datarate);
        printf("Press CTRL+C to stop\r\n");
        furi_hal_bt_start_packet_rx(channel, datarate);

        while(!cli_cmd_interrupt_received(cli)) {
            furi_delay_ms(250);
            printf("RSSI: %03.1f dB\r", (double)furi_hal_bt_get_rssi());
            fflush(stdout);
        }
        uint16_t packets_received = furi_hal_bt_stop_packet_test();
        printf("Received %hu packets", packets_received);

        bt_set_profile(bt, BtProfileSerial);
        furi_record_close(RECORD_BT);
    } while(false);
}

static void bt_cli_print_usage() {
    printf("Usage:\r\n");
    printf("bt <cmd> <args>\r\n");
    printf("Cmd list:\r\n");
    printf("\thci_info\t - HCI info\r\n");
    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug) && furi_hal_bt_is_testing_supported()) {
        printf("\ttx_carrier <channel:0-39> <power:0-6>\t - start tx carrier test\r\n");
        printf("\trx_carrier <channel:0-39>\t - start rx carrier test\r\n");
        printf(
            "\ttx_packet <channel:0-39> <pattern:0-5> <datarate:1-2>\t - start tx packet test\r\n");
        printf("\trx_packet <channel:0-39> <datarate:1-2>\t - start rx packer test\r\n");
    }
}

static void bt_cli(Cli* cli, string_t args, void* context) {
    UNUSED(context);
    furi_record_open(RECORD_BT);

    string_t cmd;
    string_init(cmd);
    BtSettings bt_settings;
    bt_settings_load(&bt_settings);

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            bt_cli_print_usage();
            break;
        }
        if(string_cmp_str(cmd, "hci_info") == 0) {
            bt_cli_command_hci_info(cli, args, NULL);
            break;
        }
        if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug) && furi_hal_bt_is_testing_supported()) {
            if(string_cmp_str(cmd, "tx_carrier") == 0) {
                bt_cli_command_carrier_tx(cli, args, NULL);
                break;
            }
            if(string_cmp_str(cmd, "rx_carrier") == 0) {
                bt_cli_command_carrier_rx(cli, args, NULL);
                break;
            }
            if(string_cmp_str(cmd, "tx_packet") == 0) {
                bt_cli_command_packet_tx(cli, args, NULL);
                break;
            }
            if(string_cmp_str(cmd, "rx_packet") == 0) {
                bt_cli_command_packet_rx(cli, args, NULL);
                break;
            }
        }

        bt_cli_print_usage();
    } while(false);

    if(bt_settings.enabled) {
        furi_hal_bt_start_advertising();
    }

    string_clear(cmd);
    furi_record_close(RECORD_BT);
}

void bt_on_system_start() {
#ifdef SRV_CLI
    Cli* cli = furi_record_open(RECORD_CLI);
    cli_add_command(cli, RECORD_BT, CliCommandFlagDefault, bt_cli, NULL);
    furi_record_close(RECORD_CLI);
#else
    UNUSED(bt_cli);
#endif
}
