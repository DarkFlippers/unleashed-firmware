#include <furi.h>
#include <furi_hal.h>
#include <cli/cli.h>
#include <lib/toolbox/args.h>
#include <lib/toolbox/hex.h>

#include <lib/nfc/nfc_types.h>
#include <lib/nfc/nfc_device.h>

static void nfc_cli_print_usage() {
    printf("Usage:\r\n");
    printf("nfc <cmd>\r\n");
    printf("Cmd list:\r\n");
    printf("\tdetect\t - detect nfc device\r\n");
    printf("\temulate\t - emulate predefined nfca card\r\n");
    printf("\tapdu\t - Send APDU and print response \r\n");
    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
        printf("\tfield\t - turn field on\r\n");
    }
}

static void nfc_cli_detect(Cli* cli, FuriString* args) {
    UNUSED(args);
    // Check if nfc worker is not busy
    if(furi_hal_nfc_is_busy()) {
        printf("Nfc is busy\r\n");
        return;
    }

    FuriHalNfcDevData dev_data = {};
    bool cmd_exit = false;
    furi_hal_nfc_exit_sleep();
    printf("Detecting nfc...\r\nPress Ctrl+C to abort\r\n");
    while(!cmd_exit) {
        cmd_exit |= cli_cmd_interrupt_received(cli);
        if(furi_hal_nfc_detect(&dev_data, 400)) {
            printf("Found: %s ", nfc_get_dev_type(dev_data.type));
            printf("UID length: %d, UID:", dev_data.uid_len);
            for(size_t i = 0; i < dev_data.uid_len; i++) {
                printf("%02X", dev_data.uid[i]);
            }
            printf("\r\n");
            break;
        }
        furi_hal_nfc_sleep();
        furi_delay_ms(50);
    }
    furi_hal_nfc_sleep();
}

static void nfc_cli_emulate(Cli* cli, FuriString* args) {
    UNUSED(args);
    // Check if nfc worker is not busy
    if(furi_hal_nfc_is_busy()) {
        printf("Nfc is busy\r\n");
        return;
    }

    furi_hal_nfc_exit_sleep();
    printf("Emulating NFC-A Type: T2T UID: 36 9C E7 B1 0A C1 34 SAK: 00 ATQA: 00/44\r\n");
    printf("Press Ctrl+C to abort\r\n");

    FuriHalNfcDevData params = {
        .uid = {0x36, 0x9C, 0xe7, 0xb1, 0x0A, 0xC1, 0x34},
        .uid_len = 7,
        .atqa = {0x44, 0x00},
        .sak = 0x00,
        .type = FuriHalNfcTypeA,
    };

    while(!cli_cmd_interrupt_received(cli)) {
        if(furi_hal_nfc_listen(params.uid, params.uid_len, params.atqa, params.sak, false, 100)) {
            printf("Reader detected\r\n");
            furi_hal_nfc_sleep();
        }
        furi_delay_ms(50);
    }
    furi_hal_nfc_sleep();
}

static void nfc_cli_field(Cli* cli, FuriString* args) {
    UNUSED(args);
    // Check if nfc worker is not busy
    if(furi_hal_nfc_is_busy()) {
        printf("Nfc is busy\r\n");
        return;
    }

    furi_hal_nfc_exit_sleep();
    furi_hal_nfc_field_on();

    printf("Field is on. Don't leave device in this mode for too long.\r\n");
    printf("Press Ctrl+C to abort\r\n");

    while(!cli_cmd_interrupt_received(cli)) {
        furi_delay_ms(50);
    }

    furi_hal_nfc_field_off();
    furi_hal_nfc_sleep();
}

static void nfc_cli_apdu(Cli* cli, FuriString* args) {
    UNUSED(cli);
    if(furi_hal_nfc_is_busy()) {
        printf("Nfc is busy\r\n");
        return;
    }

    furi_hal_nfc_exit_sleep();
    FuriString* data = NULL;
    data = furi_string_alloc();
    FuriHalNfcTxRxContext tx_rx = {};
    FuriHalNfcDevData dev_data = {};
    uint8_t* req_buffer = NULL;
    uint8_t* resp_buffer = NULL;
    size_t apdu_size = 0;
    size_t resp_size = 0;

    do {
        if(!args_read_string_and_trim(args, data)) {
            printf(
                "Use like `nfc apdu 00a404000e325041592e5359532e444446303100 00a4040008a0000003010102` \r\n");
            break;
        }

        printf("detecting tag\r\n");
        if(!furi_hal_nfc_detect(&dev_data, 300)) {
            printf("Failed to detect tag\r\n");
            break;
        }
        do {
            apdu_size = furi_string_size(data) / 2;
            req_buffer = malloc(apdu_size);
            hex_chars_to_uint8(furi_string_get_cstr(data), req_buffer);

            memcpy(tx_rx.tx_data, req_buffer, apdu_size);
            tx_rx.tx_bits = apdu_size * 8;
            tx_rx.tx_rx_type = FuriHalNfcTxRxTypeDefault;

            printf("Sending APDU:%s to Tag\r\n", furi_string_get_cstr(data));
            if(!furi_hal_nfc_tx_rx(&tx_rx, 300)) {
                printf("Failed to tx_rx\r\n");
                break;
            }
            resp_size = (tx_rx.rx_bits / 8) * 2;
            resp_buffer = malloc(resp_size);
            uint8_to_hex_chars(tx_rx.rx_data, resp_buffer, resp_size);
            resp_buffer[resp_size] = 0;
            printf("Response: %s\r\n", resp_buffer);
            free(req_buffer);
            free(resp_buffer);
            req_buffer = NULL;
            resp_buffer = NULL;
        } while(args_read_string_and_trim(args, data));
    } while(false);

    free(req_buffer);
    free(resp_buffer);
    furi_string_free(data);
    furi_hal_nfc_sleep();
}

static void nfc_cli(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);
    FuriString* cmd;
    cmd = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            nfc_cli_print_usage();
            break;
        }
        if(furi_string_cmp_str(cmd, "detect") == 0) {
            nfc_cli_detect(cli, args);
            break;
        }
        if(furi_string_cmp_str(cmd, "emulate") == 0) {
            nfc_cli_emulate(cli, args);
            break;
        }

        if(furi_string_cmp_str(cmd, "apdu") == 0) {
            nfc_cli_apdu(cli, args);
            break;
        }

        if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
            if(furi_string_cmp_str(cmd, "field") == 0) {
                nfc_cli_field(cli, args);
                break;
            }
        }

        nfc_cli_print_usage();
    } while(false);

    furi_string_free(cmd);
}

void nfc_on_system_start() {
#ifdef SRV_CLI
    Cli* cli = furi_record_open(RECORD_CLI);
    cli_add_command(cli, "nfc", CliCommandFlagDefault, nfc_cli, NULL);
    furi_record_close(RECORD_CLI);
#else
    UNUSED(nfc_cli);
#endif
}
