#include <furi.h>
#include <furi_hal.h>
#include <cli/cli.h>
#include <toolbox/args.h>

#include "nfc_types.h"

static void nfc_cli_print_usage() {
    printf("Usage:\r\n");
    printf("nfc <cmd>\r\n");
    printf("Cmd list:\r\n");
    printf("\tdetect\t - detect nfc device\r\n");
    printf("\temulate\t - emulate predefined nfca card\r\n");
    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
        printf("\tfield\t - turn field on\r\n");
    }
}

void nfc_cli_detect(Cli* cli, string_t args) {
    // Check if nfc worker is not busy
    if(furi_hal_nfc_is_busy()) {
        printf("Nfc is busy\r\n");
        return;
    }
    rfalNfcDevice* dev_list;
    uint8_t dev_cnt = 0;
    bool cmd_exit = false;
    furi_hal_nfc_exit_sleep();
    printf("Detecting nfc...\r\nPress Ctrl+C to abort\r\n");
    while(!cmd_exit) {
        cmd_exit |= cli_cmd_interrupt_received(cli);
        cmd_exit |= furi_hal_nfc_detect(&dev_list, &dev_cnt, 400, true);
        if(dev_cnt > 0) {
            printf("Found %d devices\r\n", dev_cnt);
            for(uint8_t i = 0; i < dev_cnt; i++) {
                printf("%d found: %s ", i + 1, nfc_get_rfal_type(dev_list[i].type));
                if(dev_list[i].type == RFAL_NFC_LISTEN_TYPE_NFCA) {
                    printf("type: %s, ", nfc_get_nfca_type(dev_list[i].dev.nfca.type));
                }
                printf("UID length: %d, UID:", dev_list[i].nfcidLen);
                for(uint8_t j = 0; j < dev_list[i].nfcidLen; j++) {
                    printf("%02X", dev_list[i].nfcid[j]);
                }
                printf("\r\n");
            }
        }
        osDelay(50);
    }
    furi_hal_nfc_deactivate();
}

void nfc_cli_emulate(Cli* cli, string_t args) {
    // Check if nfc worker is not busy
    if(furi_hal_nfc_is_busy()) {
        printf("Nfc is busy\r\n");
        return;
    }

    furi_hal_nfc_exit_sleep();
    printf("Emulating NFC-A Type: T2T UID: 36 9C E7 B1 0A C1 34 SAK: 00 ATQA: 00/44\r\n");
    printf("Press Ctrl+C to abort\r\n");

    NfcDeviceCommonData params = {
        .uid = {0x36, 0x9C, 0xe7, 0xb1, 0x0A, 0xC1, 0x34},
        .uid_len = 7,
        .atqa = {0x44, 0x00},
        .sak = 0x00,
        .device = NfcDeviceNfca,
        .protocol = NfcDeviceProtocolMifareUl,
    };

    while(!cli_cmd_interrupt_received(cli)) {
        if(furi_hal_nfc_listen(params.uid, params.uid_len, params.atqa, params.sak, false, 100)) {
            printf("Reader detected\r\n");
            furi_hal_nfc_deactivate();
        }
        osDelay(50);
    }
    furi_hal_nfc_deactivate();
}

void nfc_cli_field(Cli* cli, string_t args) {
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
        osDelay(50);
    }

    furi_hal_nfc_field_off();
    furi_hal_nfc_deactivate();
}

static void nfc_cli(Cli* cli, string_t args, void* context) {
    string_t cmd;
    string_init(cmd);

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            nfc_cli_print_usage();
            break;
        }
        if(string_cmp_str(cmd, "detect") == 0) {
            nfc_cli_detect(cli, args);
            break;
        }
        if(string_cmp_str(cmd, "emulate") == 0) {
            nfc_cli_emulate(cli, args);
            break;
        }

        if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
            if(string_cmp_str(cmd, "field") == 0) {
                nfc_cli_field(cli, args);
                break;
            }
        }

        nfc_cli_print_usage();
    } while(false);

    string_clear(cmd);
}

void nfc_on_system_start() {
#ifdef SRV_CLI
    Cli* cli = furi_record_open("cli");
    cli_add_command(cli, "nfc", CliCommandFlagDefault, nfc_cli, NULL);
    furi_record_close("cli");
#else
    UNUSED(nfc_cli);
#endif
}
